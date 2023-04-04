#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

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

    // inline auto scatterMaterial(const Ray& ray, const HitRecord& hitRecord, std::mt19937& rng)
    //     -> ScatterResult
    //{
    //     ScatterResult result;
    //     result.attenuation = hitRecord.material.color;

    //    glm::vec3 reflected = glm::reflect(glm::normalize(ray.getDirection()), hitRecord.normal);
    //    glm::vec3 scattered = reflected + hitRecord.material.roughness * randomInUnitSphere(rng);
    //    glm::vec3 fuzz = hitRecord.material.metallic * randomInUnitSphere(rng);
    //    scattered += fuzz;

    //    glm::vec3 refracted;
    //    float etaRatio = hitRecord.frontFace ? (1.0F / hitRecord.material.ior) :
    //    hitRecord.material.ior; glm::vec3 unitDirection = glm::normalize(ray.getDirection());
    //    float cosTheta = glm::min(glm::dot(-unitDirection, hitRecord.normal), 1.0F);
    //    float sinTheta = glm::sqrt(1.0F - cosTheta * cosTheta);
    //    if (etaRatio * sinTheta > 1.0f)
    //    {
    //        refracted = glm::reflect(unitDirection, hitRecord.normal);
    //    }
    //    else
    //    {
    //        refracted = glm::refract(unitDirection, hitRecord.normal, etaRatio);
    //    }

    //    result.scatteredRay = Ray(hitRecord.point, scattered);

    //    return result;
    //}

    inline float randomFloat(std::mt19937& rng)
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng);
    }

    inline float schlick(float cosTheta, float etaRatio)
    {
        float r0 = (1.0F - etaRatio) / (1.0F + etaRatio);
        r0 = r0 * r0;
        return r0 + (1.0F - r0) * glm::pow((1.0F - cosTheta), 5);
    }

    // inline auto scatterMaterial(const Ray& ray, const HitRecord& hitRecord, std::mt19937& rng)
    //     -> ScatterResult
    //{
    //     ScatterResult result;
    //     result.attenuation = hitRecord.material.color;
    //     glm::vec3 reflected = glm::reflect(glm::normalize(ray.getDirection()), hitRecord.normal);
    //     glm::vec3 scattered = reflected + hitRecord.material.roughness * randomInUnitSphere(rng);
    //     glm::vec3 fuzz = hitRecord.material.metallic * randomInUnitSphere(rng);
    //     scattered += fuzz;
    //     glm::vec3 refracted;
    //     float etaRatio =
    //         hitRecord.frontFace ? (1.0F / hitRecord.material.ior) : hitRecord.material.ior;
    //     glm::vec3 unitDirection = glm::normalize(ray.getDirection());
    //     float cosTheta = glm::min(glm::dot(-unitDirection, hitRecord.normal), 1.0F);
    //     float sinTheta = glm::sqrt(1.0F - cosTheta * cosTheta);
    //     if (etaRatio * sinTheta > 1.0f)
    //     {
    //         refracted = glm::reflect(unitDirection, hitRecord.normal);
    //     }
    //     else
    //     {
    //         refracted = glm::refract(unitDirection, hitRecord.normal, etaRatio);
    //     }

    //    // Added code for specular reflection
    //    float reflectProb =
    //        schlick(cosTheta, etaRatio);  // Schlick's approximation for Fresnel factor
    //    if (randomFloat(rng) < reflectProb)
    //    {  // Randomly choose between reflection and refraction based on Fresnel factor
    //        result.scatteredRay =
    //            Ray(hitRecord.point, reflected);  // Use reflected ray for specular reflection
    //    }
    //    else
    //    {
    //        result.scatteredRay =
    //            Ray(hitRecord.point, refracted);  // Use refracted ray for transmission
    //    }

    //    return result;
    //}

    inline auto scatterMaterial(const Ray& ray, const HitRecord& hitRecord, std::mt19937& rng)
        -> ScatterResult
    {
        ScatterResult result;
        result.attenuation = hitRecord.material.color;
        glm::vec3 reflected = glm::reflect(glm::normalize(ray.getDirection()), hitRecord.normal);
        glm::vec3 scattered = reflected + hitRecord.material.roughness * randomInUnitSphere(rng);
        glm::vec3 fuzz = hitRecord.material.metallic * randomInUnitSphere(rng);
        scattered += fuzz;
        glm::vec3 refracted;
        float etaRatio =
            hitRecord.frontFace ? (1.0F / hitRecord.material.ior) : hitRecord.material.ior;
        glm::vec3 unitDirection = glm::normalize(ray.getDirection());
        float cosTheta = glm::min(glm::dot(-unitDirection, hitRecord.normal), 1.0F);
        float sinTheta = glm::sqrt(1.0F - cosTheta * cosTheta);
        if (etaRatio * sinTheta > 1.0f)
        {
            refracted = glm::reflect(unitDirection, hitRecord.normal);
        }
        else
        {
            refracted = glm::refract(unitDirection, hitRecord.normal, etaRatio);
        }

        // Modified code for BSDF model
        float reflectProb =
            schlick(cosTheta, etaRatio);  // Schlick's approximation for Fresnel factor
        float diffuseProb = (1.0F - reflectProb)
            * (1.0F - hitRecord.material.metallic);  // Probability of diffuse reflection
        float specularProb = (1.0F - reflectProb)
            * hitRecord.material.metallic;  // Probability of specular reflection
        float randomProb = randomFloat(rng);  // Random number between 0 and 1
        if (randomProb < reflectProb)
        {
            // Use reflected ray for total internal reflection or high Fresnel factor
            result.scatteredRay = Ray(hitRecord.point, reflected);
        }
        else if (randomProb < reflectProb + diffuseProb)
        {
            // Use scattered ray for diffuse reflection
            result.scatteredRay = Ray(hitRecord.point, scattered);
        }
        else
        {
            // Use refracted ray for transmission or low Fresnel factor
            result.scatteredRay = Ray(hitRecord.point, refracted);
        }

        return result;
    }

}  // namespace pth
