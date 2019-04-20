#include "VulkanWrapper2.hpp"

#include "SDL.h"
#include "SDL_vulkan.h"

#include <iostream>

const int kDefWindowWidth = 1024;
const int kDefWindowHeight = 1024;

//---

Vulkan::Instance::Instance(const std::string &app_name,
	std::unique_ptr<DeviceSelectionRule::Base> deviceSelectionRule)
{
	// create SDL window with Vulkan support; this must be done first
	// (we need the window for querying Vulkan extensions for drawing onto it)
	window.reset(
		new SDL::Window(
			app_name,
			kDefWindowWidth, kDefWindowHeight)
	);

	std::vector<const char*> extensions = listNeededSDLExtensions();

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
		.enabledExtensionCount = uint32_t(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	if (vkCreateInstance(&inst_info, nullptr, &instance) != 0) {
		throw Vulkan::Error("vkCreateInstance() failed (incompatible driver or not enough memory?)");
	}

	std::cout << "vulkan: instance created" << std::endl;

	if (!SDL_Vulkan_CreateSurface(window->unwrap(), instance, &surface)) {
		throw Vulkan::Error("SDL_Vulkan_CreateSurface() failed");
	}

	std::cout << "vulkan: surface created" << std::endl;

	// fetch list of GPUs
	std::vector<VkPhysicalDevice> gpus = listPhysicalDevices();
	std::cout << "vulkan: detected " << gpus.size() << " physical device(s)" << std::endl;

	// look at properties of each GPU and choose one
	int gpuSelected = -1;
	std::cout << "vulkan: choosing GPU device (" << deviceSelectionRule->describe() << ")" << std::endl;
	for (int i = 0; i < gpus.size(); i++) {
		VkPhysicalDevice& gpu = gpus[i];
		VkPhysicalDeviceProperties gpuProperties;
		VkPhysicalDeviceFeatures gpuFeatures;
		vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
		vkGetPhysicalDeviceFeatures(gpu, &gpuFeatures);
		std::cout << "vulkan: device #" << i << ": " << gpuProperties.deviceName << std::endl;
		std::cout << "vulkan:    - API supported: "
			<< VK_VERSION_MAJOR(gpuProperties.apiVersion) << "."
			<< VK_VERSION_MINOR(gpuProperties.apiVersion) << "."
			<< VK_VERSION_PATCH(gpuProperties.apiVersion) << std::endl;
		std::cout << "vulkan:    - max image dimensions (1D, 2D, 3D): "
			<< gpuProperties.limits.maxImageDimension1D << ", "
			<< gpuProperties.limits.maxImageDimension2D << ", "
			<< gpuProperties.limits.maxImageDimension3D << std::endl;

		auto queueFamilies = listQueueFamilies(gpu);
		std::cout << "vulkan:    - queue families: " << queueFamilies.size() << std::endl;
		int queueIndex = 0;
		for (auto family : queueFamilies) {
			std::cout << "vulkan:        #" << queueIndex << " (" << family.queuesAvailable << " queues available):";
			if (family.canDoGraphics) {
				std::cout << " gfx";

				// if we don't yet have a queue family for graphics, use this
				if (gfxQueueFamily < 0)
					gfxQueueFamily = family.index;

				// if this queue can also do presentation, and we don't yet have another
				// one for this, use this as well
				if (family.canDoPresentation) {
					if (preQueueFamily < 0)
						preQueueFamily = family.index;
				}
			}
			if (family.canDoComputation) {
				std::cout << " compute";
			}
			if (family.canDoTransfers) {
				std::cout << " xfer";
			}
			if (family.canDoSparseBinding) {
				std::cout << " sparse";
			}
			if (family.canDoPresentation) {
				std::cout << " present";
			}
			std::cout << std::endl;
		}

		if (gfxQueueFamily < 0) {
			std::cout << "vulkan:    - device not usable: no gfx queues found" << std::endl;
		}
		else if (preQueueFamily < 0) {
			std::cout << "vulkan:    - device not usable: no presentation queues found" << std::endl;
		}
		else if (deviceSelectionRule->isSuitable(gpuProperties)) {
			gpuSelected = i;
			std::cout << "vulkan:    - device selected" << std::endl;
			std::cout << "vulkan:    - will use queue family #" << gfxQueueFamily << " for graphics" << std::endl;
			std::cout << "vulkan:    - will use queue family #" << preQueueFamily << " for presentation" << std::endl;
			break;
		}
	}

	if (gpuSelected < 0) {
		vkDestroyInstance(instance, nullptr);
		throw Vulkan::Error("could not find suitable GPU device");
	}

	// set up queues we want
	float queuePriorities[1] = {0.0};
	VkDeviceQueueCreateInfo queueInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = nullptr,
		.queueCount = 1,
		.pQueuePriorities = queuePriorities
	};

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = nullptr
	};
}

//---

Vulkan::Instance::~Instance()
{
	if (instance) {
		vkDestroyInstance(instance, nullptr);
	}
}

//---

std::vector<VkPhysicalDevice> Vulkan::Instance::listPhysicalDevices()
{
	uint32_t gpuCount = 0;
	if (0 != vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr)) {
		throw Vulkan::Error("vkEnumeratePhysicalDevices() failed");
	}

	std::vector<VkPhysicalDevice> gpus;

	if (gpuCount > 0) {
		gpus.resize(gpuCount);
		if (0 != vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data())) {
			throw Vulkan::Error("vkEnumeratePhysicalDevices() failed (2nd)");
		}
	}

	return gpus;
}

//---

std::vector<Vulkan::QueueFamily> Vulkan::Instance::listQueueFamilies(VkPhysicalDevice phy)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phy, &queueFamilyCount, 0);

	std::cout << "vulkan:    - detected " << queueFamilyCount << " queue families" << std::endl;

	std::vector<VkQueueFamilyProperties> families;

	if (queueFamilyCount > 0) {
		families.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phy, &queueFamilyCount, families.data());
	}

	std::vector<Vulkan::QueueFamily> familiesTranslated;
	int familyIndex = 0;
	for (auto f : families) {

		// identify and store the queue family properties
		Vulkan::QueueFamily family;
		family.index = familyIndex;
		family.canDoGraphics = (f.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
		family.canDoComputation = (f.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
		family.canDoTransfers = (f.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;
		family.canDoSparseBinding = (f.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0;

		// check if presentation to our surface is also supported on this queue
		VkBool32 supportsPresentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(phy, familyIndex, surface, &supportsPresentation);
		family.canDoPresentation = supportsPresentation;

		familiesTranslated.push_back(family);

		familyIndex++;
	}

	return familiesTranslated;	
}

//---

std::vector<const char*> Vulkan::Instance::listNeededSDLExtensions()
{
	unsigned int count = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(window->unwrap(), &count, nullptr)) {
		throw Vulkan::Error("SDL_Vulkan_GetInstanceExtensions() failed: " + std::string(SDL_GetError()));
	}

	std::vector<const char*> extensions = {
	    /* VK_EXT_DEBUG_REPORT_EXTENSION_NAME // Sample additional extension */
	};
	size_t additionalExtensionCount = extensions.size();
	extensions.resize(additionalExtensionCount + count);
	if (!SDL_Vulkan_GetInstanceExtensions(window->unwrap(), &count, extensions.data())) {
		throw Vulkan::Error("SDL_Vulkan_GetInstanceExtensions() failed: " + std::string(SDL_GetError()));
	}

	std::cout << "vulkan: requesting extensions:" << std::endl;
	for (auto ex : extensions) {
		std::cout << "vulkan:    - " << ex << std::endl;
	}

	return extensions;
}
