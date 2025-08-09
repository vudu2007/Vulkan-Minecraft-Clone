#pragma once

#include "block.hpp"
#include "engine/camera.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/physics/shapes/aabb.hpp"
#include "engine/renderer/window.hpp"
#include "world.hpp"

// TODO: at larger coordinates the player will get stuck in voxels. In the future, we can (1) increase the precision by
// using doubles, (2) have a floating origin so that the player is always near coordinates [0,0,0], (3) do stuff in a
// chunk's local space, and/or (4) round the player's position to safe positions.

class Player
{
  private:
    static constexpr float DEFAULT_SPRINT_MULTIPLIER = 3.0f;
    static constexpr float DEFAULT_PLAYER_HEIGHT = 1.8f;

    Window& window;

    World& world;
    ChunkCenter chunkCenter;

    Camera camera;
    Ray reach;
    Aabb3d hitbox;

    // Player attributes.
    glm::vec3 position;
    glm::vec3 prevPosition;
    float speed;
    unsigned renderDistance;

    bool isOnFloor;

    // Mouse variables.
    float mouseSensitivity = 0.08f;
    float cursorPrevX = 0.0f;
    float cursorPrevY = 0.0f;

    void updatePosition();

    int getKeyState(const int key) const;
    bool isKeyPressed(const int key_state) const;

    void pollKeyboardControls(const double delta);
    void eventKeyboardControls(const int key, const int scancode, const int action, const int mods);

    void eventMouseControls(const int button, const int action, const int mods);

  public:
    const Block* activeBlock = nullptr;

    Player(Window& window, World& world, const glm::vec3& pos, const float speed, const unsigned render_distance);

    void update(const double delta);

    const Camera& getCamera() const;
    const glm::vec3 getPosition() const;
    const unsigned getRenderDistance() const;
    const Ray& getRay() const;
};
