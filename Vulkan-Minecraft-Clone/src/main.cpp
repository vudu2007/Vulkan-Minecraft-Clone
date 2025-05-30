#include <iostream>

#include "engine/physics/collision-handler.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/physics/shapes/box.hpp"
#include "game.hpp"

int main()
{
    Box3d box(glm::vec3(0), glm::vec3(1));
    Ray ray(glm::vec3(0), glm::vec3(1), 0.0f, 1.0f);
    CollisionHandler::shapeRayIntersect(box, ray);

    Game game;

    try
    {
        game.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
