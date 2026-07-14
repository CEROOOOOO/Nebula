#include <vulkan/vulkan.h>
#include <stdio.h>
#include "device.h"
#include "buffer.h"
#include "present.h"
VkDescriptorSetLayout descriptorsetlayout; // layout i think
VkPipelineLayout pipelineLayout;           // pipeline layout i think
VkDescriptorPool descriptorpool;
VkDescriptorSet *descriptorsets;
bool setupdescriptorpool()
{
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = images_count;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = images_count;
    if (vkCreateDescriptorPool(logical_device, &poolInfo, NULL, &descriptorpool) != VK_SUCCESS)
    {
        printf("Error creating descriptor pool \n");
        return false;
    }
    return true;
}
bool createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;                                        // location to say and use it in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // its for uniform buffer
    uboLayoutBinding.descriptorCount = 1;                                // like only one ubo na
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional
    // now the goat
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(logical_device, &layoutInfo, NULL, &descriptorsetlayout) != VK_SUCCESS)
    {
        printf("failed to create descriptor set layout!");
        return false;
    }
    return true ;
}
bool setupdescriptorset()
{
    VkDescriptorSetLayout *layout;
    layout = malloc(sizeof(VkDescriptorSetLayout) * images_count);
    for (int i = 0; i < images_count; i++)
    {
        layout[i] = descriptorsetlayout;
    }
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorpool;
    allocInfo.descriptorSetCount = images_count;
    allocInfo.pSetLayouts = &layout[0];
    descriptorsets = malloc(sizeof(VkDescriptorSet) * images_count);
    if (vkAllocateDescriptorSets(logical_device, &allocInfo, &descriptorsets[0]) != VK_SUCCESS)
    {
        printf("cant create descriptor sets\n");
    }
    for (size_t i = 0; i < images_count; i++)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = uniform_buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ubox);
        VkWriteDescriptorSet descriptorWrite = {0};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorsets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = NULL;       // Optional
        descriptorWrite.pTexelBufferView = NULL; // Optional
        vkUpdateDescriptorSets(logical_device,
                               1, &descriptorWrite,
                               0, NULL);
    }
    return true;
}
void descriptor_cleanup()
{

    vkDestroyDescriptorPool(logical_device, descriptorpool, NULL);
    vkDestroyDescriptorSetLayout(logical_device, descriptorsetlayout, NULL);
}