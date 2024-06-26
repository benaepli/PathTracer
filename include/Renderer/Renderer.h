#pragma once

#include <optional>
#include <random>
#include <span>

#include "Renderer/HitRecord.h"
#include "Renderer/Ray.h"
#include "Scene/Sphere.h"
#include "exage/Scene/Scene.h"

namespace pth
{
    constexpr double EPSILON = 0.0001;

    class Renderer
    {
      public:
        explicit Renderer(glm::uvec2 extent) noexcept;
        ~Renderer() = default;

        void render(exage::Scene& scene) noexcept;
        void resize(glm::uvec2 extent) noexcept;

        void setCameraEntity(exage::Entity cameraEntity) noexcept { _cameraEntity = cameraEntity; }

        [[nodiscard]] auto getSampleCount() const noexcept -> size_t { return _sampleCount; }
        [[nodiscard]] auto getPixels() noexcept -> std::span<uint8_t> { return _pixels; }

        void clear() noexcept
        {
			_sampleCount = 0;
			_image.clear();
			_image.resize(_extent.x * _extent.y, glm::vec4(0.0F));
		}

        [[ndoiscard]] auto getCameraEntity() const noexcept -> exage::Entity { return _cameraEntity; }

      private:
        [[nodiscard]] auto traceRay(const pth::Ray& ray, exage::Scene& scene, uint32_t depth) noexcept
            -> glm::vec3;

        auto hitScene(const pth::Ray& ray, exage::Scene& scene) noexcept
            -> std::optional<HitRecord>;

        auto hitSphere(const pth::Ray& ray,
                       exage::Entity entity,
                       glm::vec3 center,
                       float radius,
                       float maxDistance) noexcept -> std::optional<HitRecord>;

        glm::uvec2 _extent;
        uint32_t maxDepth = 25;

        std::vector<uint8_t> _pixels;
        std::vector<glm::vec4> _image;

        exage::Entity _cameraEntity;

        size_t _sampleCount = 0;

        std::mt19937 _randomEngine;
    };
}  // namespace pth
