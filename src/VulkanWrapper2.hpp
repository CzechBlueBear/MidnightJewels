#pragma once

#include "vulkan/vulkan.h"

#include "SDLWrapper.h"

#include <list>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

namespace Vulkan {

class Error : public std::runtime_error
{
public:

	Error(const std::string &what) : std::runtime_error(what) {}
};

//---

class QueueFamily
{
public:

	int index = -1;
	bool canDoGraphics = false;
	bool canDoTransfers = false;
	bool canDoComputation = false;
	bool canDoSparseBinding = false;
	bool canDoPresentation = false;
	uint32_t queuesAvailable = 0;
};

//---

/// Physical device (GPU).
class PhysicalDevice
{
public:

	bool is_discrete_gpu;
	bool is_integrated_gpu;
	bool is_logical_gpu;
	std::vector<QueueFamily> queue_families;
};

//---

/// For choosing a hardware device if there are multiple.
namespace DeviceSelectionRule {

	class Base
	{
	public:

		virtual std::string describe() const = 0;
		virtual bool isSuitable(const VkPhysicalDeviceProperties& properties) = 0;
	};

	/// Simply uses the first device that is detected.
	class AnythingIsFine : public Base
	{
	public:

		std::string describe() const { return "anything is fine"; }
		bool isSuitable(const VkPhysicalDeviceProperties& properties) { return true; }
	};

}

//---

class VkInstanceDeleter
{
public:

	void operator()(VkInstance instance) { vkDestroyInstance(instance, nullptr); }
};

//---

class Instance
{
public:

	Instance(const std::string &app_name,
		std::unique_ptr<DeviceSelectionRule::Base> deviceSelectionRule = std::make_unique<DeviceSelectionRule::AnythingIsFine>());
	~Instance();

protected:

	std::unique_ptr<SDL::Window> window;
	VkInstance instance;
	VkSurfaceKHR surface;
	std::vector<PhysicalDevice> physicalDevices;

	int gfxQueueFamily = -1;	///< queue family used for graphics
	int preQueueFamily = -1;	///< queue family used for presentation

	std::vector<VkPhysicalDevice> listPhysicalDevices();
	std::vector<Vulkan::QueueFamily> listQueueFamilies(VkPhysicalDevice phy);
	std::vector<const char*> listNeededSDLExtensions();
};

//---

class LogicalDevice
{
public:

	LogicalDevice(Instance& instance, PhysicalDevice &phy);
};

};
