# IMPORTANT: if you need to define a anything, make a .env.cmake file in the top level directory and
# define it there.
include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}.")

####################################################################################################
# (0) Required initial setup.
cmake_minimum_required(VERSION 3.31.0)
message(STATUS "Using CMake version: ${CMAKE_VERSION}.")

project(Vulkan-Minecraft-Clone CXX)


####################################################################################################
# (1) Make sure dependencies are installed.
# (1.a) Vulkan.
if(DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIB "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
    message(STATUS "Found Vulkan: using variable `VULKAN_SDK_PATH`.")
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}.")
    set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
    set(Vulkan_LIB "$ENV{VULKAN_SDK}/Lib")
endif()
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using Vulkan library at: ${Vulkan_LIB}.")
endif()

# (1.b) GLFW.
if(DEFINED GLFW_PATH)
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
    if (MSVC)
        if (GLFW_MSVC_YEAR STREQUAL "2022")
            set(GLFW_LIB "${GLFW_PATH}/lib-vc2022")
        else()
            message(FATAL_ERROR "`GLFW_PATH` was defined and found `MSVC`, however, `GLFW_MSVC_YEAR` not set/supported!")
        endif()
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        message(STATUS "USING MINGW")
        set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
    else()
        message(FATAL_ERROR "`GLFW_PATH` was defined, however, not sure which library to use!")
    endif()
else()
    find_package(glfw3 3.4 REQUIRED)
    set(GLFW_LIB glfw)
    message(STATUS "Found GLFW.")
endif()
if(NOT GLFW_LIB)
    message(FATAL_ERROR "Could not find GLFW library!")
else()
    message(STATUS "Using GLFW library at: ${GLFW_LIB}.")
endif()

# (1.*) Should be available in directory `external`.
include_directories(external)
set(FAST_NOISE_LITE_PATH external/FastNoiseLite)
set(GLM_PATH external/glm-1.0.1/Include)
set(STB_PATH external/stb)
set(BS_THREAD_POOL_PATH external/thread-pool-5.0.0)
set(TINYOBJ_PATH external/tinyobjloader)
set(VMA external/VulkanMemoryAllocator-3.3.0)


####################################################################################################
# (2) Prepare them for a build.
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/src/*.hpp)

foreach(FILE_PATH ${SOURCES} ${HEADERS})
    # Get the relative path to the file from the project root.
    # E.g., ${FILE_PATH} is `project/src/main.cpp` and ${CMAKE_SOURCE_DIR} is `project/`, then it
    # will set ${REL_PATH} to `src/main.cpp`.
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${FILE_PATH}")

    # Get the directory part of the relative path.
    # E.g., using the previous comment, it takes ${REL_PATH} and sets ${GROUP_PATH} to `src/`.
    get_filename_component(GROUP_PATH "${REL_PATH}" PATH)

    source_group("${GROUP_PATH}" FILES "${FILE_PATH}")
endforeach()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

if(WIN32)
    message(STATUS "Creating a build for Windows.")
    
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${Vulkan_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
        ${GLM_PATH}
        ${TINYOBJ_PATH}
        ${FAST_NOISE_LITE_PATH}
        ${STB_PATH}
        ${BS_THREAD_POOL_PATH}
        ${VMA}
    )
    target_link_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_LIB}
        ${GLFW_LIB}
    )
    target_link_libraries(${PROJECT_NAME} vulkan-1 glfw3)
else()
    message(FATAL_ERROR "Currently, CMake not configured for current target system!")
endif()
