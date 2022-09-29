#include "Buffer/IEBuffer.hpp"
#include "glm/glm.hpp"
#include "IERenderable.hpp"
#include "Shader/IEUniformBufferObject.hpp"

namespace IE::Graphics {
struct Tracker : public IEAspect {
public:
    glm::mat4                     modelMatrix{};
    IEUniformBufferObject         uniformBufferObject{};
    std::shared_ptr<IERenderable> renderable{};
    std::weak_ptr<IEAsset>        asset{};
};
}  // namespace IE::Graphics