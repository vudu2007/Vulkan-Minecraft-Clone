#include "world.hpp"

#include <cassert>
#include <iostream>

void World::runChunkLoadedCallbacks(const Chunk& chunk, const int propagation_depth)
{
    std::array<const Chunk*, 6> neighboring_chunks = getNeighboringChunks(chunk.getCenter());
    for (const auto& callback : chunkLoadedCallbacks)
    {
        callback(chunk, neighboring_chunks);
    }

    // Tell the neighbors to update if they're still visible.
    if (propagation_depth > 0)
    {
        for (const auto c : neighboring_chunks)
        {
            if (c != nullptr && visibleChunks.contains(c->getCenter()))
            {
                runChunkLoadedCallbacks(*c, propagation_depth - 1);
            }
        }
    }
}

void World::runChunkUnloadedCallbacks(const Chunk& chunk)
{
    for (const auto& callback : chunkUnloadedCallbacks)
    {
        callback(chunk);
    }
}

bool World::isChunkActive(const ChunkCenter& cc) const
{
    return activeChunks.contains(cc) && chunks.contains(cc);
}

std::array<const Chunk*, 6> World::getNeighboringChunks(const ChunkCenter& cc) const
{
    std::array<const Chunk*, 6> neighboring_chunks{};

    std::vector<glm::vec3> offsets = {
        glm::vec3(chunkSize, 0.0f, 0.0f),  // +x
        glm::vec3(-chunkSize, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, chunkSize, 0.0f),  // +y
        glm::vec3(0.0f, -chunkSize, 0.0f), // -y
        glm::vec3(0.0f, 0.0f, chunkSize),  // +z
        glm::vec3(0.0f, 0.0f, -chunkSize), // -z
    };

    for (size_t i = 0; i < offsets.size(); ++i)
    {
        const glm::vec3 offset = offsets[i];
        const ChunkCenter neighbor_cc = getPosToChunkCenter(cc + offset);

        if (chunks.contains(neighbor_cc))
        {
            neighboring_chunks[i] = chunks.at(neighbor_cc);
        }
    }

    return neighboring_chunks;
}

World::World(const unsigned seed, const int chunk_size, const unsigned num_threads)
    : terrainHeightNoise(seed), seed(seed), chunkSize(chunk_size), threadPool(std::max(num_threads, 1u))
{
    // The number of threads for this world will always be at least 1.

    // Set up noises.
    terrainHeightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    terrainHeightNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    terrainHeightNoise.SetFractalOctaves(5);
    terrainHeightNoise.SetFractalWeightedStrength(1.5f);
}

World::~World()
{
    chunkLoadedCallbacks.clear();
    chunkUnloadedCallbacks.clear();

    threadPool.purge();
    threadPool.wait();

    for (auto& chunk : chunks)
    {
        free(chunk.second);
    }
}

void World::init(const glm::vec3& origin, const unsigned radius)
{
    std::cout << ">>> Generating world with seed (" << seed << ") using (" << threadPool.get_thread_count()
              << ") threads..." << std::endl;

    const unsigned total_num_chunks = updateChunks(origin, radius);

    // Report an update every second.
    do
    {
        std::cout << "  Loaded " << (total_num_chunks - chunksToAdd.size()) << "/" << total_num_chunks << " chunks."
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!chunksToAdd.empty());

    std::cout << "  Loaded " << (total_num_chunks - chunksToAdd.size()) << "/" << total_num_chunks << " chunks."
              << std::endl;

    std::cout << ">>> Finished loading world!" << std::endl;
}

std::optional<glm::vec3> World::getReachableBlock(const Ray& ray, glm::ivec3* face_entered)
{
    std::lock_guard<std::mutex> lock(accessChunksMutex);

    std::optional<glm::vec3> reachable_block_pos;

    // Under the assumption that the player's reach is never infinity.

    // Check the chunk that contains the ray's origin (the player's position).
    const ChunkCenter cc = getPosToChunkCenter(ray.getOrigin());
    if (isChunkActive(cc))
    {
        reachable_block_pos = chunks[cc]->getReachableBlock(ray, face_entered);
    }
    if (reachable_block_pos.has_value())
    {
        return reachable_block_pos;
    }

    // Check the chunk that contains the ray's direction at its max distance (where the player is
    // looking).
    const ChunkCenter next_cc = getPosToChunkCenter(ray.getOrigin() + ray.getDirection() * ray.getMax());
    if (next_cc == cc)
    {
        return reachable_block_pos;
    }
    if (isChunkActive(next_cc))
    {
        reachable_block_pos = chunks[next_cc]->getReachableBlock(ray, face_entered);
    }

    return reachable_block_pos;
}

bool World::doesEntityIntersect(
    const glm::vec3& pos,
    const glm::vec3& velocity,
    const float delta,
    const Aabb3d& hitbox,
    float& new_delta,
    glm::vec3* normal)
{
    // Given the entity's position, determine which chunks to check.
    // TODO: entity can go so fast that the chunks haven't loaded yet; can maybe add boundaries to already loaded chunks
    // where the player cannot enter chunks that haven't been loaded?
    // TODO: maybe add all chunks relevant to the player's displacement?
    std::unordered_set<ChunkCenter> chunk_centers;
    std::vector<ChunkCenter> offsets = {
        {0.0f,       0.0f,       0.0f      },
        {chunkSize,  0.0f,       0.0f      },
        {-chunkSize, 0.0f,       0.0f      },
        {0.0f,       chunkSize,  0.0f      },
        {0.0f,       -chunkSize, 0.0f      },
        {0.0f,       0.0f,       chunkSize },
        {0.0f,       0.0f,       -chunkSize},
    };
    for (const auto& offset : offsets)
    {
        {
            std::lock_guard<std::mutex> lock(accessChunksMutex);
            const ChunkCenter cc = getPosToChunkCenter(pos) + offset;
            if (chunks.contains(cc))
            {
                chunk_centers.emplace(cc);
            }
        }
    }

    // Ask the chunks to run intersection test on this entity and return the result.
    new_delta = delta;
    bool intersected = false;
    glm::vec3 closest_normal{};
    for (const auto& cc : chunk_centers)
    {
        float curr_new_delta;
        glm::vec3 curr_normal;
        {
            std::lock_guard<std::mutex> lock(accessChunksMutex);
            if (chunks[cc]->doesEntityIntersect(velocity, delta, hitbox, curr_new_delta, &curr_normal))
            {
                if (curr_new_delta < new_delta)
                {
                    new_delta = curr_new_delta;
                    closest_normal = curr_normal;
                }
                intersected = true;
            }
        }
    }

    if (normal != nullptr)
    {
        *normal = closest_normal;
    }

    // No intersection was found, so return false.
    return intersected;
}

void World::addChunk(const std::vector<glm::vec3> chunk_centers)
{
    for (const auto& cc : chunk_centers)
    {
        {
            std::lock_guard<std::mutex> lock(accessChunksMutex);
            // Don't generate the chunk if it was already generated by another thread.
            if (chunks.contains(cc))
            {
                continue;
            }
        }

        Chunk* chunk = new Chunk(terrainHeightNoise, cc, chunkSize);

        {
            std::lock_guard<std::mutex> lock(accessChunksMutex);

            assert(!chunks.contains(cc));

            chunks[cc] = chunk;
            chunksToAdd.erase(cc);
        }
    }
}

void World::draw(const glm::vec3& origin, const unsigned radius, const Frustum& frustum)
{
    // Load all chunks visible to the player.
    const int render_distance = static_cast<int>(radius);
    const float offset = static_cast<float>(render_distance * chunkSize);

    std::unordered_set<ChunkCenter> hidden_chunks = visibleChunks;
    const float chunk_extent = chunkSize * 0.5f;

    // Iterate through new and/or old active chunks.
    float x = origin.x - offset;
    for (int i = 0; i <= (render_distance * 2); ++i)
    {
        float y = origin.y - offset;
        for (int j = 0; j <= (render_distance * 2); ++j)
        {
            float z = origin.z - offset;
            for (int k = 0; k <= (render_distance * 2); ++k)
            {
                const ChunkCenter cc = getPosToChunkCenter({x, y, z});

                // Handle chunk visibility; whether this active chunk is visible according to the frustum.
                const Aabb3d chunk_collider((cc - chunk_extent), (cc + chunk_extent));
                const bool in_frustum = frustum.isAabbInside(chunk_collider);
                if (in_frustum)
                {
                    hidden_chunks.erase(cc);
                    if (!visibleChunks.contains(cc))
                    {
                        chunksToShow.emplace(cc);
                    }
                }
                z += chunkSize;
            }
            y += chunkSize;
        }
        x += chunkSize;
    }

    // Remove now hidden chunks.
    for (const auto& cc : hidden_chunks)
    {
        visibleChunks.erase(cc);

        {
            std::lock_guard<std::mutex> lock(accessChunksMutex);

            if (chunks.contains(cc))
            {
                runChunkUnloadedCallbacks(*chunks[cc]);
            }
        }
    }

    // Add any visible chunks that are now ready.
    if (!chunksToShow.empty())
    {
        std::vector<ChunkCenter> to_remove;
        to_remove.reserve(chunksToShow.size());
        for (const auto& cc : chunksToShow)
        {
            {
                std::lock_guard<std::mutex> lock(accessChunksMutex);
                if (chunks.contains(cc))
                {
                    visibleChunks.emplace(cc);
                    to_remove.push_back(cc);
                    runChunkLoadedCallbacks(*chunks[cc], 0);
                }
            }
        }
        for (const auto& cc : to_remove)
        {
            chunksToShow.erase(cc);
        }
    }
}

unsigned World::updateChunks(const glm::vec3& origin, const unsigned radius)
{
    // Load all chunks visible to the player.
    const int render_distance = static_cast<int>(radius);
    const float offset = static_cast<float>(render_distance * chunkSize);

    std::vector<ChunkCenter> new_chunk_centers;
    activeChunks.clear();

    // Iterate through new active chunks.
    float x = origin.x - offset;
    for (int i = 0; i <= (render_distance * 2); ++i)
    {
        float y = origin.y - offset;
        for (int j = 0; j <= (render_distance * 2); ++j)
        {
            float z = origin.z - offset;
            for (int k = 0; k <= (render_distance * 2); ++k)
            {
                const ChunkCenter cc = getPosToChunkCenter({x, y, z});

                // Make sure the chunk is marked as active even if it hasn't loaded.
                activeChunks.emplace(cc);

                {
                    std::lock_guard<std::mutex> lock(accessChunksMutex);
                    if (!chunks.contains(cc))
                    {
                        if (!chunksToAdd.contains(cc))
                        {
                            // Create the chunk asynchrounously and add it later.
                            chunksToAdd.emplace(cc);
                            new_chunk_centers.emplace_back(cc);
                        } // Else, don't add it to the set of chunks to add because it's already
                          // in there.
                    }
                }

                z += chunkSize;
            }
            y += chunkSize;
        }
        x += chunkSize;
    }

    // Add chunks in batches asynchronously.
    if (!new_chunk_centers.empty())
    {
        // Give a batch of chunks to a `num_sections` async thread(s) to process.
        const size_t num_sections = threadPool.get_thread_count(); // Guaranteed to be at least 1 thread.
        const size_t section_size = new_chunk_centers.size() / num_sections;
        for (size_t section_i = 0; section_i < num_sections - 1; ++section_i)
        {
            std::vector<ChunkCenter> section(
                new_chunk_centers.begin() + section_size * section_i,
                new_chunk_centers.begin() + section_size * (section_i + 1));
            threadPool.detach_task([this, section]() { addChunk(section); });
        }
        std::vector<ChunkCenter> section(
            new_chunk_centers.begin() + section_size * (num_sections - 1),
            new_chunk_centers.end());
        threadPool.detach_task([this, section]() { addChunk(section); });
    }

    return static_cast<unsigned>(new_chunk_centers.size());
}

void World::addBlock(const glm::vec3 block_pos)
{
    // std::vector<glm::vec3> offsets = {
    //     glm::vec3(0.0f, 0.0f, 0.0f),
    //     glm::vec3(1.0f, 0.0f, 0.0f),
    //     glm::vec3(-1.0f, 0.0f, 0.0f),
    //     glm::vec3(0.0f, 1.0f, 0.0f),
    //     glm::vec3(0.0f, -1.0f, 0.0f),
    //     glm::vec3(0.0f, 0.0f, 1.0f),
    //     glm::vec3(0.0f, 0.0f, -1.0f),
    // };
    // for (const auto& offset : offsets)
    //{
    //     const ChunkCenter cc = getPosToChunkCenter(block_pos + offset);
    //     if (isChunkActive(cc))
    //     {
    //         chunks[cc]->addBlock(block_pos);
    //         runChunkLoadedCallbacks(*chunks[cc]);
    //     }
    // }
}

void World::removeBlock(const glm::vec3 block_pos)
{
    // std::vector<glm::vec3> offsets = {
    //     glm::vec3(0.0f, 0.0f, 0.0f),
    //     glm::vec3(1.0f, 0.0f, 0.0f),
    //     glm::vec3(-1.0f, 0.0f, 0.0f),
    //     glm::vec3(0.0f, 1.0f, 0.0f),
    //     glm::vec3(0.0f, -1.0f, 0.0f),
    //     glm::vec3(0.0f, 0.0f, 1.0f),
    //     glm::vec3(0.0f, 0.0f, -1.0f),
    // };
    // for (const auto& offset : offsets)
    //{
    //     const ChunkCenter cc = getPosToChunkCenter(block_pos + offset);
    //     if (isChunkActive(cc))
    //     {
    //         chunks[cc]->removeBlock(block_pos);
    //         runChunkLoadedCallbacks(*chunks[cc]);
    //     }
    // }
}

void World::addChunkLoadedCallback(
    const std::function<void(const Chunk&, const std::array<const Chunk*, 6>&)>& callback)
{
    chunkLoadedCallbacks.push_back(callback);
}

void World::clearChunkLoadedCallbacks()
{
    chunkLoadedCallbacks.clear();
}

void World::addChunkUnloadedCallback(const std::function<void(const Chunk&)>& callback)
{
    chunkUnloadedCallbacks.push_back(callback);
}

void World::clearChunkUnloadedCallbacks()
{
    chunkUnloadedCallbacks.clear();
}

const ChunkCenter World::getPosToChunkCenter(const glm::vec3& pos) const
{
    const float fp_chunk_size = static_cast<float>(chunkSize);
    const float stride = fp_chunk_size;
    const float x = std::floor((pos.x + 0.5f) / fp_chunk_size + 0.5f) * stride;
    const float y = std::floor((pos.y + 0.5f) / fp_chunk_size + 0.5f) * stride;
    const float z = std::floor((pos.z + 0.5f) / fp_chunk_size + 0.5f) * stride;
    return ChunkCenter(x, y, z);
}

// const Model World::getModel() const
//{
//     std::vector<Model::Vertex> vertices;
//     std::vector<Model::Index> indices;
//     size_t vertices_size = 0;
//     size_t indices_size = 0;
//     for (const auto& cc : activeChunks)
//     {
//         const Model& chunk_model = chunks.at(cc)->getModel();
//         vertices_size += chunk_model.getVertices().size();
//         indices_size += chunk_model.getIndices().size();
//     }
//     vertices.reserve(vertices_size);
//     indices.reserve(indices_size);
//     Model::Index accum = 0;
//     for (const auto& cc : activeChunks)
//     {
//         const Model& chunk_model = chunks.at(cc)->getModel();
//
//         const auto& chunk_vertices = chunk_model.getVertices();
//         vertices.insert(vertices.end(), chunk_vertices.begin(), chunk_vertices.end());
//
//         std::vector<Model::Index> chunk_indices = chunk_model.getIndices();
//         for (auto& chunk_index : chunk_indices)
//         {
//             chunk_index += accum;
//         }
//         indices.insert(indices.end(), chunk_indices.begin(), chunk_indices.end());
//         accum += static_cast<Model::Index>(chunk_vertices.size());
//     }
//
//     return Model(vertices, indices);
// }

float World::getGravity() const
{
    return gravity;
}
