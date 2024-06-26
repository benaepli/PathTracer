#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace pth
{

    inline glm::vec3 fresnelSchlick(float cosTheta, glm::vec3 F0)
    {
        return F0 + (glm::vec3(1.F) - F0) * glm::pow(1.F - cosTheta, 5.F);
    }

        // inline auto calculateIncomingRay(const Ray& ray, const HitRecord& hitRecord, std::mt19937&
    // rng)
    //     -> RayResult
    //{
    //     std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    //     float u1 = distribution(rng);
    //     float u2 = distribution(rng);

    //    const float ior = hitRecord.material.ior;
    //    float F0 = glm::pow((1.0F - ior) / (1.0F + ior), 2.0F);
    //    float F = F0
    //        + (1.0F - F0) * glm::pow(1.0F - glm::dot(-ray.getDirection(),
    //        hitRecord.normal), 5.0F);

    //    float k = glm::pow(hitRecord.material.roughness + 1.F, 2.F) / 8.F;
    //    float G1 = 1 / (glm::dot(-ray.getDirection(), hitRecord.normal) * (1 - k) + k);
    //    float G2 = 1 / (glm::dot(-ray.getDirection(), hitRecord.normal) * (1 - k) + k);
    //    float G = G1 * G2;

    //    float alpha = glm::pow(hitRecord.material.roughness, 2.F);
    //    float D = glm::pow(alpha, 2.F)
    //        / (glm::pi<float>()
    //           * glm::pow(
    //               (glm::pow(alpha, 2.F) - 1.F) * glm::dot(hitRecord.normal, hitRecord.normal)
    //                   + 1.F,
    //               2.F));

    //    float diffuseWeight = (1.F - F) * (1.F - hitRecord.material.metallic);
    //    float specularWeight = F * hitRecord.material.metallic;

    //    float thetaL = glm::acos(glm::sqrt(u1));
    //    float phiL = 2.F * glm::pi<float>() * u2;
    //    glm::vec3 bounceL = glm::rotate(
    //        hitRecord.normal, thetaL, glm::cross(hitRecord.normal, -ray.getDirection()));
    //    bounceL = glm::rotate(bounceL, phiL, hitRecord.normal);

    //    float thetaH = glm::atan(glm::sqrt(alpha * u1 / (1.F - u1)));
    //    float phiH = 2.F * glm::pi<float>() * u2;
    //    glm::vec3 halfVector = glm::rotate(
    //        hitRecord.normal, thetaH, glm::cross(hitRecord.normal, -ray.getDirection()));
    //    halfVector = glm::rotate(halfVector, phiH, hitRecord.normal);
    //    glm::vec3 bounceC = glm::reflect(-ray.getDirection(), halfVector);

    //    glm::vec3 bounce = glm::normalize(diffuseWeight * bounceL + specularWeight * bounceC);
    //    Ray incomingRay(hitRecord.point, bounce);

    //    glm::vec3 BRDFL = hitRecord.material.color / glm::pi<float>();
    //    glm::vec3 BRDFC = F * G * D
    //        / (4 * glm::dot(ray.getDirection(), hitRecord.normal)
    //           * glm::dot(bounce, hitRecord.normal))
    //        * glm::vec3 {};

    //    glm::vec3 BRDF = diffuseWeight * BRDFL + specularWeight * BRDFC;

    //    float PDFL = glm::dot(bounceL, hitRecord.normal) / glm::pi<float>();
    //    float PDFC = glm::dot(bounceC, hitRecord.normal)
    //                    * glm::dot(halfVector, hitRecord.normal)
    //                    / (4 * glm::dot(bounceC, halfVector));

    //    float PDF = diffuseWeight * PDFL + specularWeight * PDFC;

    //    return { incomingRay, BRDF, PDF };
    //}

    // inline auto calculateIncomingRay(const Ray& ray, const HitRecord& hitRecord, std::mt19937&
    // rng)
    //     -> RayResult
    //{
    //     std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    //     float u1 = distribution(rng);
    //     float u2 = distribution(rng);

    //    const float ior = hitRecord.material.ior;
    //    float F0 = glm::pow((1.0F - ior) / (1.0F + ior), 2.0F);
    //    float F = F0
    //        + (1.0F - F0) * glm::pow(1.0F - glm::dot(-ray.getDirection(),
    //        hitRecord.normal), 5.0F);

    //    float k = glm::pow(hitRecord.material.roughness + 1.F, 2.F) / 8.F;
    //    float G1 = 1 / (glm::dot(-ray.getDirection(), hitRecord.normal) * (1 - k) + k);
    //    float G2 = 1 / (glm::dot(-ray.getDirection(), hitRecord.normal) * (1 - k) + k);
    //    float G = G1 * G2;

    //    float alpha = glm::pow(hitRecord.material.roughness, 2.F);
    //    float D = glm::pow(alpha, 2.F)
    //        / (glm::pi<float>()
    //           * glm::pow(
    //               (glm::pow(alpha, 2.F) - 1.F) * glm::dot(hitRecord.normal, hitRecord.normal)
    //                   + 1.F,
    //               2.F));

    //    float diffuseWeight = (1.F - F) * (1.F - hitRecord.material.metallic);
    //    float specularWeight = F * hitRecord.material.metallic;

    //    if (distribution(rng) < specularWeight)
    //    {
    //        float thetaH = glm::atan(glm::sqrt(alpha * u1 / (1.F - u1)));
    //        float phiH = 2.F * glm::pi<float>() * u2;
    //        glm::vec3 halfVector = glm::rotate(
    //            hitRecord.normal, thetaH, glm::cross(hitRecord.normal, -ray.getDirection()));
    //        halfVector = glm::rotate(halfVector, phiH, hitRecord.normal);
    //        glm::vec3 bounceC = glm::reflect(-ray.getDirection(), halfVector);

    //        glm::vec3 BRDFC = F * G * D
    //            / (4 * glm::dot(ray.getDirection(), hitRecord.normal)
    //               * glm::dot(bounceC, hitRecord.normal))
    //            * glm::vec3 {};

    //        float PDFC = glm::dot(bounceC, hitRecord.normal)
    //            * glm::dot(halfVector, hitRecord.normal) / (4 * glm::dot(bounceC, halfVector));

    //        Ray incomingRay(hitRecord.point, bounceC);
    //        return {incomingRay, BRDFC, PDFC};
    //    }
    //    else
    //    {
    //        float thetaL = glm::acos(glm::sqrt(u1));
    //        float phiL = 2.F * glm::pi<float>() * u2;
    //        glm::vec3 bounceL = glm::rotate(
    //            hitRecord.normal, thetaL, glm::cross(hitRecord.normal, -ray.getDirection()));
    //        bounceL = glm::rotate(bounceL, phiL, hitRecord.normal);

    //        glm::vec3 BRDFL = hitRecord.material.color / glm::pi<float>();
    //        float PDFL = glm::dot(bounceL, hitRecord.normal) / glm::pi<float>();

    //        glm::vec3 bounce = bounceL;
    //        Ray incomingRay(hitRecord.point, bounce);

    //        glm::vec3 BRDF = BRDFL;
    //        float PDF = PDFL;

    //        return {incomingRay, BRDF, PDF};
    //    }
    //}

 /*         float specular = hitRecord.material.metallic;
        float threshold = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);

        if (false)  // Cook-Torrance Specular BRDF
        {
            std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
            float randomFloat = distribution(rng);

            float alpha = hitRecord.material.roughness * hitRecord.material.roughness;
            float alphaSqr = alpha * alpha;

            float cosTheta = glm::sqrt(alphaSqr / ((randomFloat * (alphaSqr - 1.0F)) + 1.0F));
            float sinTheta = glm::sqrt(1.0F - cosTheta * cosTheta);

            float phi = 2.0F * glm::pi<float>() * distribution(rng);

            glm::vec3 H = glm::vec3(sinTheta * glm::cos(phi), sinTheta * glm::sin(phi), cosTheta);

            glm::vec3 up = glm::abs(hitRecord.normal.z) < 0.999F ? glm::vec3(0.0F, 0.0F, 1.0F)
                                                                 : glm::vec3(1.0F, 0.0F, 0.0F);
            glm::vec3 tangent = glm::normalize(glm::cross(up, hitRecord.normal));
            glm::vec3 bitangent = glm::cross(hitRecord.normal, tangent);

            glm::vec3 specularDirection =
                glm::normalize(tangent * H.x + bitangent * H.y + hitRecord.normal * H.z);
            float specularPDF = alphaSqr * cosTheta
                / (glm::pi<float>()
                   * glm::pow(cosTheta * cosTheta * (alphaSqr - 1.0F) + 1.0F, 2.0F));

            float D = alphaSqr
                / (glm::pi<float>()
                   * glm::pow(cosTheta * cosTheta * (alphaSqr - 1.0F) + 1.0F, 2.0F));

            glm::vec3 F0 = glm::vec3(0.04F);
            glm::vec3 F = fresnelSchlick(glm::dot(ray.getDirection(), specularDirection), F0);

            float G = glm::min(1.0F,
                               glm::min(2.0F * cosTheta * glm::dot(hitRecord.normal, H)
                                            / glm::dot(ray.getDirection(), H),
                                        2.0F * cosTheta * glm::dot(hitRecord.normal, H)
                                            / glm::dot(specularDirection, H)));

            glm::vec3 specularBRDF = (D * F * G)
                / (4.0F * glm::dot(hitRecord.normal, ray.getDirection())
                   * glm::dot(hitRecord.normal, specularDirection));

            return RayResult {Ray(hitRecord.point, specularDirection), specularBRDF, specularPDF};
        }
        else  // Simple Lambertian BRDF with cosine-weighted sampling
        {
            // Calculate random direction in hemisphere
            glm::vec3 randomDirection = randomInUnitSphere(rng);
            if (glm::dot(randomDirection, hitRecord.normal) < 0.0F)
            {
                randomDirection = -randomDirection;
            }

            // Calculate PDF
            float PDF = glm::dot(hitRecord.normal, randomDirection) / glm::pi<float>();

            float cosTheta = glm::dot(hitRecord.normal, randomDirection);
            glm::vec3 F0 = glm::vec3(0.04F);
            glm::vec3 F = fresnelSchlick(cosTheta, F0);

            glm::vec3 diffuseBRDF = (1.0F - F) * hitRecord.material.color / glm::pi<float>();

            return RayResult {Ray(hitRecord.point, randomDirection), diffuseBRDF, PDF};
        }*/

}  // namespace pth
