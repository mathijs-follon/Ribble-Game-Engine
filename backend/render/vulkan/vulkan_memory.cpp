#include "vulkan_memory.h"

namespace backend {

    uint32_t find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                              VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
            if ((typeFilter & (1u << i)) &&
                (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        return VK_MAX_MEMORY_TYPES;
    }

    bool create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps,
                       VkBuffer &outBuffer, VkDeviceMemory &outMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &outBuffer) != VK_SUCCESS)
            return false;

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, outBuffer, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = find_memory_type(physicalDevice, memReqs.memoryTypeBits, memProps);

        if (allocInfo.memoryTypeIndex >= VK_MAX_MEMORY_TYPES) {
            vkDestroyBuffer(device, outBuffer, nullptr);
            return false;
        }

        if (vkAllocateMemory(device, &allocInfo, nullptr, &outMemory) != VK_SUCCESS) {
            vkDestroyBuffer(device, outBuffer, nullptr);
            return false;
        }

        vkBindBufferMemory(device, outBuffer, outMemory, 0);
        return true;
    }

    void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory) {
        if (buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(device, buffer, nullptr);
        if (memory != VK_NULL_HANDLE)
            vkFreeMemory(device, memory, nullptr);
    }

    bool create_image(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
                      VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                      VkMemoryPropertyFlags memProps, VkImage &outImage, VkDeviceMemory &outMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &outImage) != VK_SUCCESS)
            return false;

        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, outImage, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = find_memory_type(physicalDevice, memReqs.memoryTypeBits, memProps);

        if (allocInfo.memoryTypeIndex >= VK_MAX_MEMORY_TYPES) {
            vkDestroyImage(device, outImage, nullptr);
            return false;
        }

        if (vkAllocateMemory(device, &allocInfo, nullptr, &outMemory) != VK_SUCCESS) {
            vkDestroyImage(device, outImage, nullptr);
            return false;
        }

        vkBindImageMemory(device, outImage, outMemory, 0);
        return true;
    }

    void destroy_image(VkDevice device, VkImage image, VkDeviceMemory memory) {
        if (image != VK_NULL_HANDLE)
            vkDestroyImage(device, image, nullptr);
        if (memory != VK_NULL_HANDLE)
            vkFreeMemory(device, memory, nullptr);
    }

} // namespace backend
