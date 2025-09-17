# Vulkan Minecraft Clone
An exploratory project on the Vulkan API (using v.1.1 features) by implementing a Minecraft like game as the goal.  
As of now, the project has only been tested on Windows 11 using Visual Studio 2022.  

## Controls
- `W`/`A`/`S`/`D` to move.
- `Left-Shift` to descend.
- `Space` to ascend.
- Hold `Left-Control` to speed up.
- `Left-Alt` to toggle the cursor.
- `F1` to cycle "game modes".

## Dependencies being used (don't need to install)
- [BS Thread Pool](https://github.com/bshoshany/thread-pool) (v.5.0.0)
- [FastNoiseLite](https://github.com/Auburn/FastNoiseLite) (v.1.1.1)
- [GLFW](https://www.glfw.org/) (v.3.4)
- [GLM](https://github.com/g-truc/glm) (v.1.0.1)
- [stb_image](https://github.com/nothings/stb) (v.2.30)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) (v.2.0.0)
- [volk](https://github.com/zeux/volk) (v.1.4.304)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) (n/a)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) (v.3.3.0)

## Optional dependencies to install for development
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (tested with v.1.4.303)

## Build and Run with CMake
Make sure your compiler supports `C++20`!  
Optionally, install the Vulkan SDK (tested with v.1.4.303) for development tools, such as validation layers.  

(**1**) Compile the shaders.  
Go to the shaders folder, currently in `Vulkan-Minecraft-Clone/assets/shaders/compiler/` and compile the shaders.  
You can use `glslc` (included in repository as a Windows executable, but can use one in Vulkan SDK if you do not trust the one here); if you are on Windows, you can use the batch file also located there: `Vulkan-Minecraft-Clone/assets/shaders/compiler/compile.bat`.  

(**2**) Build and run it.  
In the top-level directory with your default generator (optionally, specify with `-G` flag with a generator to choose one):
```
cmake -B build
```  
This will create a build directory and will contain the files needed for you to build with respect to the generator chosen.  
For example, to build,  
- with Visual Studio, there will be a solution file that you can open and build;
- with Ninja, you'll run `Ninja -C build all` in the top-level directory and it will make an executable in the build directory.

## Resources
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Learn OpenGL](https://learnopengl.com/)
- [Brendan Galea - Vulkan (c++) Game Engine Tutorials Playlist](https://youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&si=4Qpm8svqKWqGVDzK)
- [BrendanL.K - Swept AABB Collision](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/)
- [Kentom Hamaluik - Swept AABB Minkowski](https://blog.hamaluik.ca/posts/swept-aabb-collision-using-minkowski-difference/)
