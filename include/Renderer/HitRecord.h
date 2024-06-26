#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Math/PBR.h"
#include "Renderer/Material.h"

namespace pth
{
    struct HitRecord
    {
        glm::vec3 point;
        glm::vec3 normal;
        double distance;
        bool frontFace;
        Material material;
    };

    inline glm::vec3 randomInUnitSphere(std::mt19937& rng)
    {
        float x = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng);
        float y = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng);
        float z = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng);
        glm::vec3 p(x, y, z);
        return glm::normalize(p);
    }

    inline auto fresnelMix(float ior, glm::vec3 diffuse, glm::vec3 specular, float VdotH)
        -> glm::vec3
    {
        float F0sqrt = (1.0F - ior) / (1.0F + ior);
        float F0 = F0sqrt * F0sqrt;
        float F = F0 + (1.0F - F0) * glm::pow(1.0F - VdotH, 5.0F);
        return glm::mix(diffuse, specular, F);
    }

    inline auto conductorFresnel(glm::vec3 F0, glm::vec3 BSDF, float VdotH) -> glm::vec3
    {
        return BSDF * (F0 + (1.0F - F0) * glm::pow(1.0F - VdotH, 5.0F));
    }

    inline auto diffuseBRDF(glm::vec3 color) -> glm::vec3
    {
        return color * glm::one_over_pi<float>();
    }

    inline auto GGX_D(float alpha, glm::vec3 H, glm::vec3 N) -> float
    {
        float NdotH = glm::dot(N, H);
        float alpha2 = alpha * alpha;
        float top = alpha2 * (NdotH > 0.F ? 1.F : 0.F);

        float bottom = glm::pi<float>() * glm::pow(NdotH * NdotH * (alpha2 - 1.F) + 1.F, 2.F);
        return top / bottom;
    }

    inline auto GGX_V(float alpha, glm::vec3 H, glm::vec3 V, glm::vec3 N) -> float
    {
        float HdotL = glm::dot(H, V);
        float NdotL = glm::dot(N, V);
        float NdotV = glm::dot(N, V);

        float alpha2 = alpha * alpha;

        float top = HdotL > 0.F ? 1.F : 0.F;
        float bottom = NdotL * glm::sqrt(alpha2 + (1.F - alpha2) * NdotV * NdotV);

        return top / bottom;
    }

    inline auto specularBRDF(glm::vec3 color, float alpha, glm::vec3 H, glm::vec3 N) -> glm::vec3
    {
        float D = pth::GGX_D(alpha, H, N);
        float V = pth::GGX_V(alpha, H, H, N);

        return color * D * V;
    }

    inline auto calculateBRDF(glm::vec3 incomingRay,
                              glm::vec3 outgoingRay,
                              const HitRecord& hitRecord) -> glm::vec3
    {
        /* First,
         we calculate the dielectric BRDF thorugh the Cook-Torrance Specular BRDF and the
         Lambertian Diffuse BRDF. We mix the specular and diffuse BRDFs together through a Fresnel
         approximation (Schlick). After, we calculate the metal BRDF by multiplying the
         specular BRDF with a conductor Fresnel approximation (Schlick). */

        glm::vec3 H = glm::normalize(incomingRay + outgoingRay);
        float VdotH = glm::dot(outgoingRay, H);

        // Diffuse BRDF
        glm::vec3 diffuse = diffuseBRDF(hitRecord.material.color);

        // Specular BRDF
        float alpha = hitRecord.material.roughness * hitRecord.material.roughness;
        glm::vec3 specular = specularBRDF(hitRecord.material.color, alpha, H, hitRecord.normal);

        glm::vec3 dielectric = fresnelMix(hitRecord.material.ior, diffuse, specular, VdotH);

        // Metal BRDF
        glm::vec3 F0 = glm::vec3(0.04F);
        glm::vec3 metal = conductorFresnel(F0, specular, VdotH);

        // Mix the dielectric and metal BRDFs together
        return glm::mix(dielectric, metal, hitRecord.material.metallic);
    }

    inline auto sampleRay(const Ray& ray, const HitRecord& hitRecord, std::mt19937& rng)
        -> Ray
    {
        glm::vec3 target = hitRecord.point + hitRecord.normal + randomInUnitSphere(rng);
		glm::vec3 direction = glm::normalize(target - hitRecord.point);
		return Ray(hitRecord.point, direction);
    }

    inline auto getProbabilityDistribution(const Ray& ray, const HitRecord& hitRecord)
        -> float
    {
		return 1.0F / (4.0F * glm::pi<float>());
	}

}  // namespace pth
