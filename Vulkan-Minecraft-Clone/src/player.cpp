#include "player.hpp"

#include <iostream>

void Player::pollKeyboardControls()
{
    float adj_speed = speed;
    if (window.getKeyboardKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        adj_speed *= DEFAULT_SPRINT_MULTIPLIER;
    }

    const glm::vec3 forward = camera.getForward();
    const glm::vec3 right = camera.getRight();

    bool player_moved = false;
    if (window.getKeyboardKey(GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.moveForwardXZ(adj_speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.moveBackwardXZ(adj_speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.moveLeftXZ(adj_speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.moveRightXZ(adj_speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.moveUpXZ(adj_speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.moveDownXZ(adj_speed);
        player_moved = true;
    }

    if (player_moved)
    {
        // world.updateChunks(getPosition(), getRenderDistance());
        reach.setOrigin(position + camera.getEye()); // TODO:
    }
}

void Player::eventKeyboardControls(const int key, const int scancode, const int action, const int mods)
{
    // TODO: move somewhere else that is UI related.
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
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
}

void Player::eventMouseControls(const int button, const int action, const int mods)
{
    // TODO: add something for attacking mobs.
}

Player::Player(Window& window, World& world, const glm::vec3& pos, const float speed, const unsigned render_distance)
    : window(window), world(world),
      camera(
          window,
          glm::vec3(0, 2, 0),
          glm::vec3(0, 2, -1),
          glm::vec3(0, 1, 0),
          glm::radians(70.0f),
          (static_cast<float>(Window::DEFAULT_WIDTH) / static_cast<float>(Window::DEFAULT_HEIGHT)),
          0.1f,
          1000.0f),
      position(pos), speed(speed), renderDistance(render_distance),
      reach(pos + camera.getEye(), camera.getForward(), 0.0f, 2.0f)
{
    window.addKeyCallback([this](int key, int scancode, int action, int mods) {
        this->eventKeyboardControls(key, scancode, action, mods);
    });
    window.addMouseButtonCallback(
        [this](int button, int action, int mods) { this->eventMouseControls(button, action, mods); });
}

void Player::processInput()
{
    // Keyboard input.
    pollKeyboardControls();

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
}

const Camera& Player::getCamera() const
{
    return camera;
}

const glm::vec3 Player::getPosition() const
{
    // TODO:
    // return position;
    return camera.getEye();
}

const unsigned Player::getRenderDistance() const
{
    return renderDistance;
}

const Ray& Player::getRay() const
{
    return reach;
}
