#pragma once

#include "VulkanEngine/gameobject.hpp"


class Player : public Gameobject{
public:
    Player(
        glm::vec3 position = glm::vec3(0),
        float max_speed = 1.f,
        float friction = 0.1f,
        float acc = 0.3f,
        float jump_force = 5.f,
        float gravity_force = 2.f
    ) : Gameobject(position){
        this -> max_speed = max_speed;
        this -> friction = friction;
        this -> acc = acc;
        this -> jump_force = jump_force;
        this -> gravity_force = gravity_force;

        vertices = {
            // FRONT FACE (Normal 0, 0, 1)
            {glm::vec3(-0.5, 0.5, 0.5), glm::vec3(0, 0, 1), glm::vec3(0.5)}, // TL
            {glm::vec3(0.5, 0.5, 0.5),  glm::vec3(0, 0, 1), glm::vec3(0.5)}, // TR
            {glm::vec3(0.5, -0.5, 0.5), glm::vec3(0, 0, 1), glm::vec3(0.5)}, // BR
            {glm::vec3(-0.5, -0.5, 0.5),glm::vec3(0, 0, 1), glm::vec3(0.5)}, // BL

            // BACK FACE (Normal 0, 0, -1)
            {glm::vec3(0.5, 0.5, -0.5),  glm::vec3(0, 0, -1), glm::vec3(0.5)}, // TL
            {glm::vec3(-0.5, 0.5, -0.5), glm::vec3(0, 0, -1), glm::vec3(0.5)}, // TR
            {glm::vec3(-0.5, -0.5, -0.5),glm::vec3(0, 0, -1), glm::vec3(0.5)}, // BR
            {glm::vec3(0.5, -0.5, -0.5), glm::vec3(0, 0, -1), glm::vec3(0.5)}, // BL

            // RIGHT FACE (Normal 1, 0, 0)
            {glm::vec3(0.5, 0.5, 0.5),  glm::vec3(1, 0, 0), glm::vec3(0.5)}, // TL
            {glm::vec3(0.5, 0.5, -0.5), glm::vec3(1, 0, 0), glm::vec3(0.5)}, // TR
            {glm::vec3(0.5, -0.5, -0.5),glm::vec3(1, 0, 0), glm::vec3(0.5)}, // BR
            {glm::vec3(0.5, -0.5, 0.5), glm::vec3(1, 0, 0), glm::vec3(0.5)}, // BL

            // LEFT FACE (Normal -1, 0, 0)
            {glm::vec3(-0.5, 0.5, -0.5), glm::vec3(-1, 0, 0), glm::vec3(0.5)}, // TL
            {glm::vec3(-0.5, 0.5, 0.5),  glm::vec3(-1, 0, 0), glm::vec3(0.5)}, // TR
            {glm::vec3(-0.5, -0.5, 0.5), glm::vec3(-1, 0, 0), glm::vec3(0.5)}, // BR
            {glm::vec3(-0.5, -0.5, -0.5),glm::vec3(-1, 0, 0), glm::vec3(0.5)}, // BL

            // TOP FACE (Normal 0, 1, 0)
            {glm::vec3(-0.5, 0.5, -0.5), glm::vec3(0, 1, 0), glm::vec3(0.5)}, // TL
            {glm::vec3(0.5, 0.5, -0.5),  glm::vec3(0, 1, 0), glm::vec3(0.5)}, // TR
            {glm::vec3(0.5, 0.5, 0.5),   glm::vec3(0, 1, 0), glm::vec3(0.5)}, // BR
            {glm::vec3(-0.5, 0.5, 0.5),  glm::vec3(0, 1, 0), glm::vec3(0.5)}, // BL

            // BOTTOM FACE (Normal 0, -1, 0)
            {glm::vec3(-0.5, -0.5, 0.5), glm::vec3(0, -1, 0), glm::vec3(0.5)}, // TL
            {glm::vec3(0.5, -0.5, 0.5),  glm::vec3(0, -1, 0), glm::vec3(0.5)}, // TR
            {glm::vec3(0.5, -0.5, -0.5), glm::vec3(0, -1, 0), glm::vec3(0.5)}, // BR
            {glm::vec3(-0.5, -0.5, -0.5),glm::vec3(0, -1, 0), glm::vec3(0.5)}, // BL
        };

        indices = {
            // Front
            0, 1, 2,  0, 2, 3,
            // Back
            4, 5, 6,  4, 6, 7,
            // Right
            8, 9, 10, 8, 10, 11,
            // Left
            12, 13, 14, 12, 14, 15,
            // Top
            16, 17, 18, 16, 18, 19,
            // Bottom
            20, 21, 22, 20, 22, 23
        };
    }
    ~Player() = default;

    // Delete Copying
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;


    // Enable moving
    Player(Player&& other) noexcept 
        : Gameobject(std::move(other))
        {}

    Player& operator=(Player&& other) noexcept {
        if (this != &other) {
            Gameobject::operator=(std::move(other));
        }
        return *this;
    }




    void update(const float dtime) override;

    UniformBufferGameObjects& getUBO(){
        if(dirty_model){
            getModelMat();
        }

        ubo.model = model;

        return ubo;
    }

private:
    // Movement related variables (position already in Gameobject class)
    float max_speed;
    glm::vec3 velocity;

    float friction;
    float acc;
    glm::vec3 acceleration;

    float jump_force;
    float gravity_force;


};

