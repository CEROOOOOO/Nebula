#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>
extern VkDescriptorSetLayout descriptorsetlayout;
extern VkDescriptorSet *descriptorsets;
bool setupdescriptorpool();
bool createDescriptorSetLayout();
bool setupdescriptorset();
void descriptor_cleanup();