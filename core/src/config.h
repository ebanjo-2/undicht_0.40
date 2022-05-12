#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_MODE

#define USE_GLFW
#define USE_VULKAN

//#define USE_GLFW
//#define USE_GL33

#if defined(unix) || defined(__unix__) || defined(__unix)
#define PLATFORM_UNIX
#endif

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#endif

#endif // CONFIG_H
