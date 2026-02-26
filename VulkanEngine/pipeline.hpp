#pragma once

#include "../Helpers/GeneralLibraries.hpp"

/**
 * This is a builder class.
 * The idea is to create a pipelinebundle obj and then move it when completed
 */
class PipelineBuilder{
public:
    RasterPipelineBundle pipeline_bundle;

    // All the builder functions
    void set_name(std::string name);
    void add_shader(std::string path, vk::ShaderStageFlagBits stage, vk::raii::Device &logical_device);
    void set_topology(vk::PrimitiveTopology topology);
    void set_rasterizer(vk::CullModeFlags cull_mode, vk::FrontFace front_face, vk::PolygonMode mode);
    void set_multisampling_none();
    void add_non_blend_color_attachment(vk::ColorComponentFlags write_mask);
    void set_color_and_depth_format(std::vector<vk::Format> color_formats,vk::Format depth_format);
    void set_depth_stencil(bool depth_test_enable, bool depth_write_enable, vk::CompareOp op);

    RasterPipelineBundle build(std::vector<vk::DescriptorSetLayoutBinding> *bindings, vk::raii::Device &logical_device);


    // Helper functions
    static vk::raii::DescriptorSetLayout createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> &bindings, const vk::raii::Device &logical_device);
    static vk::raii::DescriptorPool createDescriptorPool(std::vector<vk::DescriptorSetLayoutBinding> &bindings, vk::raii::Device &logical_device, int max_frames_in_flight);
    static std::vector<vk::raii::DescriptorSet> createDescriptorSets(vk::raii::DescriptorSetLayout &descriptor_set_layout, vk::raii::DescriptorPool &descriptor_pool, vk::raii::Device &logical_device, int max_frames_in_flight);
    static void writeDescriptorSets(const std::vector<vk::raii::DescriptorSet> &descriptor_sets, const std::vector<vk::DescriptorSetLayoutBinding> &bindings, const std::vector<void *> &resources, vk::raii::Device &logical_device, const int max_frames_in_flight);

private:
    // Helper functions
    vk::raii::ShaderModule createShaderModule(const std::vector<char> &code, const vk::raii::Device &logical_device);
    std::vector<char> readFile(const std::string& filename);
};