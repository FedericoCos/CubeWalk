#pragma once

// File containing libraries used by most of the classes
#include <vector>
#include <stdio.h>
#include <stdexcept>
#include <array>
#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <optional>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream> 
#include <map>
#include <chrono>
#include <deque>
#include <thread>

#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vk_mem_alloc.h"




// Structure to hold queue family indices, the queues themselves and the relative command pool
struct QueuePool {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> transfer_family;

    vk::raii::Queue graphics_queue = nullptr;
    vk::raii::Queue transfer_queue = nullptr;
    vk::raii::Queue present_queue = nullptr;

    vk::raii::CommandPool graphics_command_pool = nullptr;
    vk::raii::CommandPool transfer_command_pool = nullptr;

    vk::raii::CommandBuffers graphics_command_buffers = nullptr;
    int max_frames_in_flight = 2;

    bool isIndexComplete() const{
        return graphics_family.has_value() && 
            present_family.has_value() &&
            transfer_family.has_value();
    }

    bool isQueueComplete(){
        return graphics_queue != nullptr &&
            present_queue != nullptr &&
            transfer_queue != nullptr;
    }

    bool isPoolComplete(){
        return graphics_command_pool != nullptr &&
            transfer_command_pool != nullptr;
    }


};

// Structure that holds swpachain and relative resources
struct SwapchainBundle{
    vk::raii::SwapchainKHR swapchain = nullptr;
    std::vector<vk::Image> images; // can't use raii here since using swapchain.getImages() to extract the images of swapchain
    std::vector<vk::raii::ImageView> image_views;
    vk::Format format;
    vk::Extent2D extent;
    vk::PresentModeKHR present_mode;
    vk::SharingMode sharing_mode;

    // Ovveride printing method
    std::ostream& operator<<(std::ostream& os) {
        return (os << "Format: " << vk::to_string(format)
                << "\nExtent: " << extent.width << ", " << extent.height
                << "\nImages size: " << images.size()
                << "\nPresent mode: " << vk::to_string(present_mode)
                << "\nSharing mode: " << vk::to_string(sharing_mode) << "\n");
    }

    std::string to_str(){
        std::stringstream ss;
        *this << ss;
        return ss.str();
    }
};

// Structure that holds all the info about allocated images
struct AllocatedImage{
    vk::Image image = nullptr;
    VmaAllocation allocation = nullptr;
    VmaAllocator allocator = nullptr;
    vk::Extent3D image_extent;
    vk::Format image_format;
    vk::ImageType image_type;
    uint32_t mip_levels = 1;
    vk::raii::ImageView image_view = nullptr;
    uint32_t array_layers = 1;
    std::string image_name = "default_name";
    vk::ImageCreateFlags flags;

    AllocatedImage(){};

    ~AllocatedImage(){
        if(image && allocator){
            std::cout << "Destroying image: " << image_name << std::endl;
            if(image_view != nullptr){
                image_view = nullptr;
            }
            vmaDestroyImage(allocator, image, allocation);
            image = nullptr;
            allocator = nullptr;
        }
    }

    // Disable copying to prevent double-free
    AllocatedImage(const AllocatedImage&) = delete;
    AllocatedImage& operator=(const AllocatedImage&) = delete;

    // Enable moving
    AllocatedImage(AllocatedImage&& other) noexcept 
        : image(other.image), allocation(other.allocation), allocator(other.allocator), image_extent(other.image_extent),
            image_format(other.image_format), image_type(other.image_type), mip_levels(other.mip_levels),
            image_view(std::move(other.image_view)), array_layers(other.array_layers), flags(other.flags), 
            image_name(std::move(other.image_name)) {
        other.image = nullptr; 
        other.allocation = nullptr;
        other.allocator = nullptr;
    }

    AllocatedImage& operator=(AllocatedImage&& other) noexcept {
        if (this != &other) {
            // Destroy current if it exists
            image_view = nullptr;
            if (image && allocator) vmaDestroyImage(allocator, image, allocation);
            
            image = other.image;
            allocation = other.allocation;
            allocator = other.allocator;
            image_extent = other.image_extent;
            image_format = other.image_format;
            image_type = other.image_type;
            mip_levels = other.mip_levels;
            array_layers = other.array_layers;
            image_view = std::move(other.image_view);
            image_name = std::move(other.image_name);
            flags = other.flags;
            
            other.image = nullptr;
            other.allocation = nullptr;
            other.allocator = nullptr;
        }
        return *this;
    }

    // Ovveride printing method
    std::ostream& operator<<(std::ostream& os) {
        return (os << "Name: " << image_name.c_str()
                << "\nExtent: " << image_extent.width << ", " << image_extent.height
                << "\nFormat: " << vk::to_string(image_format)
                << "\nType: " << vk::to_string(image_type)
                << "\nMip levels: " << mip_levels
                << "\nArray Layers: " << array_layers << "\n");
    }

    std::string to_str(){
        std::stringstream ss;
        *this << ss;
        return ss.str();
    }
};


// Stuct that holds all the information about a raster pipeline
struct RasterPipelineBundle{
    std::string name = "default pipeline name";
    vk::raii::Pipeline pipeline = nullptr;
    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
    vk::raii::PipelineLayout layout = nullptr;
    vk::raii::DescriptorPool descriptor_pool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptor_sets;

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = {};
    std::vector<vk::raii::ShaderModule> shaders = {};

    vk::PipelineInputAssemblyStateCreateInfo input_assembly = {};
    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    vk::PipelineMultisampleStateCreateInfo multisampling = {};

    std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachments = {};
    vk::PipelineColorBlendStateCreateInfo color_blending = {};
    std::vector<vk::Format> color_formats;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil = {};

    vk::PipelineRenderingCreateInfo pipeline_rendering_create_info = {};

    RasterPipelineBundle() = default;

    // Disable copying to prevent double-free
    RasterPipelineBundle(const RasterPipelineBundle&) = delete;
    RasterPipelineBundle& operator=(const RasterPipelineBundle&) = delete;

    // Enable moving
    RasterPipelineBundle(RasterPipelineBundle&& other) noexcept
        : name(other.name), pipeline(std::move(other.pipeline)), 
        descriptor_set_layout(std::move(other.descriptor_set_layout)),
        layout(std::move(other.layout)), 
        descriptor_pool(std::move(other.descriptor_pool)),
        descriptor_sets(std::move(other.descriptor_sets)),
        shader_stages(other.shader_stages),
        input_assembly(other.input_assembly), rasterizer(other.rasterizer),
        multisampling(other.multisampling), color_blend_attachments(std::move(other.color_blend_attachments)),
        color_blending(other.color_blending), 
        color_formats(std::move(other.color_formats)),
        depth_stencil(other.depth_stencil),
        pipeline_rendering_create_info(other.pipeline_rendering_create_info)
        {}

    RasterPipelineBundle& operator=(RasterPipelineBundle&& other) noexcept{
        if(this != &other){
            name = std::move(other.name);
            pipeline = std::move(other.pipeline);
            descriptor_set_layout = std::move(other.descriptor_set_layout);
            layout = std::move(other.layout);
            descriptor_sets = std::move(other.descriptor_sets);
            descriptor_pool = std::move(other.descriptor_pool);

            shader_stages = std::move(other.shader_stages);
            input_assembly = input_assembly;
            rasterizer = other.rasterizer;
            multisampling = other.multisampling;
            color_blend_attachments = std::move(other.color_blend_attachments);
            color_blending = other.color_blending;
            color_formats = std::move(other.color_formats);
            depth_stencil = other.depth_stencil;
            pipeline_rendering_create_info = other.pipeline_rendering_create_info;
        }
        return *this;
    }

    std::ostream& operator<<(std::ostream& os) {
        return (os << "Name:" << name.c_str()
                << "\n"
                );
    }

    std::string to_str(){
        std::stringstream ss;
        *this << ss;
        return ss.str();
    }
};

// Structure that holds all info about Vertices
struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;


    // Defining how to consider two Vertex equal or not
    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && color == other.color;
    }

    // Info needed to tell VUlkan how to pass Vertex data to the shader
    static vk::VertexInputBindingDescription getBindingDescription(){
        // First is index, second is stride, last means either read data
        // one vertex at the time, or one instance at the time
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        return{
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
            vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
        };
    }
};

// Structure that holds buffer data
struct AllocatedBuffer{
    vk::Buffer buffer = nullptr;
    VmaAllocation allocation = nullptr;
    VmaAllocator allocator = nullptr;
    vk::DeviceSize size;
    VmaAllocationInfo info = {};

    std::string name = "default name";

    AllocatedBuffer() = default;
    ~AllocatedBuffer() {
        if(buffer && allocation && allocator){
            std::cout << "Destroying buffer: " << name << std::endl;
            vmaDestroyBuffer(allocator, buffer, allocation);
            buffer = nullptr;
            allocation = nullptr;
            allocator = nullptr;
        }
    }

    // Disablying copying
    AllocatedBuffer(const AllocatedBuffer&) = delete;
    AllocatedBuffer& operator=(const AllocatedBuffer&) = delete;

    //Enabling moving
    AllocatedBuffer(AllocatedBuffer&& other) noexcept :
        buffer(other.buffer), allocation(other.allocation), allocator(other.allocator), info(other.info), size(other.size), name(std::move(other.name)){
            other.buffer = nullptr;
            other.allocation = nullptr;
            other.allocator = nullptr;
            other.name = "default name";
        }
    
    AllocatedBuffer& operator=(AllocatedBuffer&& other) noexcept {
        if(this != &other){
            if(buffer && allocation && allocator){
                vmaDestroyBuffer(allocator, buffer, allocation);
            }

            buffer = other.buffer;
            allocation = other.allocation;
            allocator = other.allocator;
            name = std::move(other.name);
            info = other.info;
            size = other.size;

            other.buffer = nullptr;
            other.allocation = nullptr;
            other.allocator = nullptr;
            other.name = "default name";
        }

        return *this;
    }

};


// Enum fro camera movement directions
enum class CameraMovement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Uniform Buffer object for View and Prokection matrix from camera
struct UniformBufferCamera{
    glm::mat4 view;
    glm::mat4 proj;
};

struct UniformBufferGameObjects{
    glm::mat4 model;
};

// Uniform Buffer object for mapped data
struct MappedUBO{
    AllocatedBuffer buffer;
    void *data = nullptr;

    MappedUBO() = default;
    ~MappedUBO(){
        if(buffer.allocation && buffer.allocator)
            vmaUnmapMemory(buffer.allocator, buffer.allocation);
    }

    // Disable copying
    MappedUBO(const MappedUBO&) = delete;
    MappedUBO& operator=(const MappedUBO&) = delete;

    // Enable moving
    MappedUBO(MappedUBO&& other) noexcept 
        : buffer(std::move(other.buffer)), data(other.data) {
        other.data = nullptr;
    }

    MappedUBO& operator=(MappedUBO&& other) noexcept {
        if (this != &other) {
            buffer = std::move(other.buffer);
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }
};


enum class InputState{
    PRESSED,
    RELEASED,
    HOLD
};

