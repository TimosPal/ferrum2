#include "ferrum/graphics/OpenGLWindowBinding.hpp"
#include "ferrum/renderer/Renderer.hpp"

namespace ferrum::renderer {

class OpenGLRenderer : public Renderer {
public:
    explicit OpenGLRenderer(graphics::OpenGLWindowBinding& binding);
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;
    OpenGLRenderer(OpenGLRenderer&&) = delete;
    OpenGLRenderer& operator=(OpenGLRenderer&&) = delete;
    ~OpenGLRenderer() override = default;

    void Clear(const graphics::Color& color) override;
    void Present() override;

private:
    graphics::OpenGLWindowBinding& binding_;
};

}  // namespace ferrum::renderer