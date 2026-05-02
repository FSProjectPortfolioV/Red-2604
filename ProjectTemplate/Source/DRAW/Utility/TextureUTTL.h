#ifndef TEXTUREUTILS_H
#define TEXTUREUTILS_H

#define NOMINMAX 

#include "../../gateware-main/Gateware.h"
#include <vulkan/vulkan.h>
#include <algorithm>
#include <cmath> 

#include "stb_image.h"

// FOR UI
// A universal struct to hold any 2D UI Image
struct UIIcon {
    unsigned int* pixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
};
// This dictionary will store all loaded UI icons by a string name!
inline std::unordered_map<std::string, UIIcon> activeUIIcons;

// Loads an image file into a UIIcon struct and saves it to the activeUIIcons dictionary with the given name
inline void LoadUIIcon(const std::string& name, const std::string& filepath) {
    UIIcon newIcon;
    newIcon.pixels = (unsigned int*)stbi_load(filepath.c_str(), &newIcon.width, &newIcon.height, &newIcon.channels, 4);

    if (newIcon.pixels == nullptr) {
        std::cout << "WARNING: UI Icon Failed to load: " << filepath << std::endl;
    }
    else {
        // Save it to the dictionary
        activeUIIcons[name] = newIcon;
    }
}


// A lightweight struct to hold raw pixel data
struct RawImage {
    int width, height, component, bits;
    std::vector<unsigned char> image;
};

// 1. This does the actual Vulkan memory allocation and buffer copying
inline void UploadTextureToGPU(GW::GRAPHICS::GVulkanSurface _surface, const RawImage& _img,
    VkDeviceMemory& _outTextureMemory, VkImage& _outTextureImage,
    VkImageView& _outTextureImageView, bool _linearColorSpace = true)
{
    // grab all the needed handles
    VkQueue vkQGX;
    VkDevice vkDev;
    VkPhysicalDevice vkPDev;
    VkCommandPool vkCmdPool;
    _surface.GetDevice(reinterpret_cast<void**>(&vkDev));
    _surface.GetGraphicsQueue(reinterpret_cast<void**>(&vkQGX));
    _surface.GetPhysicalDevice(reinterpret_cast<void**>(&vkPDev));
    _surface.GetCommandPool(reinterpret_cast<void**>(&vkCmdPool));

    // Set up Texture staging buffer
    VkDeviceSize imageSize = _img.width * _img.height * _img.component;
    VkBuffer staging_bufferIM;
    VkDeviceMemory staging_buffer_memoryIM;

    // determine format (8bit default)
    VkFormat format = (_linearColorSpace)
        ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB;
    if (_img.bits == 16)
        format = VK_FORMAT_R16G16B16A16_SFLOAT;
    else if (_img.bits == 32)
        format = VK_FORMAT_R32G32B32A32_SFLOAT;


    // Create the staging buffer
    GvkHelper::create_buffer(vkPDev, vkDev, imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_bufferIM, &staging_buffer_memoryIM);

    // Copy the pixel data over to the staging buffer
    GvkHelper::write_to_buffer(vkDev, staging_buffer_memoryIM,
        _img.image.data(), static_cast<unsigned int>(imageSize));

    // Setup extent and calculate how many mipmap levels we need
    VkExtent3D tempExtent = { static_cast<uint32_t>(_img.width), static_cast<uint32_t>(_img.height), 1 };
    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(_img.width, _img.height))) + 1);

    // Create the final Vulkan Image
    GvkHelper::create_image(vkPDev, vkDev, tempExtent, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &_outTextureImage, &_outTextureMemory);

    // Transition the image layout and copy the buffer data to the image
    GvkHelper::transition_image_layout(vkDev, vkCmdPool, vkQGX, mipLevels, _outTextureImage, format,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    GvkHelper::copy_buffer_to_image(vkDev, vkCmdPool, vkQGX, staging_bufferIM, _outTextureImage, tempExtent);

    // Create mipmaps
    GvkHelper::create_mipmaps(vkDev, vkCmdPool, vkQGX, _outTextureImage, _img.width, _img.height, mipLevels);

    // Create the image view so the shader can sample it
    GvkHelper::create_image_view(vkDev, _outTextureImage, format,
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, nullptr, &_outTextureImageView);

    // Clean up temporary staging buffer
    vkDestroyBuffer(vkDev, staging_bufferIM, nullptr);
    vkFreeMemory(vkDev, staging_buffer_memoryIM, nullptr);
}

// 2. THE FILE LOADER: Opens the image file and passes it to the function above
inline void UploadTextureToGPU(GW::GRAPHICS::GVulkanSurface _surface, const std::string& _file,
    VkDeviceMemory& _outTextureMemory, VkImage& _outTextureImage,
    VkImageView& _outTextureImageView, bool _linearColorSpace = true)
{
    RawImage img = {};

    // Open a file using stb_image
    int width, height, component;
    auto data = stbi_load(_file.c_str(), &width, &height, &component, STBI_rgb_alpha); // force 4 channels

    if (!data) {
        // Just a safety check in case the file path is wrong!
        return;
    }

    img.width = width;
    img.height = height;
    img.component = 4; // always 4 channels even if the image is 3
    img.bits = 8; // always 8 bits per channel
    img.image.resize(img.width * img.height * img.component);

    // Copy the raw pixel data into our struct
    std::memcpy(img.image.data(), data, img.image.size());

    // Pass it to the core function above!
    UploadTextureToGPU(_surface, img, _outTextureMemory, _outTextureImage,
        _outTextureImageView, _linearColorSpace);

    // Free the raw loaded data
    stbi_image_free(data);
}

// 3. THE SAMPLER MAKER: Creates the rules for how the shader zooms/stretches the image
inline VkResult CreateSampler(GW::GRAPHICS::GVulkanSurface _surface, VkSampler& _outSampler,
    VkSamplerAddressMode _addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    VkFilter _filter = VK_FILTER_LINEAR, float _anisotropy = 4.0f)
{
    // grab all the needed handles
    VkDevice vkDev;
    _surface.GetDevice(reinterpret_cast<void**>(&vkDev));

    // create the image view and sampler
    VkSamplerCreateInfo samplerInfo = {};
    // Set the struct values
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.flags = 0;
    samplerInfo.addressModeU = _addressMode; // REPEAT IS COMMON
    samplerInfo.addressModeV = _addressMode;
    samplerInfo.addressModeW = _addressMode;
    samplerInfo.magFilter = _filter;
    samplerInfo.minFilter = _filter;
    samplerInfo.mipmapMode = (_filter == VK_FILTER_NEAREST)
        ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0;
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.anisotropyEnable = (_anisotropy >= 1.0f) ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = _anisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.pNext = nullptr;

    return vkCreateSampler(vkDev, &samplerInfo, nullptr, &_outSampler);
}

#endif // !TEXTUREUTILS_H