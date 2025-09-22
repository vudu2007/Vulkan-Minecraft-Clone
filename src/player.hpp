#pragma once

#include "block.hpp"
#include "engine/camera.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/physics/shapes/aabb.hpp"
#include "engine/window.hpp"
#include "world.hpp"

class Player
{
  public:
    enum GameMode
    {
        Creative,
        Survival,
    };

  private:
    static constexpr float DEFAULT_SPRINT_MULTIPLIER = 3.0f;
    static constexpr float DEFAULT_PLAYER_HEIGHT = 1.8f;

    Window& window;

    World& world;
    ChunkCenter chunkCenter;

    Camera camera;
    Ray reach;
    Aabb3d hitbox;

    GameMode gameMode = GameMode::Creative;

    // Player attributes.
    glm::vec3 position;
    glm::vec3 prevPosition;
    glm::vec3 velocity;
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
