#include "scene.hpp"

void Scene::createInitResources()
{
    // Setting up the player
    player = Player();
    player.start(vma_allocator, logical_device, queue_pool);

    ubo_player_mapped.clear();
    ubo_player_mapped.resize(queue_pool.max_frames_in_flight);
    vk::DeviceSize player_buffer_size = sizeof(UniformBufferGameObjects);

    for(size_t i = 0; i < queue_pool.max_frames_in_flight; i++){
        ubo_player_mapped[i].buffer = Device::createBuffer(
            player_buffer_size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            "Player Buffer",
            vma_allocator
        );
        vmaMapMemory(vma_allocator, ubo_player_mapped[i].buffer.allocation, &ubo_player_mapped[i].data);
    }

    // Setting up the environment
    ground = Plane(glm::vec3(0.0f), 10.f, 10.f, glm::vec3(90.f, 0.f, 0.f));
    ground.start(vma_allocator, logical_device, queue_pool);
    current_env_objs++;

    ubo_environment_mapped.clear();
    ubo_environment_mapped.resize(MAX_ENV_OBJS);
    vk::DeviceSize env_buffer_size = sizeof(UniformBufferGameObjects);
    for(size_t i = 0; i < MAX_ENV_OBJS; i++){
        ubo_environment_mapped[i].clear();
        ubo_environment_mapped[i].resize(queue_pool.max_frames_in_flight);
        for(size_t j = 0; j < queue_pool.max_frames_in_flight; j++){
            ubo_environment_mapped[i][j].buffer = Device::createBuffer(
                env_buffer_size,
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                "Env obj Buffer",
                vma_allocator
            );
            vmaMapMemory(vma_allocator, ubo_environment_mapped[i][j].buffer.allocation, &ubo_environment_mapped[i][j].data);
        }
    }


    // CAMERA RESOURCES SETUP
    ubo_camera_mapped.clear();
    ubo_camera_mapped.resize(queue_pool.max_frames_in_flight);
    vk::DeviceSize camera_buffer_size = sizeof(UniformBufferCamera);

    for(size_t i = 0; i < queue_pool.max_frames_in_flight; i++){
        ubo_camera_mapped[i].buffer = Device::createBuffer(
            camera_buffer_size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            "Camera Buffer",
            vma_allocator
        );
        vmaMapMemory(vma_allocator, ubo_camera_mapped[i].buffer.allocation, &ubo_camera_mapped[i].data);
    }

    camera = Camera(glm::vec3(0, 0, 2));



    // Pipeline setup
    const std::string vertex_shader_path = "Shaders/Samples/vertex.vert.spv";
    const std::string fragment_shader_path = "Shaders/Samples/fragment.frag.spv";

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        // Binding 0: Camera Uniform Object
        vk::DescriptorSetLayoutBinding(
            0, // binding location
            vk::DescriptorType::eUniformBuffer, // Type of binding
            1, // binding count
            vk::ShaderStageFlagBits::eVertex,
            nullptr
        ),

        // Binding 1: GameObject Uniform Buffer
        vk::DescriptorSetLayoutBinding(
            1,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eVertex,
            nullptr
        ),

        // Biding 2: Environment Uniform Buffers
        vk::DescriptorSetLayoutBinding(
            1,
            vk::DescriptorType::eUniformBuffer,
            MAX_ENV_OBJS,
            vk::ShaderStageFlagBits::eVertex,
            nullptr
        )
    };
    std::string name = "dumb pipeline";

    pipeline_builder.set_name(name);
    pipeline_builder.add_shader(vertex_shader_path, vk::ShaderStageFlagBits::eVertex, logical_device);
    pipeline_builder.add_shader(fragment_shader_path, vk::ShaderStageFlagBits::eFragment, logical_device);
    pipeline_builder.set_topology(vk::PrimitiveTopology::eTriangleList);
    pipeline_builder.set_rasterizer(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::PolygonMode::eFill);
    pipeline_builder.set_multisampling_none();
    pipeline_builder.add_non_blend_color_attachment(vk::ColorComponentFlagBits::eR | 
                                                    vk::ColorComponentFlagBits::eG | 
                                                    vk::ColorComponentFlagBits::eB | 
                                                    vk::ColorComponentFlagBits::eA);
    pipeline_builder.set_color_and_depth_format({swapchain.format}, Image::findDepthFormat(physical_device));
    pipeline_builder.set_depth_stencil(true, true, vk::CompareOp::eLess);

    main_pipeline = pipeline_builder.build(&bindings, logical_device);
    
    main_pipeline.descriptor_pool = PipelineBuilder::createDescriptorPool(bindings, logical_device, queue_pool.max_frames_in_flight);
    main_pipeline.descriptor_sets = PipelineBuilder::createDescriptorSets(main_pipeline.descriptor_set_layout,
                                                                        main_pipeline.descriptor_pool,
                                                                        logical_device,
                                                                        queue_pool.max_frames_in_flight);
    std::vector<void *> resources{
        &ubo_camera_mapped,
        &ubo_player_mapped,
        &ubo_environment_mapped
    };
    PipelineBuilder::writeDescriptorSets(main_pipeline.descriptor_sets, bindings, resources, logical_device, queue_pool.max_frames_in_flight);

}

void Scene::updateUniformBuffers(float dtime, int current_frame)
{
    UniformBufferCamera ubo_camera;

    ubo_camera.view = camera.getViewMatrix();
    ubo_camera.proj = camera.getProjectionMatrix(swapchain.extent.width * 1.f / swapchain.extent.height);

    memcpy(ubo_camera_mapped[current_frame].data, &ubo_camera, sizeof(UniformBufferCamera));

    memcpy(ubo_player_mapped[current_frame].data, &player.getUBO(), sizeof(UniformBufferGameObjects));
}

void Scene::recordCommandBuffer(uint32_t image_index)
{
    vk::raii::CommandBuffer &command_buffer = queue_pool.graphics_command_buffers[current_frame];
    command_buffer.begin({});

    Image::transitionImageLayout(swapchain.images[image_index], 
            vk::ImageLayout::eUndefined,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    {},                                                        // srcAccessMask (no need to wait for previous operations)
		    vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
            vk::ImageAspectFlagBits::eColor,
            command_buffer
    );
    vk::ClearValue  clear_color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    vk::RenderingAttachmentInfo attachment_info{};
    attachment_info.imageView = swapchain.image_views[image_index];
    attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachment_info.loadOp = vk::AttachmentLoadOp::eClear;
    attachment_info.storeOp = vk::AttachmentStoreOp::eStore;
    attachment_info.clearValue = clear_color;

    vk::RenderingInfo rendering_info{};
    rendering_info.renderArea.offset = vk::Offset2D{0, 0};
    rendering_info.renderArea.extent = swapchain.extent;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &attachment_info;

    command_buffer.beginRendering(rendering_info);
    command_buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f)); // What portion of the window to use
    command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.extent)); // What portion of the image to use
    
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *(main_pipeline.pipeline));
    command_buffer.setCullMode(main_pipeline.rasterizer.cullMode);
    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        main_pipeline.layout,
        0,
        *main_pipeline.descriptor_sets[current_frame],
        {}
    );
    UniformBufferGameObjects ubo_obj;
    command_buffer.bindVertexBuffers(0, player.getVertexBuffer(), {0});
    command_buffer.bindIndexBuffer(player.getIndexBuffer(), 0, vk::IndexType::eUint32);
    command_buffer.drawIndexed(player.getIndexSize(), 1, 0, 0, 0);
    
    command_buffer.endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    Image::transitionImageLayout(
        swapchain.images[image_index],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
        {},                                                        // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe,                 // dstStage
        vk::ImageAspectFlagBits::eColor,
        command_buffer
    );
    command_buffer.end();

    glfwSetWindowTitle(window, std::to_string(1000.0/time).c_str());
}

void Scene::processInput()
{
}




void Scene::cleanup()
{
    std::cout << "\nCLEANING UP RESOURCES..." << std::endl;
    ubo_player_mapped.clear();
    player = {};

    ubo_camera_mapped.clear();

    // Destroying the images -> this is needed since we need to destroy the allocator
    color_image.~AllocatedImage();
    depth_image.~AllocatedImage();

    vmaDestroyAllocator(vma_allocator);
}

