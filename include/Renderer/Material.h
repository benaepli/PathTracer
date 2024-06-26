#pragma once

#include <random>

#include <glm/glm.hpp>

#include "Renderer/Ray.h"

namespace pth
{
    struct Material
    {
        glm::vec3 color;
        float metallic;
        float roughness;
        float ior = 1.5F;
        glm::vec3 emissive;
    };
}  // namespace pth
