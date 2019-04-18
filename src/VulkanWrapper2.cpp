#include "VulkanWrapper2.hpp"

#include <iostream>

//---

Vulkan::Instance::Instance(const std::string &app_name)
{
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = app_name.c_str(),
		.applicationVersion = 1,
		.pEngineName = app_name.c_str(),
		.engineVersion = 1,
		.apiVersion = VK_API_VERSION_1_1
	};
	VkInstanceCreateInfo inst_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
	};

	if (0 != vkCreateInstance(&inst_info, nullptr, &instance)) {
		throw Vulkan::Error("vkCreateInstance() failed (incompatible driver or not enough memory?)");
	}

	std::cout << "vulkan: instance created" << std::endl;

	// how many GPUs are available
	uint32_t gpu_count = 1;
	if (0 != vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr)) {
		vkDestroyInstance(instance, nullptr);
		throw Vulkan::Error("vkEnumeratePhysicalDevices() failed");
	}

	std::cout << "vulkan: detected " << gpu_count << " physical device(s)" << std::endl;

	if (gpu_count == 0) {
		vkDestroyInstance(instance, nullptr);
		throw Vulkan::Error("no physical devices detected supporting Vulkan 1.1+");
	}

	// fetch list of GPUs
	std::vector<VkPhysicalDevice> gpus;
	gpus.resize(gpu_count);
	if (0 != vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data())) {
		vkDestroyInstance(instance, nullptr);
		throw Vulkan::Error("vkEnumeratePhysicalDevices() failed");
	}

	// which GPU we will select to use (assume the first one for now)
	int gpu_selected = 0;

	// look at properties of each GPU
	for (int i = 0; i < gpus.size(); i++) {
		VkPhysicalDevice& gpu = gpus[i];
		VkPhysicalDeviceProperties gpu_properties;
		vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
		std::cout << "vulkan: device #" << i << ": " << gpu_properties.deviceName << std::endl;
		std::cout << "vulkan:    max image dimensions (1D, 2D, 3D): "
			<< gpu_properties.limits.maxImageDimension1D << ", "
			<< gpu_properties.limits.maxImageDimension2D << ", "
			<< gpu_properties.limits.maxImageDimension3D << std::endl;
	}
}

//---

Vulkan::Instance::~Instance()
{
	if (instance) {
		vkDestroyInstance(instance, nullptr);
	}
}

