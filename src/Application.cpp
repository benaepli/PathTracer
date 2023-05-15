#include "Application.h"

#include "Scene/Camera.h"

namespace pth
{
    constexpr auto windowAPI = exage::WindowAPI::eGLFW;
    constexpr auto graphicsAPI = exage::Graphics::API::eVulkan;

    Application::Application() noexcept
        : _renderer(_imageExtent)
    {
        exage::init();

        exage::Monitor monitor = exage::getDefaultMonitor(windowAPI);
        exage::WindowInfo windowInfo {
            .name = "PathTracer",
            .extent = {monitor.extent.x / 2, monitor.extent.y / 2},
            .fullScreen = false,
            .windowBordered = true,
            .exclusiveRefreshRate = monitor.refreshRate,
            .exclusiveMonitor = monitor,
        };

        tl::expected windowResult = exage::Window::create(windowInfo, windowAPI);
        assert(windowResult.has_value());
        _window = std::move(*windowResult);

        exage::ResizeCallback resizeCallback {};
        resizeCallback.data = this;
        resizeCallback.callback = [](void* data, glm::uvec2 extent)
        { static_cast<Application*>(data)->resize(extent); };
        _window->addResizeCallback(resizeCallback);

        exage::Graphics::ContextCreateInfo contextInfo {
            .api = graphicsAPI,
            .windowAPI = windowAPI,
            .optionalWindow = _window.get(),
            .maxFramesInFlight = 2,
        };
        contextInfo.optionalWindow = _window.get();
        contextInfo.api = graphicsAPI;

        tl::expected contextResult = exage::Graphics::Context::create(contextInfo);
        assert(contextResult.has_value());
        _context = std::move(*contextResult);

        exage::Graphics::SwapchainCreateInfo swapchainInfo {
            .window = *_window, .presentMode = exage::Graphics::PresentMode::eTripleBufferVSync};
        _swapchain = _context->createSwapchain(swapchainInfo);

        exage::Graphics::TextureCreateInfo textureInfo {
            .extent = {_window->getExtent(), 1},
            .format = exage::Graphics::Texture::Format::eRGBA8,
            .usage = exage::Graphics::Texture::UsageFlags::eColorAttachment
                | exage::Graphics::Texture::UsageFlags::eTransferSource,
        };
        std::shared_ptr<exage::Graphics::Texture> renderTexture =
            _context->createTexture(textureInfo);

        _frameBuffer = _context->createFrameBuffer(_window->getExtent());
        _frameBuffer->attachColor(renderTexture);

        exage::Graphics::QueueCommandRepoCreateInfo queueCommandRepoInfo {.context = *_context};
        _queueCommandRepo = exage::Graphics::QueueCommandRepo {queueCommandRepoInfo};

        exage::Graphics::ImGuiInitInfo imguiInfo {
            .context = *_context,
            .window = *_window,
        };
        _imgui = exage::Graphics::ImGuiInstance {imguiInfo};

        exage::Graphics::DynamicFixedBufferCreateInfo imageBufferInfo {
            .context = *_context,
            .size = _imageExtent.x * _imageExtent.y * 4,
            .cached = false,
            .useStagingBuffer = true,
        };
        _imageBuffer = exage::Graphics::DynamicFixedBuffer {imageBufferInfo};

        exage::Graphics::TextureCreateInfo imageTextureInfo {
            .extent = {_imageExtent, 1},
            .format = exage::Graphics::Texture::Format::eRGBA8,
            .usage = exage::Graphics::Texture::UsageFlags::eTransferDestination,
        };
        _imageTexture = _context->createTexture(imageTextureInfo);

        loadScene();
    }

    Application::~Application()
    {
        _context->waitIdle();
    }

    void Application::run() noexcept
    {
        // time in seconds
        std::chrono::high_resolution_clock::time_point lastTime =
            std::chrono::high_resolution_clock::now();

        while (!_window->shouldClose())
        {
            _window->update();

            if (_window->isMinimized())
            {
                continue;
            }

            std::chrono::high_resolution_clock::time_point currentTime =
                std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTimeClock = currentTime - lastTime;
            float deltaTime = deltaTimeClock.count();
            lastTime = currentTime;

            _scene.updateHierarchy(true);

            if (_render)
            {
                _renderer.render(_scene);
            }

            exage::Graphics::CommandBuffer& cmd = _queueCommandRepo->current();
            _context->getQueue().startNextFrame();

            std::optional swapError = _swapchain->acquireNextImage();
            if (swapError.has_value())
            {
                continue;
            }

            std::shared_ptr<exage::Graphics::Texture> const texture = _frameBuffer->getTexture(0);
            cmd.begin();

            std::span<uint8_t> pixels = _renderer.getPixels();
            std::span<std::byte> pixelsBytes = {reinterpret_cast<std::byte*>(pixels.data()),
                                                pixels.size()};
            _imageBuffer->write(pixelsBytes, 0);
            _imageBuffer->update(cmd);

            cmd.bufferBarrier(_imageBuffer->currentHost(),
                              exage::Graphics::PipelineStageFlags::eTransfer,
                              exage::Graphics::PipelineStageFlags::eTransfer,
                              exage::Graphics::AccessFlags::eTransferWrite,
                              exage::Graphics::AccessFlags::eTransferRead);

            cmd.textureBarrier(_imageTexture,
                               exage::Graphics::Texture::Layout::eTransferDst,  // new layout
                               exage::Graphics::PipelineStageFlags::eTransfer,
                               exage::Graphics::PipelineStageFlags::eTransfer,
                               exage::Graphics::AccessFlags::eTransferWrite,
                               exage::Graphics::AccessFlags::eTransferRead);

            cmd.copyBufferToTexture(_imageBuffer->currentHost(),
                                    _imageTexture,
                                    /*srcOffset*/ 0,
                                    /*dstOffset*/ glm::uvec3 {0, 0, 0},
                                    /*dstMipLevel*/ 0,
                                    /*dstFirstLayer*/ 0,
                                    /*dstNumLayers*/ 1,
                                    /*extent*/ {_imageExtent, 1});

            cmd.textureBarrier(_imageTexture,
                               exage::Graphics::Texture::Layout::eShaderReadOnly,
                               exage::Graphics::PipelineStageFlags::eTransfer,
                               exage::Graphics::PipelineStageFlags::eFragmentShader,
                               exage::Graphics::AccessFlags::eTransferWrite,
                               exage::Graphics::AccessFlags::eShaderRead);

            cmd.textureBarrier(texture,
                               exage::Graphics::Texture::Layout::eColorAttachment,
                               exage::Graphics::PipelineStageFlags::eTransfer,
                               exage::Graphics::PipelineStageFlags::eColorAttachmentOutput,
                               exage::Graphics::AccessFlags::eTransferWrite,
                               exage::Graphics::AccessFlags::eColorAttachmentWrite);

            exage::Graphics::ClearColor const clearColor {.clear = true, .color = {}};
            exage::Graphics::ClearDepthStencil const clearDepthStencil {.clear = false};
            cmd.beginRendering(_frameBuffer, {clearColor}, clearDepthStencil);

            _imgui->begin();

            drawGUI(deltaTime);

            _imgui->end();

            _imgui->renderMainWindow(cmd);

            cmd.endRendering();

            cmd.textureBarrier(texture,
                               exage::Graphics::Texture::Layout::eTransferSrc,
                               exage::Graphics::PipelineStageFlags::eTopOfPipe,
                               exage::Graphics::PipelineStageFlags::eTransfer,
                               {},
                               exage::Graphics::AccessFlags::eTransferWrite);

            _swapchain->drawImage(cmd, texture);

            cmd.end();

            exage::Graphics::QueueSubmitInfo submitInfo {.commandBuffer = cmd};
            _context->getQueue().submit(submitInfo);

            _imgui->renderAdditional();

            exage::Graphics::QueuePresentInfo presentInfo {.swapchain = *_swapchain};
            swapError = _context->getQueue().present(presentInfo);
        }
    }

    void Application::loadScene() noexcept
    {
        Material materialGround {
            .color = {0.8, 0.8, 0.0},
            .metallic = 0.F,
            .roughness = 1.F,
            .ior = 1.5F,
            .emissive = {0.0, 0.0, 0.0},
        };

        Material materialCenter {
            .color = {0.7, 0.3, 0.3},
            .metallic = 0.F,
            .roughness = 1.F,
            .ior = 1.5F,
            .emissive = {0.0, 0.0, 0.0},
        };

        Material materialLeft {
            .color = {0.8, 0.8, 0.8},
            .metallic = 0.3F,
            .roughness = 0.F,
            .ior = 3.F,
            .emissive = {0.0, 0.0, 0.0},
        };

        Material materialRight {
            .color = {0.8, 0.6, 0.2},
            .metallic = 0.1F,
            .roughness = 0.F,
            .ior = 1.5F,
            .emissive = {0.0, 0.0, 0.0},
        };

        using exage::Entity;
        {
            Entity groundEntity = _scene.createEntity();
            auto& transform = _scene.addComponent<exage::Transform3D>(groundEntity);
            transform.position = {0, -100.5F, -1};
            auto& sphere = _scene.addComponent<Sphere>(groundEntity);
            sphere.radius = 100.F;
            sphere.material = materialGround;
        }
        {
            Entity centerEntity = _scene.createEntity();
            auto& transform = _scene.addComponent<exage::Transform3D>(centerEntity);
            transform.position = {0, 0, -1};
            auto& sphere = _scene.addComponent<Sphere>(centerEntity);
            sphere.radius = 0.5F;
            sphere.material = materialCenter;
        }
        {
            Entity leftEntity = _scene.createEntity();
            auto& transform = _scene.addComponent<exage::Transform3D>(leftEntity);
            transform.position = {-1, 0, -1};
            auto& sphere = _scene.addComponent<Sphere>(leftEntity);
            sphere.radius = 0.5F;
            sphere.material = materialLeft;
        }
        {
            Entity rightEntity = _scene.createEntity();
            auto& transform = _scene.addComponent<exage::Transform3D>(rightEntity);
            transform.position = {1, 0, -1};
            auto& sphere = _scene.addComponent<Sphere>(rightEntity);
            sphere.radius = 0.5F;
            sphere.material = materialRight;
        }
        {
            Entity camera = _scene.createEntity();
            auto& transform = _scene.addComponent<exage::Transform3D>(camera);
            transform.position = {0, 0, 0};
            transform.rotation = glm::vec3 {0, 0, 0};
            auto& cameraComponent = _scene.addComponent<CameraComponent>(camera);
            _renderer.setCameraEntity(camera);
        }
    }

    void Application::drawGUI(float deltaTime) noexcept
    {
        bool open = true;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus;

        static ImGuiDockNodeFlags const dockspaceFlags = ImGuiDockNodeFlags_None;

        if ((dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0)
        {
            windowFlags |= ImGuiWindowFlags_NoBackground;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiIO const& io = ImGui::GetIO();
        ImGuiStyle const& style = ImGui::GetStyle();

        ImGui::Begin("DockSpace", &open, windowFlags);

        if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0)
        {
            ImGuiID const dockspaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), dockspaceFlags);
        }

        bool showTextureWindow = true;
        ImVec2 const textureWindowSize = {static_cast<float>(_imageExtent.x),
                                          static_cast<float>(_imageExtent.y)};
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowContentSize(textureWindowSize);
        ImGui::Begin("Texture", &showTextureWindow, ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::IsWindowHovered())
        {
            cameraInput(deltaTime);
        }

        ImGui::Image(_imageTexture.get(), textureWindowSize);
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Render Settings");
        if (ImGui::Button("Toggle Render"))
        {
            _render = !_render;
        }
        ImGui::End();

        ImGui::End();
    }

    void Application::cameraInput(float deltaTime) noexcept
    {
        glm::vec3 movement {0.F};

        if (ImGui::IsKeyDown(ImGuiKey_W))
        {
            movement.z -= 1.F;
        }

        if (ImGui::IsKeyDown(ImGuiKey_S))
        {
            movement.z += 1.F;
        }
        if (ImGui::IsKeyDown(ImGuiKey_A))
        {
            movement.x -= 1.F;
        }
        if (ImGui::IsKeyDown(ImGuiKey_D))
        {
            movement.x += 1.F;
        }
        if (ImGui::IsKeyDown(ImGuiKey_E))
        {
            movement.y += 1.F;
        }
        if (ImGui::IsKeyDown(ImGuiKey_Q))
        {
            movement.y -= 1.F;
        }

        if (movement != glm::vec3 {0.F})
        {
            movement = glm::normalize(movement);
            movement *= deltaTime;
            auto& camera = _scene.getComponent<CameraComponent>(_renderer.getCameraEntity());
            auto& transform = _scene.getComponent<exage::Transform3D>(_renderer.getCameraEntity());

            glm::vec3 currentRotation = *std::get_if<glm::vec3>(&transform.rotation);

            glm::vec3 right = glm::normalize(glm::vec3 {glm::cos(currentRotation.y),
                                                          glm::sin(currentRotation.x),
                                                          glm::sin(currentRotation.y)});

            glm::vec3 forward = glm::normalize(glm::cross(right, glm::vec3 {0.F, 1.F, 0.F}));

            glm::vec3 up = glm::normalize(glm::cross(right, forward));

            transform.position += forward * movement.z;

            transform.position += right * movement.x;

            transform.position -= up * movement.y;

            _renderer.clear();
        }
    }

    void Application::resize(glm::uvec2 extent) noexcept
    {
        _swapchain->resize(extent);
        _frameBuffer->resize(extent);
    }
}  // namespace pth
