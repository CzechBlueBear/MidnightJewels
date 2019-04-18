#pragma once

#include "vulkan/vulkan.h"

#include <list>
#include <vector>
#include <string>
#include <stdexcept>

namespace Vulkan {

class Error : public std::runtime_error
{
public:

	Error(const std::string &what) : std::runtime_error(what) {}
};

typedef int QueueFamilyIndex;

class QueueFamily
{
public:

	QueueFamilyIndex index_in_phy;
	bool can_do_graphics;
	bool can_do_transfers;
	bool can_do_compute;
	bool can_do_sparse_bindings;
	uint32_t count;
	uint32_t timestamp_valid_bits;
	VkExtent3D min_image_transfer_granularity;
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

class Instance
{
public:

	Instance(const std::string &app_name);
	~Instance();

	std::vector<PhysicalDevice> physical_devices;

protected:

	VkInstance instance;
};

//---

class LogicalDevice
{
public:

	LogicalDevice(Instance& instance, PhysicalDevice &phy);
};

};
