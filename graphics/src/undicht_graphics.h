#ifndef UNDICHT_GRAPHICS_H
#define UNDICHT_GRAPHICS_H

#include "config.h"
#include "graphics_types.h"


#ifdef USE_GLFW
#include "window/glfw/window_api.h"
#include "window/glfw/window.h"
#include "window/glfw/monitor.h"
#endif // USE_GLFW

#ifdef USE_VULKAN
#include "core/vulkan/graphics_api.h"
#include "core/vulkan/graphics_device.h"
#include "core/vulkan/graphics_surface.h" 
#include "core/vulkan/swap_chain.h"

#include "graphics_pipeline/vulkan/shader.h"
#include "graphics_pipeline/vulkan/renderer.h"
#include "graphics_pipeline/vulkan/pipeline.h"
#include "graphics_pipeline/vulkan/vertex_buffer.h"
#include "graphics_pipeline/vulkan/uniform_buffer.h"
#include "graphics_pipeline/vulkan/texture.h"
#endif // USE_VULKAN

#include "user_interface/font.h"
#include "user_interface/font_renderer.h"

#endif // UNDICHT_GRAPHICS_H
