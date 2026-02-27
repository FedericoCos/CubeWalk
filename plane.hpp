#pragma once

#include "VulkanEngine/gameobject.hpp"

class Plane : public Gameobject{
public:
    Plane(
        glm::vec3 position = glm::vec3(0),
        float base,
        float heihgt,
        glm::vec3 rotation = glm::vec3(0),
        glm::vec3 color = glm::vec3(0.5f)
    ) : Gameobject(position, glm::vec3(1.f), rotation){
        this -> base = base;
        this -> height = height;

        float half_base = base / 2.0;
        float half_height = height / 2.0;

        vertices = {
            {glm::vec3(-half_base, half_height, 0), glm::vec3(0, 0, 1), color},
            {glm::vec3(half_base, half_height, 0),  glm::vec3(0, 0, 1), color}, 
            {glm::vec3(half_base, -half_height, 0), glm::vec3(0, 0, 1), color}, 
            {glm::vec3(-half_base, -half_height, 0),glm::vec3(0, 0, 1), color}
        };

        indices = {
            0, 1, 2,
            0, 2, 3
        };
    }

private:
    float base;
    float height;
};