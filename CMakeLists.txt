####################################################################################################
# (0) Required initial setup.
cmake_minimum_required(VERSION 3.31.0)
message(STATUS "Using CMake version: ${CMAKE_VERSION}.")

project(Vulkan-Minecraft-Clone CXX)


####################################################################################################
# (1) Make sure dependencies are installed.
# (1.a) Vulkan.
set(VULKAN_HEADERS_PATH external/Vulkan-Headers)
add_subdirectory(${VULKAN_HEADERS_PATH})

set(VOLK_PATH external/volk-1.4.304)
add_subdirectory(${VOLK_PATH})
target_link_libraries(volk PUBLIC Vulkan::Headers)

# (1.b) GLFW.
set(GLFW_PATH external/glfw-3.4)
add_subdirectory(${GLFW_PATH})

# (1.*) Everything else in `external`.
include_directories(external)
set(FAST_NOISE_LITE_PATH external/FastNoiseLite)
set(GLM_PATH external/glm-1.0.1/Include)
set(STB_PATH external/stb)
set(BS_THREAD_POOL_PATH external/thread-pool-5.0.0)
set(TINYOBJ_PATH external/tinyobjloader)
set(VMA_PATH external/VulkanMemoryAllocator-3.3.0)

set(ALL_EXTERNAL_PATHS 
    ${FAST_NOISE_LITE_PATH}
    ${GLM_PATH}
    ${STB_PATH}
    ${BS_THREAD_POOL_PATH}
    ${TINYOBJ_PATH}
    ${VMA_PATH}
)

####################################################################################################
# (2) Prepare them for a build.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/assets/*)
set(TOP_LEVEL_DOCS
    ${PROJECT_SOURCE_DIR}/README.md
    ${PROJECT_SOURCE_DIR}/.gitignore
    ${PROJECT_SOURCE_DIR}/.gitattributes
)

# Create a directory structure for IDE.
source_group(
    TREE ${PROJECT_SOURCE_DIR}
    FILES ${TOP_LEVEL_DOCS}
)
source_group(
    TREE ${PROJECT_SOURCE_DIR}/src
    PREFIX "src"
    FILES ${SOURCES} ${HEADERS}
)
source_group(
    TREE ${PROJECT_SOURCE_DIR}/assets
    PREFIX "assets"
    FILES ${ASSETS}
)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${HEADERS}
)

target_sources(${PROJECT_NAME} PRIVATE
    ${TOP_LEVEL_DOCS}
    ${ASSETS}
)
set_source_files_properties(
    ${ASSETS} 
    ${TOP_LEVEL_DOCS} 
    PROPERTIES
        HEADER_FILE_ONLY TRUE
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/x64/")
set_property(DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

if(WIN32)
    message(STATUS "Creating build for Windows.")
    
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${ALL_EXTERNAL_PATHS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw volk)
elseif (UNIX)
    message(STATUS "Creating build for Unix.")

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${ALL_EXTERNAL_PATHS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw volk)
else()
    message(FATAL_ERROR "Currently, CMake not configured for current target system!")
endif()
