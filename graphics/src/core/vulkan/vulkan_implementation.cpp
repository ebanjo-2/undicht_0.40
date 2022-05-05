// this might be stupid

// vulkan.hpp is a really big file to include
// compiling it takes forever and can take a lot of ram

// this is the only include of vulkan.hpp in the project that does anything
// (other includes are only there to convince the ide that everything is fine)
#include "vulkan_pch.hpp"

// source files that need vulkan.hpp
// dont add these to the project!!!

// general files
#include "graphics_types.cpp"

// core files
#include "core/vulkan/graphics_api.cpp"
#include "core/vulkan/graphics_device.cpp"
#include "core/vulkan/graphics_surface.cpp"
#include "core/vulkan/swap_chain.cpp"

// graphics pipeline files
#include "graphics_pipeline/vulkan/shader.cpp"
#include "graphics_pipeline/vulkan/renderer.cpp"
#include "graphics_pipeline/vulkan/render_subpass.cpp"
#include "graphics_pipeline/vulkan/vram_buffer.cpp"
#include "graphics_pipeline/vulkan/vertex_buffer.cpp"
#include "graphics_pipeline/vulkan/uniform_buffer.cpp"
#include "graphics_pipeline/vulkan/texture.cpp"
