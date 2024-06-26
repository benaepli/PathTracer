#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/Ray.h"
#include "exage/Scene/Hierarchy.h"

namespace pth
{
    struct CameraComponent
    {
        float fovY = 45.F;
        float nearPlane = 0.1F;
        float farPlane = 10000.F;
    };

    [[nodiscard]] inline auto computeCameraRay(glm::uvec2 extent,
                         uint32_t x,
                         uint32_t y,
                         float fov,
                         glm::vec3 position,
                         glm::quat rotation) -> Ray
    {
        float aspectRatio = static_cast<float>(extent.x) / static_cast<float>(extent.y);
        float tanHalfFov = tan(glm::radians(fov) / 2.0f);

        // Compute direction vector
        glm::vec3 direction = glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));

        // Compute right vector
        glm::vec3 right = glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f));

        // Compute up vector
        glm::vec3 up = glm::normalize(glm::cross(right, direction));

        // Compute viewport height and width
        float viewportHeight = 2.0f * tanHalfFov;
        float viewportWidth = aspectRatio * viewportHeight;

        // Compute normalized device coordinates (NDC)
        float ndcX = (2.0f * x + 1.0f) / extent.x - 1.0f;
        float ndcY = 1.0f - (2.0f * y + 1.0f) / extent.y;

        // Compute direction vector in world space
        glm::vec3 rayDirection =
            glm::normalize(direction + ndcX * right * viewportWidth + ndcY * up * viewportHeight);

        // Compute origin point of ray
        glm::vec3 rayOrigin = position;

        return Ray(rayOrigin, rayDirection);
    }

    template<typename Rng>
    [[nodiscard]] auto generateRandomRay(uint32_t x,
                           uint32_t y,
                           glm::uvec2 extent,
                           const CameraComponent& camera,
                           const exage::Transform3D& transform,
                           Rng& rng) noexcept
    {
        // Determine the coordinates of the pixel on the camera plane
        const float aspectRatio = static_cast<float>(extent.x) / static_cast<float>(extent.y);
        const float tanFovY = std::tan(glm::radians(camera.fovY) / 2.0f);
        const float tanFovX = tanFovY * aspectRatio;
        const float ndcX = (2.0f * x + 1.0f) / static_cast<float>(extent.x) - 1.0f;
        const float ndcY = 1.0f - (2.0f * y + 1.0f) / static_cast<float>(extent.y);
        const float cameraX = ndcX * tanFovX * camera.nearPlane;
        const float cameraY = ndcY * tanFovY * camera.nearPlane;

        // Convert the point into world space
        const glm::mat4 invProj = glm::inverse(glm::perspective(
            glm::radians(camera.fovY), aspectRatio, camera.nearPlane, camera.farPlane));
        const glm::vec4 cameraPos = invProj * glm::vec4(cameraX, cameraY, -camera.nearPlane, 1.0f);
        const glm::mat4 globalMatrix =
            transform.globalMatrix * glm::mat4_cast(transform.globalRotation);
        const glm::vec3 worldPos = glm::vec3(globalMatrix * cameraPos);

        // Generate a random direction for the ray
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        const float theta = 2.0f * glm::pi<float>() * dist(rng);
        const float phi = std::acos(2.0f * dist(rng) - 1.0f);
        const glm::vec3 dir = glm::vec3(
            std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi));

        return Ray(worldPos, glm::normalize(dir));
    }
}  // namespace pth
