#include "pipeline.hpp"

void PipelineBuilder::set_name(std::string name)
{
    pipeline_bundle.name = std::move(name);
}

void PipelineBuilder::add_shader(std::string path, vk::ShaderStageFlagBits stage, vk::raii::Device &logical_device)
{
    pipeline_bundle.shaders.push_back(createShaderModule(readFile(path), logical_device));

    vk::PipelineShaderStageCreateInfo shader_info;
    shader_info.stage = stage;
    shader_info.module = *pipeline_bundle.shaders[pipeline_bundle.shaders.size() - 1];
    shader_info.pName = "main";

    pipeline_bundle.shader_stages.push_back(shader_info);
}

void PipelineBuilder::set_topology(vk::PrimitiveTopology topology)
{
    vk::PipelineInputAssemblyStateCreateInfo input_assembly;
    input_assembly.topology = topology;
    input_assembly.primitiveRestartEnable = vk::False;

    pipeline_bundle.input_assembly = input_assembly;
}
void PipelineBuilder::set_rasterizer(vk::CullModeFlags cull_mode, vk::FrontFace front_face, vk::PolygonMode mode)
{
    pipeline_bundle.rasterizer.cullMode = cull_mode;
    pipeline_bundle.rasterizer.frontFace = front_face;
    pipeline_bundle.rasterizer.polygonMode = mode;
    pipeline_bundle.rasterizer.lineWidth = 1.f;
}

void PipelineBuilder::set_multisampling_none()
{
    pipeline_bundle.multisampling.sampleShadingEnable = vk::False;
    pipeline_bundle.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    pipeline_bundle.multisampling.minSampleShading = 1.f;
    pipeline_bundle.multisampling.pSampleMask = nullptr;
    pipeline_bundle.multisampling.alphaToCoverageEnable = vk::False;
    pipeline_bundle.multisampling.alphaToOneEnable = vk::False;
}

void PipelineBuilder::add_non_blend_color_attachment(vk::ColorComponentFlags write_mask)
{
    vk::PipelineColorBlendAttachmentState color_blend_attachment;
    color_blend_attachment.blendEnable = vk::False;
    color_blend_attachment.colorWriteMask = write_mask;
    pipeline_bundle.color_blend_attachments.push_back(color_blend_attachment);
}

void PipelineBuilder::set_color_and_depth_format(std::vector<vk::Format> color_formats, vk::Format depth_format)
{
    pipeline_bundle.color_formats = std::move(color_formats);

    pipeline_bundle.pipeline_rendering_create_info.pColorAttachmentFormats = pipeline_bundle.color_formats.data();
    pipeline_bundle.pipeline_rendering_create_info.colorAttachmentCount = pipeline_bundle.color_formats.size();
    pipeline_bundle.pipeline_rendering_create_info.depthAttachmentFormat = depth_format;
}

void PipelineBuilder::set_depth_stencil(bool depth_test_enable, bool depth_write_enable, vk::CompareOp op)
{
    pipeline_bundle.depth_stencil.depthTestEnable = depth_test_enable;
    pipeline_bundle.depth_stencil.depthWriteEnable = depth_write_enable;
    pipeline_bundle.depth_stencil.depthBoundsTestEnable = vk::False;
    pipeline_bundle.depth_stencil.stencilTestEnable = vk::False;
    pipeline_bundle.depth_stencil.depthCompareOp = op;
}

RasterPipelineBundle PipelineBuilder::build(std::vector<vk::DescriptorSetLayoutBinding> *bindings, vk::raii::Device &logical_device)
{
    pipeline_bundle.descriptor_set_layout = createDescriptorSetLayout(*bindings, logical_device);


    // Vertex components
    vk::VertexInputBindingDescription binding_description = Vertex::getBindingDescription();
    auto attribute_descriptions = Vertex::getAttributeDescriptions(); // Here auto since probably the array will change in future, it is just safer

    vk::PipelineVertexInputStateCreateInfo vertex_input_info;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description; 
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()); // Good practice to cast
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data(); 

    // Layout create info
    vk::PipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &*(pipeline_bundle.descriptor_set_layout);    
    pipeline_bundle.layout = vk::raii::PipelineLayout(logical_device, pipeline_layout_info);

    // Dynamic states
    std::vector dynamic_states = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eCullMode
    };
    vk::PipelineDynamicStateCreateInfo dynamic_state;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    // Viewport information
    vk::PipelineViewportStateCreateInfo viewport_state;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    // Finalizing color attachments
    pipeline_bundle.color_blending.logicOpEnable = vk::False;
    pipeline_bundle.color_blending.attachmentCount = pipeline_bundle.color_blend_attachments.size();
    pipeline_bundle.color_blending.pAttachments = pipeline_bundle.color_blend_attachments.data();

    vk::GraphicsPipelineCreateInfo pipeline_info;
    pipeline_info.pNext = &pipeline_bundle.pipeline_rendering_create_info;
    pipeline_info.stageCount = pipeline_bundle.shader_stages.size();
    pipeline_info.pStages = pipeline_bundle.shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &pipeline_bundle.input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &pipeline_bundle.rasterizer;
    pipeline_info.pColorBlendState = &pipeline_bundle.color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = pipeline_bundle.layout;
    pipeline_info.pDepthStencilState = &pipeline_bundle.depth_stencil;
    pipeline_info.pMultisampleState = &pipeline_bundle.multisampling;

    pipeline_bundle.pipeline = vk::raii::Pipeline(logical_device, nullptr, pipeline_info);

    std::cout << "Created Pipeline:\n" << pipeline_bundle.to_str() << std::endl;


    return std::move(pipeline_bundle);
}

vk::raii::ShaderModule PipelineBuilder::createShaderModule(const std::vector<char> &code, const vk::raii::Device &logical_device)
{
    vk::ShaderModuleCreateInfo create_info;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    vk::raii::ShaderModule shader_module{logical_device, create_info};
    return std::move(shader_module);
}

std::vector<char> PipelineBuilder::readFile(const std::string& filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        throw std::runtime_error("failed to open file: " + filename);
    }
    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}


vk::raii::DescriptorSetLayout PipelineBuilder::createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> &bindings, const vk::raii::Device &logical_device)
{
    vk::DescriptorSetLayoutCreateInfo layout_info({}, bindings.size(), bindings.data());

    return std::move(vk::raii::DescriptorSetLayout(logical_device, layout_info));
}

vk::raii::DescriptorPool PipelineBuilder::createDescriptorPool(std::vector<vk::DescriptorSetLayoutBinding> &bindings, vk::raii::Device &logical_device, int max_frames_in_flight)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes;

    int total_uniform_buffers = 0;
    int total_ssbo = 0;
    for(size_t i = 0; i < bindings.size(); i++){
        if(bindings[i].descriptorType == vk::DescriptorType::eUniformBuffer){
            total_uniform_buffers += bindings[i].descriptorCount;
        }
        else if(bindings[i].descriptorType == vk::DescriptorType::eStorageBuffer){
            total_ssbo += bindings[i].descriptorCount;
        }
    }

    if(total_uniform_buffers > 0){
        pool_sizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, total_uniform_buffers));
    }
    if(total_ssbo > 0){
        pool_sizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, total_ssbo));
    }

    vk::DescriptorPoolCreateInfo pool_info;
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = max_frames_in_flight;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size()); 
    pool_info.pPoolSizes = pool_sizes.data();

    return std::move(vk::raii::DescriptorPool(logical_device, pool_info));
}

std::vector<vk::raii::DescriptorSet> PipelineBuilder::createDescriptorSets(vk::raii::DescriptorSetLayout &descriptor_set_layout, vk::raii::DescriptorPool &descriptor_pool, vk::raii::Device &logical_device, int max_frames_in_flight)
{
    std::vector<vk::raii::DescriptorSet> descriptor_sets;

    vk::DescriptorSetAllocateInfo alloc_info(*descriptor_pool, 1, &*descriptor_set_layout);

    for(size_t i = 0; i < max_frames_in_flight; i++){
        descriptor_sets.push_back(std::move(logical_device.allocateDescriptorSets(alloc_info).front()));
    }

    return std::move(descriptor_sets);
}

// TODO: Chenge from void * to a more safe option
void PipelineBuilder::writeDescriptorSets(
    const std::vector<vk::raii::DescriptorSet> &descriptor_sets, 
    const std::vector<vk::DescriptorSetLayoutBinding> &bindings, 
    const std::vector<void *> &resources, 
    vk::raii::Device &logical_device, 
    const int max_frames_in_flight) 
{
    for (size_t i = 0; i < max_frames_in_flight; i++) {
        std::vector<vk::WriteDescriptorSet> writes;
        
        std::deque<std::vector<vk::DescriptorBufferInfo>> multi_buffer_infos;
        std::deque<vk::DescriptorBufferInfo> single_buffer_infos;

        for (size_t j = 0; j < bindings.size(); j++) {
            if (bindings[j].descriptorType == vk::DescriptorType::eUniformBuffer || bindings[j].descriptorType == vk::DescriptorType::eStorageBuffer) {
                
                if (bindings[j].descriptorCount > 1) {
                    auto &info_vec = multi_buffer_infos.emplace_back();
                    info_vec.reserve(bindings[j].descriptorCount);

                    auto* res_ptr = static_cast<std::vector<std::vector<MappedUBO>>*>(resources[j]);
                    
                    for (size_t k = 0; k < bindings[j].descriptorCount; k++) {
                        AllocatedBuffer &buffer = (*res_ptr)[k][i].buffer;
                        info_vec.push_back(vk::DescriptorBufferInfo{buffer.buffer, 0, buffer.size});
                    }

                    writes.push_back(vk::WriteDescriptorSet{
                        *descriptor_sets[i], bindings[j].binding, 0,
                        bindings[j].descriptorCount, bindings[j].descriptorType,
                        nullptr, info_vec.data(), nullptr
                    });
                } 
                else {
                    auto* res_ptr = static_cast<std::vector<MappedUBO>*>(resources[j]);
                    AllocatedBuffer &buffer = (*res_ptr)[i].buffer;
                    
                    vk::DescriptorBufferInfo &info = single_buffer_infos.emplace_back(
                        buffer.buffer, 0, buffer.size
                    );

                    writes.push_back(vk::WriteDescriptorSet{
                        *descriptor_sets[i], bindings[j].binding, 0,
                        1, bindings[j].descriptorType,
                        nullptr, &info, nullptr
                    });
                }
            }
        }

        if (!writes.empty()) {
            logical_device.updateDescriptorSets(writes, nullptr);
        }

        multi_buffer_infos.clear();
        single_buffer_infos.clear();
    }
}
