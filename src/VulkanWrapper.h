#pragma once

#include <string>
#include <vector>

#include "vulkan/vulkan.h"

namespace Vulkan {

class Instance {
public:

	Instance(const std::string& appName_);
	~Instance();
	bool Ok() const { return ok; }
	VkInstance& Unwrap() { return wrapped; }

protected:

	std::string appName;
	bool ok = false;
	VkInstance wrapped;
};

class GpuTable {
public:

	GpuTable(Instance& instance);
	~GpuTable();
	bool Ok() const { return ok; }
	size_t size() const { return gpus.size(); }
	VkPhysicalDevice& at(int index) { return gpus.at(index); }

protected:

	bool ok = false;
	std::vector<VkPhysicalDevice> gpus;
};

};
