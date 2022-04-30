
find_package(Vulkan REQUIRED)

# .cpp files including vulkan.hpp should not be added to this
# compiling vulkan.hpp takes a long time, so it is only included once in vulkan_implementation.cpp
# .cpp using vulkan.hpp get included (yes, including .cpp files is uncommon) in this file
set(GRAPHICS_CORE_SOURCES

    src/core/vulkan/vulkan_pch.hpp
    
    src/core/vulkan/vulkan_declaration.h
    src/core/vulkan/vulkan_implementation.cpp

    src/core/vulkan/graphics_api.h
    src/core/vulkan/graphics_device.h
    src/core/vulkan/graphics_surface.h
    src/core/vulkan/swap_chain.h
)

set(GRAPHICS_WINDOW_SOURCES

    src/window/glfw/window_api.h
    src/window/glfw/window_api.cpp

    src/window/glfw/monitor.h
    src/window/glfw/monitor.cpp

    src/window/glfw/window.h
    src/window/glfw/window.cpp
)

set(GRAPHICS_PIPELINE_SOURCES

		src/graphics_pipeline/vulkan/shader.h
		src/graphics_pipeline/vulkan/renderer.h
		src/graphics_pipeline/vulkan/render_subpass.h
		src/graphics_pipeline/vulkan/vram_buffer.h
		src/graphics_pipeline/vulkan/vertex_buffer.h

)

add_library(graphics

    src/undicht_graphics.h
    src/graphics_types.h
		
    ${GRAPHICS_WINDOW_SOURCES}
    ${GRAPHICS_CORE_SOURCES}
		${GRAPHICS_PIPELINE_SOURCES}
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
