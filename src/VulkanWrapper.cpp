#include "VulkanWrapper.h"

#include "SDL.h"

Vulkan::Instance::Instance(const std::string& appName_)
	: appName(appName_)
{
	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = appName.c_str(),
		.applicationVersion = 1,
		.pEngineName = appName.c_str(),
		.engineVersion = 1,
		.apiVersion = VK_API_VERSION_1_0
	};
	VkInstanceCreateInfo instInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
	};

	VkResult res = vkCreateInstance(&instInfo, NULL, &wrapped);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
		SDL_SetError("vkCreateInstance(): cannot find a compatible Vulkan ICD\n");
		return;
	} else if (res) {
		SDL_SetError("vkCreateInstance(): unknown error\n");
		return;
	}

	ok = true;
}

Vulkan::Instance::~Instance()
{
	if (wrapped)
		vkDestroyInstance(wrapped, nullptr);
}

Vulkan::GpuTable::GpuTable(Instance &instance)
{
    uint32_t gpu_count = 1;
	if (0 != vkEnumeratePhysicalDevices(instance.Unwrap(), &gpu_count, nullptr)) {
		SDL_SetError("vkEnumeratePhysicalDevices() failed");
		return;
	}

	gpus.resize(gpu_count);
	if (0 != vkEnumeratePhysicalDevices(instance.Unwrap(), &gpu_count, gpus.data())) {
		SDL_SetError("vkEnumeratePhysicalDevices() failed (2nd run)");
		return;
	}

	ok = true;
}

Vulkan::GpuTable::~GpuTable()
{

}
