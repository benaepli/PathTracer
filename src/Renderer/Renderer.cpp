#include "Renderer/Renderer.h"

#include "Renderer/Ray.h"
#include "Scene/Camera.h"

namespace pth
{
    Renderer::Renderer(glm::uvec2 extent) noexcept
        : _extent(extent)
    {
        // default construct pixels to 0
        _pixels = std::vector<uint8_t>(extent.x * extent.y * 4, 255);
        _image = std::vector<glm::vec4>(extent.x * extent.y, glm::vec4(0.0F));
    }

    void Renderer::render(exage::Scene& scene) noexcept
    {
        if (!scene.hasComponent<CameraComponent>(_cameraEntity))
        {
            return;
        }

        auto& camera = scene.getComponent<CameraComponent>(_cameraEntity);
        auto& transform = scene.getComponent<exage::Transform3D>(_cameraEntity);

        for (uint32_t y = 0; y < _extent.y; ++y)
        {
            uint32_t yValue = _extent.y - y - 1;

            for (uint32_t x = 0; x < _extent.x; ++x)
            {
                Ray ray = computeCameraRay(
                    _extent, x, y, camera.fovY, transform.globalPosition, transform.globalRotation);

                glm::vec3 color = traceRay(ray, scene, 0);
                // Accumulate color
                _image[y * _extent.x + x] += glm::vec4(color, 1.0F);

                // Average color
                color = _image[y * _extent.x + x] / static_cast<float>(_sampleCount + 1);

                _pixels[(y * _extent.x + x) * 4 + 0] = static_cast<uint8_t>(color.x * 255);
                _pixels[(y * _extent.x + x) * 4 + 1] = static_cast<uint8_t>(color.y * 255);
                _pixels[(y * _extent.x + x) * 4 + 2] = static_cast<uint8_t>(color.z * 255);
                _pixels[(y * _extent.x + x) * 4 + 3] = 255;
            }
        }

        ++_sampleCount;
    }

    void Renderer::resize(glm::uvec2 extent) noexcept
    {
        _extent = extent;

        _pixels.resize(extent.x * extent.y * 4);
        _image.resize(extent.x * extent.y);

        _sampleCount = 0;

        std::fill(_pixels.begin(), _pixels.end(), 0);
        std::fill(_image.begin(), _image.end(), glm::vec4(0.0F));
    }

    auto Renderer::traceRay(const pth::Ray& ray, exage::Scene& scene, uint32_t depth) noexcept
        -> glm::vec3
    {
        if (depth > maxDepth)
        {
            return glm::vec3(0.0F);
        }

        auto hitRecord = hitScene(ray, scene);
        if (!hitRecord.has_value())
        {
            glm::vec3 skyColor = glm::vec3(0.5F, 0.7F, 1.0F);
            glm::vec3 unitDirection = glm::normalize(ray.getDirection());
            float t = 0.5F * (unitDirection.y + 1.0F);
            return (1.0F - t) * glm::vec3(1.0F, 1.0F, 1.0F) + t * skyColor;
        }

        Material& material = hitRecord->material;

        // indirect lighting
        Ray rayResult = sampleRay(ray, hitRecord.value(), _randomEngine);
        glm::vec3 incomingLight = traceRay(rayResult, scene, depth + 1);

        float pdf = getProbabilityDistribution(rayResult, hitRecord.value());

        float cosTheta = glm::dot(rayResult.getDirection(), hitRecord->normal);

        glm::vec3 brdf =
            calculateBRDF(rayResult.getDirection(), ray.getDirection(), hitRecord.value());

        return material.emissive + (brdf * incomingLight * cosTheta) / pdf;
    }

    auto Renderer::hitScene(const pth::Ray& ray, exage::Scene& scene) noexcept
        -> std::optional<HitRecord>
    {
        double closestDistance = std::numeric_limits<double>::max();
        std::optional<HitRecord> closestHitRecord;

        auto sphereView = scene.registry().view<Sphere, exage::Transform3D>();

        for (auto entity : sphereView)
        {
            Sphere& sphere = sphereView.get<Sphere>(entity);
            glm::vec3 center = sphereView.get<exage::Transform3D>(entity).globalPosition;
            auto hitRecord = hitSphere(ray, entity, center, sphere.radius, closestDistance);
            if (hitRecord.has_value())
            {
                hitRecord->material = sphere.material;
                closestDistance = hitRecord->distance;
                closestHitRecord = hitRecord;
            }
        }

        return closestHitRecord;
    }

    auto Renderer::hitSphere(const pth::Ray& ray,
                             exage::Entity entity,
                             glm::vec3 center,
                             float radius,
                             float maxDistance) noexcept -> std::optional<HitRecord>
    {
        glm::vec3 oc = ray.getOrigin() - center;
        float a = glm::dot(ray.getDirection(), ray.getDirection());
        float halfB = glm::dot(oc, ray.getDirection());
        float c = glm::dot(oc, oc) - radius * radius;

        float discriminant = halfB * halfB - a * c;
        if (discriminant < 0)
        {
            return std::nullopt;
        }
        float sqrtd = std::sqrt(discriminant);
        // Find the nearest root that lies in the acceptable range.
        float root = (-halfB - sqrtd) / a;
        if (root < EPSILON)
        {
            root = (-halfB + sqrtd) / a;
        }

        if (root < EPSILON || root > maxDistance)
        {
            return std::nullopt;
        }

        glm::vec3 point = ray.getPoint(root);
        glm::vec3 normal = (point - center) / radius;
        bool frontFace = glm::dot(ray.getDirection(), normal) < 0;
        if (!frontFace)
        {
            normal = -normal;
        }

        return HitRecord {point, normal, root, frontFace};
    }
}  // namespace pth
