#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// Global variables
extern VkDeviceSize destination_offset;
extern VkResult resultcmd;

extern VkCommandPool command_pool;

extern VkSemaphore renderfinishedsemaphore;

extern VkCommandBuffer command_buffer;
extern VkCommandBuffer staging_command_buffer;

extern std::vector<VkCommandBuffer> command_buffers;
extern std::vector<VkCommandBuffer> staging_command_buffers;

extern VkCommandBufferUsageFlags usage;

extern std::vector<VkCommandBufferInheritanceInfo>
    secondary_command_buffer_info;

extern std::vector<VkSemaphore> waitsemaphores;


// Function declarations
bool creatingcommand();
bool renderloop();
