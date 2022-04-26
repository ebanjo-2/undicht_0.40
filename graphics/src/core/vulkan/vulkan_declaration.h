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

	// Unique* types
	template<typename t1, typename t2>
	class UniqueHandle;
	class DispatchLoaderStatic;

    class SurfaceKHR;
	
	typedef UniqueHandle<SurfaceKHR, DispatchLoaderStatic> UniqueSurfaceKHR;

}

#endif // VULKAN_DECLARATION_H
