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

    struct ScatterResult
    {
        glm::vec3 attenuation;
        Ray scatteredRay;
    };
}  // namespace pth
