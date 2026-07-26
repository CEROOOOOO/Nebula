#define _GNU_SOURCE
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vulkan/vulkan.h>
#include "present.h"
#include "device.h"
#include <string.h>
#include <time.h>
#include <cglm/cglm.h>
#include "objloader.h"
#include "vertex.h"
VkBuffer buffer;               // this too remove
VkBuffer uniform_texel_buffer; // dont care
VkBuffer storage_texel_buffer; // dont care
VkBuffer staging_buffer;       // sufficient
VkBuffer image_staging_buffer;
VkImage image_destination_buffer; // like its the destination for image data instead of buffer just create vkimage idk why but do as website says dont argue
VkImage depth_image;
VkBuffer staging_dst_buffer;
VkBuffer index_buffer;
VkBuffer index_staging_buffer;
VkBuffer *uniform_buffers = NULL;
VkImageView image_destination_view;
VkImageView depth_image_view;
VkSampler image_destination_sampler;
VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
VkMemoryRequirements memory_requirements;                        // memory requirements we get requirements of that memory properties
VkMemoryRequirements memory_requirements_for_staging;            // requirements i will need for staging buffer
VkMemoryRequirements memory_requirements_for_dst_staging;        // requirements i will need for staging buffer
VkMemoryRequirements memory_requirements_for_uniform_texel;      // requirements i will need for staging buffer
VkMemoryRequirements memory_requirements_for_storage_texel;      // requirements i will need for staging buffer
VkMemoryRequirements memory_requirements_for_index_staging;      // bro like this will give my cpu value to gpy its a temporary sacrifice
VkMemoryRequirements memory_requirements_fubomappedor_index_dst; // like this with some magic this will hold data in gpu
VkMemoryRequirements memory_requirements_for_uniform;
VkMemoryRequirements memory_requirements_for_index_dst;
VkMemoryRequirements memory_requirements_for_image;
VkMemoryRequirements memory_requirements_for_image_destination;
VkMemoryRequirements memory_requirements_for_depth_image;
VkDeviceMemory memory_object = VK_NULL_HANDLE;                      // idk but object should be null pointer
VkDeviceMemory memory_object_for_staging = VK_NULL_HANDLE;          // idk but object should be null pointer when we call vkallocate memory it gets populatec with everything we want like from now on we will use this
VkDeviceMemory memory_object_for_dst_staging = VK_NULL_HANDLE;      // idk but object should be null pointer
VkDeviceMemory memory_object_for_storage_image = VK_NULL_HANDLE;    // idk but object should be null pointer
VkDeviceMemory memory_object_for_attachment_image = VK_NULL_HANDLE; // idk but object should be null pointer
VkDeviceMemory memory_object_for_uniform_texel = VK_NULL_HANDLE;    // idk but object should be null pointer
VkDeviceMemory memory_object_for_storage_texel = VK_NULL_HANDLE;    // idk but object should be null pointer
VkDeviceMemory memory_object_for_index_staging;                     // bro like this will give my cpu value to gpy its a temporary sacrifice
VkDeviceMemory memory_object_for_index_dst;                         // like this with some magic this will hold data in gpu
VkDeviceMemory memory_object_for_image_staging;
VkDeviceMemory memory_object_for_image_destination;
VkDeviceMemory memory_object_for_depth_image;
VkDeviceMemory *memory_object_for_uniform = NULL;
VkMemoryPropertyFlagBits memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkMemoryPropertyFlagBits memory_property_for_staging = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
VkMemoryPropertyFlagBits memory_property_for_dst_staging = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkMemoryPropertyFlagBits memory_property_for_uniform_texel = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkMemoryPropertyFlagBits memory_property_for_storage_texel = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkMemoryPropertyFlagBits memory_property_for_index_staging = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
VkMemoryPropertyFlagBits memory_property_for_index_dst = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkMemoryPropertyFlagBits memory_property_for_uniform = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; // IDK WHYYYYYYY DONT ASK ME
VkMemoryPropertyFlagBits memory_property_for_image_staging = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
VkMemoryPropertyFlagBits memory_property_for_image_destination = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkMemoryPropertyFlagBits memory_property_for_depth_image = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
VkDeviceSize memory_offset;
VkDeviceSize memory_range;
VkFormat format = VK_FORMAT_R8G8B8A8_UNORM; // how the buffer should be viewed
VkResult resultb = VK_SUCCESS;
VkSemaphore semaphorecmd;
VkFence fencecmd;
VkCommandBuffer cmdbufferforstaging;
VkCommandPool cmdpoolforstaging;
VkVertexInputBindingDescription *binding_descriptions = NULL;
VkVertexInputAttributeDescription *attribute_descriptions = NULL;
VkPipelineStageFlags generating_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
VkPipelineStageFlags consuming_stgaes = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
VkBufferMemoryBarrier *buffer_memory_barriers = NULL;
VkDeviceSize imagesize;
uint32_t miplevels;
void **ubomapped = NULL;
void *image_data = NULL;
struct timespec starttime; // like its stake holder i dont think it will get rewrite
void initstarttime()
{
    clock_gettime(CLOCK_MONOTONIC, &starttime); // this will start the timer when called this function from 1970 so its max
}
typedef struct
{ // like from this struct we will define how we will use this barries /
    VkBuffer Buffer;
    VkAccessFlags CurrentAccess;
    VkAccessFlags NewAccess;
    uint32_t CurrentQueueFamily;
    uint32_t NewQueueFamily;
} BufferTransition;
typedef struct
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubox;
// Triangle vertices (NDC coordinates, so it shows up without any transforms)
VkBufferCopy copyregion = {0};
VkBufferCopy index_copyregion = {0};
// VkBufferCopy image_copyregion = {
//   .srcOffset = 0,
//.dstOffset = 0,
//.size = imagesize
///};i think i dont need thi si am passing it in a function which has size has size as its input as offset and dst are always 0
VkCommandBuffer *cmdbuffers = NULL;
VkMappedMemoryRange *memory_ranges = NULL;
VkMappedMemoryRange *memory_ranges_for_index = NULL;
BufferTransition *buffer_transitions = NULL;
VkDeviceSize offset = 0;
VkDeviceSize data_size;
VkDeviceSize index_data_size;
VkDeviceSize ubo_data_size;
bool datasetup()
{
    copyregion.srcOffset = 0;
    copyregion.dstOffset = 0;
    copyregion.size = sizeof(Vertex) * vertexcount;
    index_copyregion.srcOffset = 0;
    index_copyregion.dstOffset = 0;
    index_copyregion.size = sizeof(uint32_t) * indexcount; // memory region that should be mapped and packed from cpu its our source which will be from cpu
    data_size = sizeof(Vertex) * vertexcount;              // size of data to be packed from cpu to the memory object of gp
    index_data_size = sizeof(uint32_t) * indexcount;
    ubo_data_size = sizeof(ubox);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool creatingcmdforstaging()
{
    uint32_t count = 1;
    VkCommandPoolCreateFlags parameters = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        parameters,
        graphics_queue_family_index

    };
    resultb = vkCreateCommandPool(logical_device, &command_pool_create_info, NULL, &cmdpoolforstaging);
    if (VK_SUCCESS != resultb)
    {
        printf("Couldn't create command pool\n");
        return false;
    }
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {// allocating command buffer
                                                                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                                NULL,
                                                                cmdpoolforstaging,
                                                                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                count};
    cmdbuffers = malloc(count * sizeof(VkCommandBuffer));
    resultb = vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, cmdbuffers);
    if (VK_SUCCESS != resultb)
    {
        printf("couldn't create command buffer\n");
        return false;
    }
    cmdbufferforstaging = cmdbuffers[0]; // recording command buffer
    VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkCommandBufferInheritanceInfo *secondary_command_buffer_info = NULL;
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        usage,
        secondary_command_buffer_info};
    resultb = vkBeginCommandBuffer(cmdbufferforstaging, &command_buffer_begin_info);
    if (VK_SUCCESS != resultb)
    {
        printf("couldn't begin command buffer for staging\n");
        return false;
    }
    return true;
} // this creating buffer function has huge mistake when called to other use this recreates pool which is unnecessary so
// we have to options modify this code or write a new function which handles
// and obviously i will choose 2 option i dont want to harm  my rotating rectangle
VkCommandBuffer beginsingletimecommands() // yeah this called by functions which will be called from the functions which will be called in main
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdpoolforstaging;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer; // this is the buffer which is main it does things
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endsingletimecommands(VkCommandBuffer commandBuffer) // see this is taking the input command buffer which is brgin in above function
{                                                         // this is submitting function as there are no need of seemaphores this that it fairly simple    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkEndCommandBuffer(commandBuffer);
    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, cmdpoolforstaging, 1, &commandBuffer);
}
// wooh for copying you know i did expicitly in cmdbuffer.c funtion and tht in a public function but now lets simplify that step too
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) // i never saw this btw
{                                                                          // dont worry i will explain
    VkCommandBuffer commandBuffer = beginsingletimecommands();             // so see when we call this function this function return is cmd buffer so the variable takes it

    VkBufferCopy image_copyregion = {0};
    image_copyregion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &image_copyregion);

    endsingletimecommands(commandBuffer); // with this the command buffer gets ended simple
}
// after this staging done image staging buffers data is tranferred to image dst buffer but its an image not any vertex data so we cant just do the normal stuff her e
// we have to change the transition and then buffer to image
// remember we set layout undefined at start so the buffer image pixels may be anywhere anytype and not predictable dst buufer is also just storing it doesnt care that much that data is in wrong order it just sores
// but now the data should be done to imafge layout na so its necesssary to change that transition to something our shaders will understand
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) // this will be called
{
    VkCommandBuffer commandBuffer = beginsingletimecommands();
    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = miplevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        printf("unsupported layout transition!\n");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier);

    endsingletimecommands(commandBuffer);
}
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) // this will be called
{
    VkCommandBuffer commandBuffer = beginsingletimecommands();
    VkBufferImageCopy region = {0};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    endsingletimecommands(commandBuffer);
}
bool creatingbuffer()
{
    vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties); // we are getting memory in physical device which the memory from which we can allocate memory for buffer like we are getting properties of physical device memory jut we will get size , number of heaps i g for staging here its no need
    // VkDeviceSize size = sizeof(float);                             // see think of it in buffer sense just the size of data we want to store when creating a buffer
    // VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; // it says that this buffer can be used as source to shaders like can give values like vertex just dint care about other bits
    // VkBufferUsageFlags uniform_texel_usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    // VkBufferUsageFlags storage_texel_usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    // VkBufferCreateInfo buffer_create_info = {
    //   VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    // nullptr,
    // 0,
    // size,
    // usage,
    // VK_SHARING_MODE_EXCLUSIVE, // its like whether queues from different family can access this buffer or not this bit says no and only one queue can access this buffer at a time
    // 0,
    //
    //  nullptr };
    /*VkBufferCreateInfo uniform_texel_buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        size,
        uniform_texel_usage,
        VK_SHARING_MODE_EXCLUSIVE, // its like whether queues from different family can access this buffer or not this bit says no and only one queue can access this buffer at a time
        0,
        nullptr };
    VkBufferCreateInfo storage_texel_buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        size,
        storage_texel_usage,
        VK_SHARING_MODE_EXCLUSIVE, // its like whether queues from different family can access this buffer or not this bit says no and only one queue can access this buffer at a time
        0,
        nullptr };

    resultb = vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer);
    if (VK_SUCCESS != resultb)
    {
        std::cout << "cant create buffer\n";
        return false;
    }
    resultb = vkCreateBuffer(logical_device, &uniform_texel_buffer_create_info, nullptr, &uniform_texel_buffer);
    if (VK_SUCCESS != resultb)
    {
        std::cout << "cant create  uniform texel buffer\n";
        return false;
    }
    resultb = vkCreateBuffer(logical_device, &storage_texel_buffer_create_info, nullptr, &storage_texel_buffer);
    if (VK_SUCCESS != resultb)
    {
        std::cout << "cant create  uniform texel buffer\n";
        return false;
    }*/
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // now let us do the same for staging buffer
    VkDeviceSize size_staging = sizeof(Vertex) * vertexcount;
    VkBufferUsageFlags usage_staging = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkBufferCreateInfo buffer_create_info_staging = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size_staging,
        usage_staging,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL

    };
    resultb = vkCreateBuffer(logical_device, &buffer_create_info_staging, NULL, &staging_buffer);
    if (VK_SUCCESS != resultb)
    {
        printf("cant create staging buffer\n");
        return false;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // now let us do the same for buffer which is going to accept the data from staging buffer
    VkDeviceSize size_dst_staging = sizeof(Vertex) * vertexcount;
    VkBufferUsageFlags usage_dst_staging = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkBufferCreateInfo buffer_create_info_dst_staging = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size_dst_staging,
        usage_dst_staging,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL};
    resultb = vkCreateBuffer(logical_device, &buffer_create_info_dst_staging, NULL, &staging_dst_buffer);
    if (VK_SUCCESS != resultb)
    {
        printf("cant create dst staging buffer\n");
        return false;
    }
    // now lets do the same buffer for index you foooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooool
    VkDeviceSize size_index_staging = sizeof(uint32_t) * indexcount;
    VkBufferUsageFlags usage_index_staging = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkBufferCreateInfo buffer_create_info_index_staging = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size_index_staging,
        usage_index_staging,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL};
    resultb = vkCreateBuffer(logical_device, &buffer_create_info_index_staging, NULL, &index_staging_buffer);
    if (VK_SUCCESS != resultb)
    {
        printf("cant create index staging buffer\n");
        return false;
    }
    // now let us do the same for buffer which is going to accept the data from staging buffer like from cpu to gpu
    VkDeviceSize size_index_dst_staging = sizeof(uint32_t) * indexcount;
    VkBufferUsageFlags usage_index_dst_staging = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBufferCreateInfo buffer_create_info_index_dst_staging = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size_index_dst_staging,
        usage_index_dst_staging,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL};
    resultb = vkCreateBuffer(logical_device, &buffer_create_info_index_dst_staging, NULL, &index_buffer);
    if (VK_SUCCESS != resultb)
    {
        printf("cant create dindex dst staging buffer\n");
        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // now i think i am supposed to create buffer for uniform also less goooooooooo
    /// first resize
    uniform_buffers = malloc(sizeof(VkBuffer) * images_count);
    memory_object_for_uniform = malloc(sizeof(VkDeviceMemory) * images_count);
    ubomapped = malloc(sizeof(void *) * images_count);
    //-----------------forloop----------------------------------//
    for (size_t i = 0; i < images_count; i++)
    {
        VkBufferUsageFlags ubo_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VkBufferCreateInfo buffer_create_info_for_ubo = {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            NULL,
            0,
            ubo_data_size,
            ubo_usage,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL

        };
        vkCreateBuffer(logical_device, &buffer_create_info_for_ubo, NULL, &uniform_buffers[i]);
        vkGetBufferMemoryRequirements(logical_device, uniform_buffers[i], &memory_requirements_for_uniform); // just getting things done in gpu side like size
        for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
        {
            if ((memory_requirements_for_uniform.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_uniform) == memory_property_for_uniform))
            {
                // so memorytypebits are 32 bits on or off so we need to check when type is 0 we left shift one so left side gets 1 we do and if its getting one at that part its true
                VkMemoryAllocateInfo buffer_memory_allocate_info_for_uniform = {
                    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    NULL,
                    memory_requirements_for_uniform.size,
                    type};
                resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_uniform, NULL, &memory_object_for_uniform[i]); // now memory object is populated with buffer which is host visible
                if (VK_SUCCESS == resultb)
                {
                    break;
                }
            }
        }
        vkBindBufferMemory(logical_device, uniform_buffers[i], memory_object_for_uniform[i], 0);

        vkMapMemory(logical_device, memory_object_for_uniform[i], 0, VK_WHOLE_SIZE, 0, &ubomapped[i]); // like idk wtf i am doing buti think i am creatig a ubo buffer every frame and mapping it means ubo mapped poointer will probably have this ubo buffers address for every frame i guess dont ask me
    }

    return true;
}
VkFormat findsupportedformat(VkFormat *formats, VkImageTiling tiling, VkFormatFeatureFlags features, uint32_t formatscount)
{
    for (uint32_t i = 0; i < formatscount; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, formats[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return formats[i];
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return formats[i];
        }
    }

    printf("Error: Could not find a supported format layout matching requested features!\n");
    return VK_FORMAT_UNDEFINED;
}

VkFormat findDepthFormat()
{
    uint32_t count = 3;
    VkFormat depth[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return findsupportedformat(depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, count);
}
bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
void generateMipmaps(VkImage image, VkFormat imageformat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physical_device, imageformat, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        printf("texture image format does not support linear blitting!");
    }

    int32_t mipwidth = texWidth;
    int32_t mipheight = texHeight;
    VkCommandBuffer commandBuffer = beginsingletimecommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1; // see here i started from 1 so the base originl mip levl will obviously become the previous
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, NULL,
                             0, NULL,
                             1, &barrier);
        VkImageBlit blit = {};
        blit.srcOffsets[0].x = 0; // according to sources think the frame starting from 0 to the other corner so it means from start
        blit.srcOffsets[0].y = 0;
        blit.srcOffsets[0].z = 0;
        blit.srcOffsets[1].x = mipwidth; // so this is kinda saying whole image
        blit.srcOffsets[1].y = mipheight;
        blit.srcOffsets[1].z = 1;
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1; // before mipmapping its like saying take the imager which is before
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0].x = 0;
        blit.dstOffsets[0].y = 0;
        blit.dstOffsets[0].z = 0;
        blit.dstOffsets[1].x = mipwidth > 0 ? mipwidth / 2 : 1; // region to which data will be blitted
        blit.dstOffsets[1].y = mipheight > 0 ? mipheight / 2 : 1;
        blit.dstOffsets[1].z = 1;
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i; // this is saying after mipmapping like its the image which is mipmapped
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // see before i did pipeline barrier which transitioned from dst to src
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // here in this function i am changing but point to benoted other then this function my whole  programm is still in src in this function i did dst so like in next step shader read will become easy
                       1, &blit,
                       VK_FILTER_LINEAR);
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;     // see as i said its still in src so i trandition now it to shader read but yeah dont worry the above function already solved this problem now it doesnt even matter for src but why to keave empty so use it
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // my shader will read now
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // shader read

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, // a barrier to ensure this whole mess is going to be red by fragment shader
                             0, NULL,
                             0, NULL,
                             1, &barrier);

        if (mipwidth > 1)
            mipwidth /= 2;
        if (mipheight > 1)
            mipheight /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, NULL,
                         0, NULL,
                         1, &barrier);

    endsingletimecommands(commandBuffer);
}
bool depthimage()
{
    printf("your depth format is %d\n", (int)findDepthFormat());
    VkFormat depthFormat = findDepthFormat();
    VkImageCreateInfo depthinfo = {0};
    depthinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthinfo.pNext = NULL;
    depthinfo.imageType = VK_IMAGE_TYPE_2D;
    depthinfo.extent.width = size_of_images.width;
    depthinfo.extent.height = size_of_images.height;
    depthinfo.extent.depth = 1;
    depthinfo.mipLevels = 1;
    depthinfo.arrayLayers = 1;
    depthinfo.format = depthFormat;
    depthinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthinfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // used by one queue family
    depthinfo.samples = VK_SAMPLE_COUNT_1_BIT;
    if (vkCreateImage(logical_device, &depthinfo, NULL, &depth_image) != VK_SUCCESS)
    {
        printf("failed to create image!\n");
    }
    vkGetImageMemoryRequirements(logical_device, depth_image, &memory_requirements_for_depth_image); // like the requirements image memory should have
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    {
        if ((memory_requirements_for_depth_image.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_depth_image) == memory_property_for_depth_image))
        {
            // so memorytypebits are 32 bits on or off so we need to check when type is 0 we left shift one so left side gets 1 we do and if its getting one at that part its true
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_depth = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_depth_image.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_depth, NULL, &memory_object_for_depth_image); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    vkBindImageMemory(logical_device, depth_image, memory_object_for_depth_image, 0);
    VkImageViewCreateInfo dviewInfo = {0};
    dviewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    dviewInfo.image = depth_image;
    dviewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // view 2d
    dviewInfo.format = depthFormat;
    dviewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // this is to see depth
    dviewInfo.subresourceRange.baseMipLevel = 0;
    dviewInfo.subresourceRange.levelCount = 1;
    dviewInfo.subresourceRange.baseArrayLayer = 0;
    dviewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(logical_device, &dviewInfo, NULL, &depth_image_view) != VK_SUCCESS)
    {
        printf("failed to create texture image view!\n");
    }
}

bool createtextureimage()
{
    printf("texWidth = %d\n", texWidth);
    printf("texHeight = %d\n", texHeight);
    printf("pixels = %p\n", (void *)pixels);
    printf("imageSize = %llu\n", (unsigned long long)(texWidth * texHeight * 4));
    imagesize = texWidth * texHeight * 4;
    uint32_t max = MAX(texWidth, texHeight);
    uint32_t log = log2(max); // this basically gets the power by which 2 should multiply it self like if log2(4) = 2 because 4 can be obtained by multipling 2 two times i can say that 4 can be divided twice like 2 one time and 2 other times
    // so from this i can check how much time the image height or width can be divided by 2 so like i can get miplevels
    miplevels = floor(log) + 1;
    // just saying there are rgba four componenets in one pixel so total * 4 our imafge size is declared above na so it will fill the vkdevice whenever we call this function

    VkBufferUsageFlags image_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkBufferCreateInfo image_buffer_create_info_for_image_staging = {0};
    image_buffer_create_info_for_image_staging.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    image_buffer_create_info_for_image_staging.pNext = NULL,
    image_buffer_create_info_for_image_staging.flags = 0,
    image_buffer_create_info_for_image_staging.size = imagesize,
    image_buffer_create_info_for_image_staging.usage = image_usage,
    image_buffer_create_info_for_image_staging.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    image_buffer_create_info_for_image_staging.queueFamilyIndexCount = 0,
    image_buffer_create_info_for_image_staging.pQueueFamilyIndices = NULL;
    vkCreateBuffer(logical_device, &image_buffer_create_info_for_image_staging, NULL, &image_staging_buffer);
    vkGetBufferMemoryRequirements(logical_device, image_staging_buffer, &memory_requirements_for_image);
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    {
        if ((memory_requirements_for_image.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_image_staging) == memory_property_for_image_staging))
        {
            // so memorytypebits are 32 bits on or off so we need to check when type is 0 we left shift one so left side gets 1 we do and if its getting one at that part its true
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_image = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_image.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_image, NULL, &memory_object_for_image_staging); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    vkBindBufferMemory(logical_device, image_staging_buffer, memory_object_for_image_staging, 0);

    vkMapMemory(logical_device, memory_object_for_image_staging, 0, imagesize, 0, &image_data); // map the memory which is the size of image_size in host visible memory and give the address of that memory to image_data pointer
    memcpy(image_data, pixels, imagesize);                                                      // copy the pixels data in pointer image_data specified address and just specify the size of data now with this staging image memory is populated
    vkUnmapMemory(logical_device, memory_object_for_image_staging);                             // no use na so unma                                                // free now like i got data in my specified place na
    // now the setup for the destibnation to hold the data
    VkImageCreateInfo imageInfo = {0};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = miplevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;          // like the datta is not shown to use humans so we dont know how the data is organized
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // initially i dont know the data how is it and i dont care how is it so use this
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // used by one queue family
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional
    if (vkCreateImage(logical_device, &imageInfo, NULL, &image_destination_buffer) != VK_SUCCESS)
    {
        printf("failed to create image!\n");
    }
    vkGetImageMemoryRequirements(logical_device, image_destination_buffer, &memory_requirements_for_image_destination);
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    {
        if ((memory_requirements_for_image_destination.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_image_destination) == memory_property_for_image_destination))
        {
            // so memorytypebits are 32 bits on or off so we need to check when type is 0 we left shift one so left side gets 1 we do and if its getting one at that part its true
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_image = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_image_destination.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_image, NULL, &memory_object_for_image_destination); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }

    vkBindImageMemory(logical_device, image_destination_buffer, memory_object_for_image_destination, 0); // like saying this memory_object belongs to image_destination_buffer its mine kinda romance after this binding there is no data in destinationn buffer but still we have now space
    // so see image memory bind to memory object for image destination but still its incomplete because we never did vkcopybuffer
    transitionImageLayout(image_destination_buffer, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // image layout transfer dst optimal because for copy bufefr to image it is compulsary
    copyBufferToImage(image_staging_buffer, image_destination_buffer, texWidth, texHeight);
    generateMipmaps(image_destination_buffer, VK_FORMAT_R8G8B8A8_SRGB ,texWidth, texHeight, miplevels);                                                                                /// dont look confused these funcions are defined and they do the stuf for you like i know it good to rewrite because it sharoens concept but its using one time staging buffer we can now use that function as many times as possible reducing boiler plate
    // above transition chnages the layout to shader read optimal so the mip map also needs the correct layout
    vkDestroyBuffer(logical_device, image_staging_buffer, NULL);
    vkFreeMemory(logical_device, memory_object_for_image_staging, NULL);
    return true;
}

bool createdestinationimageview()
{
    // image view are wrappers on vkimage which tells the gpu how to access and interpret the data
    // sampler are used to say how gpu reads data during rendering
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image_destination_buffer;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // view 2d
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // this is to see colors
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = miplevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(logical_device, &viewInfo, NULL, &image_destination_view) != VK_SUCCESS)
    {
        printf("failed to create texture image view!\n");
    }
}
bool createdestinationimagesampler()
{
    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.mipLodBias = 0.0f; // Optional
    if (vkCreateSampler(logical_device, &samplerInfo, NULL, &image_destination_sampler) != VK_SUCCESS)
    {
        printf("failed to create texture sampler!\n");
    }
}
bool allocatingandbindingmemory()
{
    // vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);                                 // we are getting requirements to assign memory to allocate memeory
    vkGetBufferMemoryRequirements(logical_device, staging_buffer, &memory_requirements_for_staging);         // now that last parameter will store the memory requiremnts
    vkGetBufferMemoryRequirements(logical_device, staging_dst_buffer, &memory_requirements_for_dst_staging); // now that last parameter will store the memory requiremnts
    vkGetBufferMemoryRequirements(logical_device, index_staging_buffer, &memory_requirements_for_index_staging);
    vkGetBufferMemoryRequirements(logical_device, index_buffer, &memory_requirements_for_index_dst);

    // vkGetBufferMemoryRequirements(logical_device, uniform_texel_buffer, &memory_requirements_for_uniform_texel); // now that last parameter will store the memory requiremnts
    //  here memory types we are getting properties of physical device
    //  now iterate over available physical Device memory types which are in memorytype count
    /*for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties))
        {
            VkMemoryAllocateInfo buffer_memory_allocate_info = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements.size,
                type };
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info, NULL, &memory_object); // now memory object is populated with buffer which is device local
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }*/
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements_for_staging.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_staging) == memory_property_for_staging))
        {
            // so memorytypebits are 32 bits on or off so we need to check when type is 0 we left shift one so left side gets 1 we do and if its getting one at that part its true
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_staging = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_staging.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_staging, NULL, &memory_object_for_staging); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements_for_dst_staging.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_dst_staging) == memory_property_for_dst_staging))
        {
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_dst_staging = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_dst_staging.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_dst_staging, NULL, &memory_object_for_dst_staging); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements_for_index_staging.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_index_staging) == memory_property_for_index_staging))
        {
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_index_staging = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_dst_staging.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_index_staging, NULL, &memory_object_for_index_staging); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements_for_index_dst.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_index_dst) == memory_property_for_index_dst))
        {
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_index_dst = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_index_dst.size,
                type};
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_index_dst, NULL, &memory_object_for_index_dst); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    /*for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements_for_uniform_texel.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_uniform_texel) == memory_property_for_uniform_texel))
        {
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_uniform_texel = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_uniform_texel.size,
                type };
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_uniform_texel, NULL, &memory_object_for_uniform_texel); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }
    */
    /*for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; type++)
    { // here we took type 0 so every memory type count we will iterate we will loop untill all memory type in physical device count which is basically a array so we will iterate untill all memory type counts get included we will get type - 1 total count of types at last
        if ((memory_requirements_for_storage_texel.memoryTypeBits & (1 << type)) && ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_property_for_storage_texel) == memory_property_for_storage_texel))
        {
            VkMemoryAllocateInfo buffer_memory_allocate_info_for_storage_texel = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memory_requirements_for_storage_texel.size,
                type };
            resultb = vkAllocateMemory(logical_device, &buffer_memory_allocate_info_for_storage_texel, NULL, &memory_object_for_storage_texel); // now memory object is populated with buffer which is host visible
            if (VK_SUCCESS == resultb)
            {
                break;
            }
        }
    }*/
    vkBindBufferMemory(logical_device, staging_buffer, memory_object_for_staging, 0);
    vkBindBufferMemory(logical_device, index_staging_buffer, memory_object_for_index_staging, 0);
    vkBindBufferMemory(logical_device, staging_dst_buffer, memory_object_for_dst_staging, 0);
    vkBindBufferMemory(logical_device, index_buffer, memory_object_for_index_dst, 0);
    return true;
}

// in my case see i have a vertex buffer its in staging then going to dst it smooth soo wiht this memory barrier there are many uses of buffer to be readby cmdbuffer like first i want to first it should br vertx
//  accepting then shader swew

// THIS IS MEMORY BARRIER FOR DESTINATION BUFFER/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bool settingmemorybarrier()//buffers can be used as source of imformation which we can bind to pipeline and use shaders or we can store information from shaders
//{//there are many types of uses of a buffer we have to say that to driver so for this purpose there are memory buffer
// using namespace VulkanCookbook;
// return true;
//}
bool stagingresources() // i will just call it vertex buffer
{                       // here like i created memory in cpu but i cant just right directly on gpu so we staging buffers to mediate tranfer of data which are writte in cpu side to host visible side ig in integrted gpu its not required but ihave rtx 2050 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // its here acting as intermediate which will take dat afrom cpu to gpu
    // its the actual data which i want in staging buffer like it contains data to which i will give it host visin=ble buffer
    void *local_pointer;                                                                                        // this thing will have the memory address of  that data of stagingbuffer not at all related to gpu memopry
    resultb = vkMapMemory(logical_device, memory_object_for_staging, offset, VK_WHOLE_SIZE, 0, &local_pointer); // here i am getting the local_pointer a adrees of the dta which is cpu it now have the pointer of host visble memory
    if (VK_SUCCESS != resultb)
    { // like now i want to share data so memoryobject contains all i want so i wiill use it now with map memory its
        printf("error in mapping memory\n");
        return false;
    }
    memcpy(local_pointer, vertices, data_size); // its saying like copy data like vertex then copy them in local_pointers pointing address
    memory_ranges = malloc(sizeof(VkMappedMemoryRange));
    memory_ranges[0] = (VkMappedMemoryRange){
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        NULL,
        memory_object_for_staging,
        offset,
        VK_WHOLE_SIZE};
    resultb = vkFlushMappedMemoryRanges(logical_device, 1, &memory_ranges[0]);
    if (VK_SUCCESS != resultb)
    // like its saying i am done storing data in cpu i am ready to be fucked by gpu
    {
        printf("error in flushing mapping memory\n");
        return false;
    }
    vkUnmapMemory(logical_device, memory_object_for_staging);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // now let us do same for index dst copying then sending it to dst (gpu) i will do it tomorow here youu fool -------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ // its the actual data which i want in staging buffer like it contains data to which i will give it host visin=ble buffer
    local_pointer;
    resultb = vkMapMemory(logical_device, memory_object_for_index_staging, offset, VK_WHOLE_SIZE, 0, &local_pointer);
    if (VK_SUCCESS != resultb)
    {
        printf("error in mapping index memory\n");
        return false;
    }
    memcpy(local_pointer, indices, index_data_size);
    memory_ranges_for_index = malloc(sizeof(VkMappedMemoryRange));
    memory_ranges_for_index[0] = (VkMappedMemoryRange){
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        NULL,
        memory_object_for_index_staging,
        offset,
        VK_WHOLE_SIZE};
    resultb = vkFlushMappedMemoryRanges(logical_device, 1, &memory_ranges_for_index[0]);
    if (resultb != VK_SUCCESS)
    {
        printf("error in flushing mapping memory\n");
        return false;
    }
    vkUnmapMemory(logical_device, memory_object_for_index_staging);
    ///
    // ubo

    return true;
}
bool copyingvertexbuffer()
{
    // FROM HERE COPY INSTRUCTION CAN BE CALLED IN CMDBUFFER.CPP
    // here should be thw membarrier
    buffer_transitions = malloc(sizeof(BufferTransition) * (1) + 1);
    buffer_transitions[0] = (BufferTransition){staging_dst_buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED}; // it is important to use () cause like in cpp we can ignore type but in c type cant be ignored
    // wait untill transfering is done then do change access to vertex read
    buffer_memory_barriers = malloc(sizeof(VkBufferMemoryBarrier) * 1);

    for (uint32_t i = 0; i < 1; i++)
    {
        buffer_memory_barriers[i] = (VkBufferMemoryBarrier){
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            NULL,
            buffer_transitions[i].CurrentAccess,
            buffer_transitions[i].NewAccess,
            buffer_transitions[i].CurrentQueueFamily,
            buffer_transitions[i].NewQueueFamily,
            buffer_transitions[i].Buffer,
            0,
            VK_WHOLE_SIZE};
    }
    // NOW QUEUE SUBMIT IS IN CMDBUFFER AND IT WILL WAIT THERE ALSO ITS IN CMD BUFFER GO CHECK IT
    return true;
}
bool updatingubo() // this will get rewrite everyframe like its in loop so this will be moving big na so sunstitute it with stationary means will give the the time elapsed
{
    struct timespec currenttime; // this timeer is original which calaculates the running time
    clock_gettime(CLOCK_MONOTONIC, &currenttime);
    long secdiff = currenttime.tv_sec - starttime.tv_sec;
    long nandiff = currenttime.tv_nsec - starttime.tv_nsec;
    float elapsedSeconds = (float)secdiff + (float)nandiff / 1e9f;

    // now time to move struct values
    ubox ubo = {};
    mat4 trans = GLM_MAT4_IDENTITY_INIT;
    glm_rotate(trans, elapsedSeconds * glm_rad(90.0f), (vec3){0.01f, 0.01f, 0.01f});
    glm_scale(trans, (vec3){0.1f, 0.1f, 0.1f});
    glm_mat4_copy(trans, ubo.model);
    vec3 eye = {2.0f, 2.0f, 10.0f};
    vec3 cam = {0.0f, 0.0f, -1.0f};
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_lookat(eye, cam, up, trans);
    glm_mat4_copy(trans, ubo.view);
    float fov = glm_rad(60.0f);                                         // Field of view in radians
    float aspect = (float)size_of_images.width / size_of_images.height; // Aspect ratio (width / height)
    float nearZ = 0.1f;                                                 // Near clipping plane
    float farZ = 100.0f;                                                // Far clipping plane

    glm_perspective(fov, aspect, nearZ, farZ, trans);
    glm_mat4_copy(trans, ubo.proj);
    ubo.proj[1][1] *= -1;
    memcpy(ubomapped[image_index], &ubo, sizeof(ubo));
    return true;
}
// bool creatingbufferview()
//{//
//   using namespace VulkanCookbook;
//   VkBufferViewCreateInfo buffer_view_create_info = {
//     VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
//   nullptr,
// 0,
// buffer,
// format,
// memory_offset,
// memory_range };
// VkBufferView buffer_view; // now to store the view
// resultb = vkCreateBufferView(logical_device, &buffer_view_create_info, nullptr, &buffer_view);
// if (resultb != VK_SUCCESS)
//{
//   std::cout << "cant create buffer view\n";
//   return false;
//}
// else
//{
//    return true;
//}
/* VkBufferViewCreateInfo uniform_texel_buffer_view_create_info = {
     VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
     nullptr,
     0,
     uniform_texel_buffer,
     format,
     memory_offset,
     memory_range };
 VkBufferView uniform_texel_buffer_view; // now to store the view
 resultb = vkCreateBufferView(logical_device, &uniform_texel_buffer_view_create_info, nullptr, &uniform_texel_buffer_view);
 if (resultb != VK_SUCCESS)
 {
     std::cout << "cant create buffer view\n";f//////
     return false;
 }
 else
 {
     return true;
 }
 VkBufferViewCreateInfo storage_texel_buffer_view_create_info = {
     VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
     nullptr,
     0,
     storage_texel_buffer,
     format,
     memory_offset,
     memory_range };
 VkBufferView storage_texel_buffer_view; // now to store the view
 resultb = vkCreateBufferView(logical_device, &storage_texel_buffer_view_create_info, nullptr, &storage_texel_buffer_view);
 if (resultb != VK_SUCCESS)
 {
     std::cout << "cant create buffer view\n";
     return false;
 }
 else
 {
     return true;
 }
 return true;
}*/
bool bufferbinding() // like to specify from where to take vertex data
{
    binding_descriptions = malloc(sizeof(VkVertexInputBindingDescription) * 1);
    binding_descriptions->binding = 0;
    binding_descriptions->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_descriptions->stride = sizeof(Vertex);
    attribute_descriptions = (VkVertexInputAttributeDescription *)malloc(sizeof(VkVertexInputAttributeDescription) * 3);
    memset(attribute_descriptions, 0, sizeof(VkVertexInputAttributeDescription) * 2);
    /// this is for vertex data
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(Vertex, position);
    // now lets do for color woahhhh
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex, color);
    // lets do for texture
    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset = offsetof(Vertex, texcord);

    /// i will bind vertex in pipeline.cpp cause my all bindings are from there
    return true;
}
void buffer_cleanup()
{
    vkFreeCommandBuffers(logical_device, cmdpoolforstaging, 1, &cmdbufferforstaging);
    vkDestroyCommandPool(logical_device, cmdpoolforstaging, NULL);
    vkDestroyBuffer(logical_device, staging_buffer, NULL);
    vkDestroyBuffer(logical_device, staging_dst_buffer, NULL);
    vkFreeMemory(logical_device, memory_object_for_staging, NULL);
    vkFreeMemory(logical_device, memory_object_for_dst_staging, NULL);
    vkDestroyImage(logical_device, image_destination_buffer, NULL);
    vkFreeMemory(logical_device, memory_object_for_image_destination, NULL);
    vkDestroyImageView(logical_device, image_destination_view, NULL);
    vkDestroySampler(logical_device, image_destination_sampler, NULL);

    free(cmdbuffers);
    free(memory_ranges);
    free(buffer_transitions);
    free(buffer_memory_barriers);
    free(binding_descriptions);
    free(attribute_descriptions);
}