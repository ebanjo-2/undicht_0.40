
find_package(Vulkan REQUIRED)

set(GRAPHICS_CORE_SOURCES

    src/core/vulkan/vulkan_pch.hpp

    src/core/vulkan/vulkan_declaration.h
    src/core/vulkan/vulkan_implementation.cpp

    src/core/vulkan/graphics_api.h
    src/core/vulkan/graphics_device.h
)

set(GRAPHICS_WINDOW_SOURCES

    src/window/glfw/window_api.h
    src/window/glfw/window_api.cpp

    src/window/glfw/monitor.h
    src/window/glfw/monitor.cpp

    src/window/glfw/window.h
    src/window/glfw/window.cpp
)

add_library(graphics

    src/undicht_graphics.h

    ${GRAPHICS_WINDOW_SOURCES}
    ${GRAPHICS_CORE_SOURCES}
    ${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.hpp
)

target_include_directories(graphics PUBLIC src)

# linking undicht core
target_link_libraries(graphics core)

# linking glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)
target_link_libraries(graphics glfw)

# linking vulkan
target_include_directories(graphics PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(graphics ${Vulkan_LIBRARIES})
target_precompile_headers(graphics PUBLIC "src/core/vulkan/vulkan_pch.hpp")
