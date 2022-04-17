#ifndef UNDICHT_GRAPHICS_H
#define UNDICHT_GRAPHICS_H

// at some point these will be set by cmake
#define USE_GLFW true
#define USE_VULKAN true


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
#endif // USE_VULKAN



#endif // UNDICHT_GRAPHICS_H
