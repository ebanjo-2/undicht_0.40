#ifndef VULKAN_DECLARATION_H
#define VULKAN_DECLARATION_H

// forward declaration of vulkan types
// (so you dont have to include vulkan.hpp)
namespace vk {

	// "normal" types
    class Instance;
    class Device;
    class PhysicalDevice;
	class Queue;
	class SwapchainKHR;
	class SwapchainCreateInfoKHR;
	class SurfaceCapabilitiesKHR;
	class SurfaceFormatKHR;
	enum class PresentModeKHR;
	class Extent2D;
	class Extent3D;
	class Image;
	class ImageView;
	class ShaderModule;
	class PipelineShaderStageCreateInfo;
	class PipelineLayout;
	class AttachmentDescription;
	class AttachmentReference;
	class SubpassDescription;
	class RenderPass;
	class Pipeline;
	class Framebuffer;
	class CommandPool;
	class CommandBuffer;
	class Semaphore;
	class Fence;
    class VertexInputBindingDescription;
    class VertexInputAttributeDescription;
    enum class Format;
    class PipelineVertexInputStateCreateInfo;
    class Buffer;
    class DeviceMemory;
    enum class BufferUsageFlagBits : unsigned int;
    class MemoryType;
    class Viewport;

    // KHR extension
    class SurfaceKHR;
	

}

#endif // VULKAN_DECLARATION_H
