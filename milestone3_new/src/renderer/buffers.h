#pragma once
#include "../mesh.h"
#include "vk_utils.h"
#include <glm/glm.hpp>

struct UBOData {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

bool create_mesh_buffers(VkContext* vkcontext, const Mesh& mesh);
void destroy_mesh_buffers(VkContext* vkcontext);

bool create_ubo(VkContext* vkcontext);
void update_ubo(VkContext* vkcontext, float aspectRatio, float avg_dt);
void destroy_ubo(VkContext* vkcontext);