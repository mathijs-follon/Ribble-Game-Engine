#pragma once

#include <vulkan/vulkan.h>

namespace backend {

    uint32_t find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                              VkMemoryPropertyFlags properties);

    bool create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps,
                       VkBuffer &outBuffer, VkDeviceMemory &outMemory);

    void destroy_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory);

    bool create_image(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
                      VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                      VkMemoryPropertyFlags memProps, VkImage &outImage, VkDeviceMemory &outMemory);

    void destroy_image(VkDevice device, VkImage image, VkDeviceMemory memory);

} // namespace backend
