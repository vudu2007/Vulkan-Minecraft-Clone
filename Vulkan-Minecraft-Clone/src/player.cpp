#include "player.hpp"

void Player::pollKeyboardControls()
{
    const glm::vec3 forward = camera.getForward();
    const glm::vec3 right = camera.getRight();

    bool player_moved = false;
    if (window.getKeyboardKey(GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.moveForwardXZ(speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.moveBackwardXZ(speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.moveLeftXZ(speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.moveRightXZ(speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.moveUpXZ(speed);
        player_moved = true;
    }
    if (window.getKeyboardKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.moveDownXZ(speed);
        player_moved = true;
    }

    if (player_moved)
    {
        for (const auto& callback : moveCallbacks)
        {
            callback(*this);
        }
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

Player::Player(Window& window, const glm::vec3& pos, const float speed, const unsigned render_distance)
    : window(window), camera(
                          window,
                          glm::vec3(0, 2, 0),
                          glm::vec3(0, 2, -1),
                          glm::vec3(0, 1, 0),
                          glm::radians(45.0f),
                          (static_cast<float>(Window::DEFAULT_WIDTH) / static_cast<float>(Window::DEFAULT_HEIGHT)),
                          0.1f,
                          1000.0f),
      position(pos), speed(speed), renderDistance(render_distance)
{
    window.addKeyCallback([this](int key, int scancode, int action, int mods) {
        this->eventKeyboardControls(key, scancode, action, mods);
    });
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
        }
    }
    cursorPrevX = static_cast<float>(x);
    cursorPrevY = static_cast<float>(y);
}

const Camera& Player::getCamera() const
{
    return camera;
}

const glm::vec3 Player::getPosition() const
{
    // return position;
    return camera.getEye();
}

const unsigned Player::getRenderDistance() const
{
    return renderDistance;
}

void Player::addMoveCallback(const std::function<void(Player&)>& callback)
{
    moveCallbacks.push_back(callback);
}

void Player::clearMoveCallbacks()
{
    moveCallbacks.clear();
}
