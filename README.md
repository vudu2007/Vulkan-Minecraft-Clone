# Vulkan Minecraft Clone
An exploratory project on the Vulkan API (using v.1.0 features) by implementing a Minecraft like game as the goal.<br>
As of now, the project has only been tested on Windows 11 using Visual Studio 2022.

## Controls
- `W`/`A`/`S`/`D` to move.
- `Left-Shift` to descend.
- `Space` to ascend.
- Hold `Left-Control` to speed up.
- `Left-Alt` to toggle the cursor.
- `F1` to cycle "game modes".

## Dependencies to install
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (tested with v.1.4.303)
- [GLFW](https://www.glfw.org/) (tested with v.3.4)

## Libraries being used (don't need to install)
- [BS Thread Pool](https://github.com/bshoshany/thread-pool) (v.5.0.0)
- [FastNoiseLite](https://github.com/Auburn/FastNoiseLite) (v.1.1.1)
- [GLM](https://github.com/g-truc/glm) (v.1.0.1)
- [stb_image](https://github.com/nothings/stb) (v.2.30)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) (v.2.0.0)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) (v.3.3.0)

## Build with CMake
1. (a) Optionally, create a `.env.cmake` file in the top-level directory of this repository and define paths and related (they should be **STRINGS**) for CMake to use; some variables to take note of `VULKAN_SDK_PATH`, `GLFW_PATH`, `GLFW_MSVC_YEAR`.  
(b) Otherwise, the `CMakeLists.txt` will use `find_package(...)` to locate your dependencies and use the corresponding environment variable if you installed them that way.
2. Make sure your compiler supports C++20.
3. To build, make sure to create a build directory and use a generator of your choice. Here, I will list the steps for using the `Visual Studio 17 2022` as the generator and `Visual Studio 2022 IDE` on Windows. 
    * (CLI): Run in top-level directory.
        * `cmake -B build -G "Visual Studio 17 2022"`.
        * `cd ./build/`.
        * `start Vulkan-Minecraft-Clone.sln`.
        * Set `Vulkan-Minecraft-Clone` as the startup project and build it.
    * (GUI):
        * Set the source code as the top-level directory.
        * Create a `build` directory in the top-level directory and set that as where the binaries will be.
        * Click `Configure` and select the generator and related (here use `Visual Studio 17 2022`), and hit `Finish`.
        * Click `Configure` again, to remove the red lines.
        * Click `Generate`.
        * Click `Open Project`, which will open it in the respective Visual Studio.
        * Set `Vulkan-Minecraft-Clone` as the startup project and build it.
4. Go to the shaders folder, currently in `Vulkan-Minecraft-Clone/src/shaders/compiler/`
5. Compile the shaders, you can use `glslc` (included in repository as an Windows executable, but can use one in Vulkan SDK if you do not trust the one here); if you are on Windows, you can use the batch file also located there: `Vulkan-Minecraft-Clone/src/shaders/compiler/compile.bat`

## Resources
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Learn OpenGL](https://learnopengl.com/)
- [Brendan Galea - Vulkan (c++) Game Engine Tutorials Playlist](https://youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&si=4Qpm8svqKWqGVDzK)
- [BrendanL.K - Swept AABB Collision](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/)
- [Kentom Hamaluik - Swept AABB Minkowski](https://blog.hamaluik.ca/posts/swept-aabb-collision-using-minkowski-difference/)
