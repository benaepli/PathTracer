#pragma once

#include "exage/Core/Window.h"
#include "exage/Scene/Scene.h"
#include "exage/Graphics/Swapchain.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Graphics/HLPD/ImGuiTools.h"
#include "exage/Graphics/Utils/QueueCommand.h"
#include "Renderer/Renderer.h"

namespace pth
{
    class Application
    {
      public:
        Application() noexcept;
        ~Application();

        void run() noexcept;

      private:
        void loadScene() noexcept;

        void drawGUI(float deltaTime) noexcept;

        void cameraInput(float deltaTime) noexcept;

        void resize(glm::uvec2 extent) noexcept;

        std::unique_ptr<exage::Window> _window;
        std::unique_ptr<exage::Graphics::Context> _context;

        std::unique_ptr<exage::Graphics::Swapchain> _swapchain;
        std::shared_ptr<exage::Graphics::FrameBuffer> _frameBuffer;

        std::optional<exage::Graphics::QueueCommandRepo> _queueCommandRepo;
        std::optional<exage::Graphics::ImGuiInstance> _imgui;

        /* CPU/Ray Traced Image */
        std::optional<exage::Graphics::DynamicFixedBuffer> _imageBuffer;
        std::shared_ptr<exage::Graphics::Texture> _imageTexture;
        glm::uvec2 _imageExtent = {1000, 500};
 
        exage::Scene _scene;
        Renderer _renderer;

        bool _render = false;
    };
}  // namespace pth
