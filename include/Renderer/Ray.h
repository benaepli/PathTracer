#pragma once

#include "glm/glm.hpp"

namespace pth
{
    class Ray
    {
      public:
      public:
        Ray() = default;
        Ray(glm::vec3 origin, glm::vec3 direction) noexcept
            : _origin(origin)
            , _direction(direction)
        {
        }

        [[nodiscard]] auto getOrigin() const noexcept -> glm::vec3 { return _origin; }
        [[nodiscard]] auto getDirection() const noexcept -> glm::vec3 { return _direction; }

        [[nodiscard]] auto getPoint(float t) const noexcept -> glm::vec3
        {
            return _origin + t * _direction;
        }

      private:
        glm::vec3 _origin {};
        glm::vec3 _direction {};
    };
}  // namespace pth
