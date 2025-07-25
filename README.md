# Vulkan Minecraft Clone
An exploratory project on the Vulkan API by implementing a Minecraft like game as the goal.<br>
The project has only been tested on Windows using Visual Studio 2022.

## Controls (TODO)
- `W`/`A`/`S`/`D` to move.
- `Left-Shift` to descend.
- `Space` to ascend.
- Hold `Left-Control` to speed up.
- `Left-Alt` to toggle the cursor.

## Dependencies to install
- Vulkan (tested with v.1.4.303)
- GLFW (TODO)
- GLM (TODO)

## External
- FastNoiseLite (v.1.1.1)
- stb_image (v.2.30)
- tinyobjloader (v.2.0.0)
- Vulkan Memory Allocator (v.3.3.0)

## Build and Run
1. Download dependencies and keep track of the paths to them.
2. Clone the repository.
3. Open the solution file (`Vulkan-Minecraft-Clone.sln`) in Visual Studio (should work on 2022).
4. Follow the [Vulkan Tutorial - Development Environment](https://vulkan-tutorial.com/Development_environment) to set up the development environment.
5. Make sure you are using a compiler that supports C++20.
6. Go to the shaders folder `Vulkan-Minecraft-Clone/src/shaders/compiler/`
7. Compile the shaders, such as using `glslc` (included in repository as an Windows executable, but can use one in Vulkan SDK if you do not trust the one here); can use the batch file: `Vulkan-Minecraft-Clone/src/shaders/compiler/compile.bat`
8. Use Visual Studio to build your desired build configuration and run the application.

## Resources
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Learn OpenGL](https://learnopengl.com/)
- [Brendan Galea Vulkan (c++) Game Engine Tutorials Playlist](https://youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&si=4Qpm8svqKWqGVDzK)
