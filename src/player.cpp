#include "player.hpp"

#include <GLM/gtx/string_cast.hpp>

constexpr float EPSILON = 0.0001f;

void Player::updatePosition()
{
    const glm::vec3 displacement = position - prevPosition;
    camera.translate(displacement);
    reach.setOrigin(camera.getEye());
    hitbox.translate(displacement);
}

int Player::getKeyState(const int key) const
{
    return window.getKeyboardKey(key);
}

bool Player::isKeyPressed(const int key_state) const
{
    return getKeyState(key_state) == GLFW_PRESS;
}

void Player::pollKeyboardControls(const double delta)
{
    float adj_speed = speed;
    float dt = static_cast<float>(delta);
    if (isKeyPressed(GLFW_KEY_LEFT_CONTROL))
    {
        adj_speed *= DEFAULT_SPRINT_MULTIPLIER;
    }

    glm::vec3 forward = camera.getForward();
    forward.y = 0.0f;
    forward = glm::normalize(forward);
    glm::vec3 right = camera.getRight();
    right.y = 0.0f;
    right = glm::normalize(right);
    glm::vec3 up = camera.getWorldUp();

    glm::vec3 new_velocity = velocity;

    bool player_moved = false;

    if (gameMode == GameMode::Creative)
    {
        new_velocity = {};
        adj_speed *= 2.0f;
        if (isKeyPressed(GLFW_KEY_W))
        {
            new_velocity += forward * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_S))
        {
            new_velocity += -forward * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_A))
        {
            new_velocity += -right * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_D))
        {
            new_velocity += right * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_LEFT_SHIFT))
        {
            new_velocity += -up * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_SPACE))
        {
            new_velocity += up * adj_speed;
        }
    }
    else if (gameMode == GameMode::Survival)
    {
        new_velocity.x = 0.0f;
        new_velocity.z = 0.0f;

        if (isKeyPressed(GLFW_KEY_W))
        {
            new_velocity += forward * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_S))
        {
            new_velocity += -forward * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_A))
        {
            new_velocity += -right * adj_speed;
        }
        if (isKeyPressed(GLFW_KEY_D))
        {
            new_velocity += right * adj_speed;
        }

        // Apply forces.
        new_velocity.y += world.getGravity() * 2.0f * dt;

        constexpr float jump_force = 7.0f;
        if (isOnFloor && isKeyPressed(GLFW_KEY_SPACE))
        {
            new_velocity += up * jump_force;
        }
    }

    const float prev_dt = dt;
    if (new_velocity != glm::vec3(0.0f))
    {
        // Update the position.
        glm::vec3 new_position = position;
        Aabb3d new_hitbox = hitbox;
        float remaining_dt = dt;
        isOnFloor = false;
        for (glm::length_t i = 0; i < 3; ++i)
        {
            // Run a collision test.
            glm::vec3 normal{};
            float new_dt = remaining_dt;

            bool intersected = false;
            if (gameMode != GameMode::Creative)
            {
                intersected =
                    world.doesEntityIntersect(new_position, new_velocity, remaining_dt, new_hitbox, new_dt, &normal);
            }

            if (!intersected)
            {
                const glm::vec3 displacement = new_velocity * remaining_dt;
                new_position += displacement;
                break;
            }

            assert(glm::length(normal) > 0.0f);

            // Update the potential position.
            const glm::vec3 offset = normal * EPSILON;
            const glm::vec3 displacement = new_velocity * new_dt + offset;
            new_position += displacement;
            new_hitbox.translate(displacement);
            remaining_dt -= new_dt;

            assert(remaining_dt >= 0.0f);

            // Apply a slide response.
            new_velocity -= normal * glm::dot(new_velocity, normal);

            // Check if the player is now on a floor.
            if (normal == camera.getWorldUp())
            {
                isOnFloor = true;
            }

            // Don't continue if there is no more velocity or time to process.
            if (glm::length(new_velocity) <= 0.0f || remaining_dt <= 0.0f)
            {
                break;
            }
        }
        position = new_position;
        velocity = new_velocity;

        // Notify the world to update chunks.
        const ChunkCenter curr_chunk_center = world.getPosToChunkCenter(getPosition());
        if (chunkCenter != curr_chunk_center)
        {
            chunkCenter = curr_chunk_center;
            world.updateChunks(getPosition(), getRenderDistance());
        }
    }
}

void Player::eventKeyboardControls(const int key, const int scancode, const int action, const int mods)
{
    // TODO: move somewhere else that is UI related.
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_LEFT_ALT)
        {
            if (window.getInputMode(GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
            {
                window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else
            {
                window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }

        if (key == GLFW_KEY_F1)
        {
            switch (gameMode)
            {
            case GameMode::Creative: {
                gameMode = GameMode::Survival;
                break;
            }
            case GameMode::Survival: {
                gameMode = GameMode::Creative;
                break;
            }
            }
        }
    }
}

void Player::eventMouseControls(const int button, const int action, const int mods)
{
    // TODO: add something for attacking mobs.
}

Player::Player(Window& window, World& world, const glm::vec3& pos, const float speed, const unsigned render_distance)
    : window(window), world(world),
      camera(
          window,
          pos + glm::vec3(0.0f, DEFAULT_PLAYER_HEIGHT - 0.18f, 0.0f),
          pos + glm::vec3(0.0f, DEFAULT_PLAYER_HEIGHT - 0.18f, -1.0f),
          glm::vec3(0.0f, 1.0f, 0.0f),
          glm::radians(70.0f),
          (static_cast<float>(Window::DEFAULT_WIDTH) / static_cast<float>(Window::DEFAULT_HEIGHT)),
          0.1f,
          1000.0f),
      position(pos), prevPosition(pos), speed(speed), renderDistance(render_distance),
      reach(pos + camera.getEye(), camera.getForward(), 0.0f, 2.0f),
      hitbox(pos + glm::vec3(-0.3f, 0.0f, -0.3f), pos + glm::vec3(0.3f, DEFAULT_PLAYER_HEIGHT, 0.3f)),
      chunkCenter(world.getPosToChunkCenter(getPosition()))
{
    window.addKeyCallback([this](int key, int scancode, int action, int mods) {
        this->eventKeyboardControls(key, scancode, action, mods);
    });
    window.addMouseButtonCallback(
        [this](int button, int action, int mods) { this->eventMouseControls(button, action, mods); });
}

void Player::update(const double delta)
{
    prevPosition = position;

    // Keyboard input.
    pollKeyboardControls(delta);

    // Mouse input.
    double x = 0.0, y = 0.0;
    window.getCursorPosition(x, y);
    if (window.getInputMode(GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        const float delta_x = (static_cast<float>(x) - cursorPrevX) * mouseSensitivity;
        const float delta_y = -(static_cast<float>(y) - cursorPrevY) * mouseSensitivity;

        if (delta_x != 0.0f || delta_y != 0.0f)
        {
            camera.rotate(delta_y, delta_x, 0.0f, true);
            reach.setDirection(camera.getForward());
        }
    }
    cursorPrevX = static_cast<float>(x);
    cursorPrevY = static_cast<float>(y);

    glm::ivec3 face_entered{};
    std::optional<glm::vec3> block_pos = world.getReachableBlock(reach, &face_entered);
    if (block_pos.has_value())
    {
        if (window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            // TODO: handle block addition.
            world.addBlock(block_pos.value() + static_cast<glm::vec3>(face_entered));
        }
        else if (window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            // TODO: handle block deletion.
            world.removeBlock(block_pos.value());
        }
    }

    updatePosition();

    // std::cout << "@pos " << glm::to_string(position) << "     "
    //           << "\n@vel " << glm::to_string(velocity) << "     "
    //           << "\n@is on floor? " << (isOnFloor ? "(T)" : "(F)") << "     "
    //           << "\033[F\033[F" << "\r";
}

const Camera& Player::getCamera() const
{
    return camera;
}

const glm::vec3 Player::getPosition() const
{
    return position;
}

const unsigned Player::getRenderDistance() const
{
    return renderDistance;
}

const Ray& Player::getRay() const
{
    return reach;
}
