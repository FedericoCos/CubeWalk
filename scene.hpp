#pragma once

#include "VulkanEngine/engine.hpp"
#include "player.hpp"

class Scene : public Engine{
public:

    // Virtual functions from Engine class
    void cleanup() override;

private:
    // Main pipeline
    RasterPipelineBundle main_pipeline;

    // Player related variables
    Player player;
    std::vector<MappedUBO> ubo_player_mapped;

    // Camera variables
    

    // Virtual functions from Engine class
    void createInitResources() override;
    void updateUniformBuffers(float dtime, int current_frame) override;
    void recordCommandBuffer(uint32_t image_index) override;
    void processInput() override;

};