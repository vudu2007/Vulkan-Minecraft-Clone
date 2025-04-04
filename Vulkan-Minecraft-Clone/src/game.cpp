#include "game.hpp"

#include <iostream>

void Game::run()
{
    while (!window.shouldClose())
    {
        window.pollEvents();

        // TODO: game logic here

        renderer.drawFrame();
    }
}
