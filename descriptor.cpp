#include <vulkan/vulkan.h>
#include <stdio.h>
#include "device.h"
#include "buffer.h"
#include "present.h"
#include <string.h>
VkDescriptorSetLayout descriptorsetlayout; // layout i think
VkPipelineLayout pipelineLayout;           // pipeline layout i think
VkDescriptorPool descriptorpool;
std :: vector <VkDescriptorSet> descriptorsets;
std :: vector < VkDescriptorSetLayout > layout;
std :: vector < VkDescriptorSetLayoutBinding > bindings;
std :: vector < VkWriteDescriptorSet > descriptorWrites;
std :: vector < VkDescriptorPoolSize > poolSizes;
bool setupdescriptorpool()
{
    // VkDescriptorPoolSize *poolSizes;
    poolSizes.resize(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = images_count; // max frames in flight
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = images_count;
    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = &poolSizes[0];
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
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;                                        // location to say and use it in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // its for uniform buffer
    uboLayoutBinding.descriptorCount = 1;                                // like only one ubo na
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional
    // now for image sampling
    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;//this is location fragment shader has 
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // VkDescriptorSetLayoutBinding *bindings;
    //  storing this in array beacause vulkan doesnt accept two setlayouts idk why
    bindings.resize(2);
    bindings[0] = uboLayoutBinding;
    bindings[1] = samplerLayoutBinding;
    // now this is last piece to create a set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = &bindings[0];
    if (vkCreateDescriptorSetLayout(logical_device, &layoutInfo, NULL, &descriptorsetlayout) != VK_SUCCESS)
    {
        printf("failed to create descriptor set layout!");
        return false;
    }

    return true;
}
bool setupdescriptorset()
{                                                                  //                               // so i eve
                                                                   //                         n need layouts to be in array to give seets array
    
  layout.resize(images_count);
    for (int i = 0; i < images_count; i++)
    {
        layout[i] = descriptorsetlayout;
    }
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorpool;
    allocInfo.descriptorSetCount = images_count;
    allocInfo.pSetLayouts = &layout[0];
    descriptorsets.resize(images_count);
    if (vkAllocateDescriptorSets(logical_device, &allocInfo, &descriptorsets[0]) != VK_SUCCESS)
    {
        printf("cant create descriptor sets\n");
    }
    for (size_t i = 0; i < images_count; i++)
    {
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = uniform_buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ubox);
        VkDescriptorImageInfo imageInfo {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = image_destination_view;
        imageInfo.sampler = image_destination_sampler;
        descriptorWrites.resize(3);
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].pNext = NULL;
        descriptorWrites[0].dstSet = descriptorsets[i];
        descriptorWrites[0].dstBinding = 0; // it says here wtf it cant be posssible
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].pNext = NULL;
        descriptorWrites[1].dstSet = descriptorsets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(logical_device, 2, &descriptorWrites[0], 0, NULL);
    }
    return true;
}
