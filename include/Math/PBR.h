#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace pth
{
    inline glm::vec3 sampleGGX(float r1, float r2, glm::vec3 N, float roughness)
    {
        // Stretch the normal to account for roughness.
        glm::vec3 T = glm::normalize(glm::cross(N, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 B = glm::cross(N, T);
        glm::vec3 H = glm::normalize(
            T * r1 + B * r2
            + N * sqrtf(glm::max(0.0f, 1.0f - r1 * r1 - r2 * r2) / (1.0f + roughness * roughness)));

        return H;
    }

    inline glm::vec3 fresnelSchlick(float cosTheta, glm::vec3 F0)
    {
        return F0 + (glm::vec3(1.F) - F0) * glm::pow(1.F - cosTheta, 5.F);
    }

    inline float distributionGGX(glm::vec3 N, glm::vec3 H, float roughness)
    {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = std::max(dot(N, H), 0.F);
        float NdotH2 = NdotH * NdotH;

        float num = a2;
        float denom = (NdotH2 * (a2 - 1.F) + 1.F);
        denom = glm::pi<float>() * denom * denom;

        return num / std::max(denom, 0.0001F);  // Prevent divide by zero for roughness=0.0
    }

    inline float geometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.F);
        float k = (r * r) / 8.F;

        float num = NdotV;
        float denom = NdotV * (1.F - k) + k;

        return num / denom;
    }

    inline float geometrySmith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness)
    {
        float NdotV = std::max(dot(N, V), 0.F);
        float NdotL = std::max(dot(N, L), 0.F);
        float ggx2 = geometrySchlickGGX(NdotV, roughness);
        float ggx1 = geometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

}  // namespace pth
