// this might be stupid

// vulkan.hpp is a really big file to include
// compiling it takes forever and can take a lot of ram

// this is the only include of vulkan.hpp in the project that does anything
// (other includes are only there to convince the ide that everything is fine)
#include "vulkan/vulkan.hpp"

// source files that need vulkan.hpp
// dont add these to the project!!!
#include "core/vulkan/graphics_api.cpp"
#include "core/vulkan/graphics_device.cpp"