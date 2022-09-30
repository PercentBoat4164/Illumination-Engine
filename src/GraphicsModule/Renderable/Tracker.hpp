#include "Buffer/IEBuffer.hpp"
#include "Core/AssetModule/IEAspect.hpp"
#include "glm/glm.hpp"
#include "Shader/IEUniformBufferObject.hpp"

class IERenderable;

namespace IE::Graphics {
struct Tracker : public IEAspect {
public:
    glm::mat4                     modelMatrix{};
    std::shared_ptr<IERenderable> renderable{};
    std::weak_ptr<IEAsset>        asset{};
};
}  // namespace IE::Graphics