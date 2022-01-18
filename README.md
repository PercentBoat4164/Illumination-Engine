# Graphics Module
The Graphics Module is one of the modules that is designed to attach to the Illumination Engine core.

## Requirements for Completion
- [x] First working prototype
- [x] Vulkan implementation
- [x] OpenGL implementation
- [x] Add basic shading
- [ ] Merged implementation
  - [ ] Rewrite Vulkan render engine implementation ~33%
  - [ ] Integrate bare-bones OpenGL implementation
  - [ ] Remove Vulkan SDK requirement
  - [ ] Finish OpenGL integration
- [ ] Cleanup
  - [ ] Bring the code up to stylistic standards
  - [ ] Resolve any key or important TODOs
  - [ ] *Optional* Rewrite abstracted OpenGL and Vulkan layers

### Current Actions Being Taken:
I am currently working on the Merged Implementation requirement. I have broken it into a few sub-steps the first of which is to rewrite the Vulkan engine. It was sloppily written the first time, and it needs to be neater for the next step which is OpenGL integration. After rewriting the Vulkan engine I will integrate the OpenGL engine into the Vulkan engine so that one can be chosen to render with. After basic integration is implemented (defined by a triangle on-screen being rendered with OpenGL) I will remove Vulkan as a hard dependency for Vulkan code. This will allow for an easier time with programming as well as allow for computers that do not support Vulkan to enable computers that do to use Vulkan only features in games that are created on it. Finally, I will add support for renderables, textures, shaders, and all the other fancy stuff to OpenGL. That will finish off the Merged Implementation.

From that point, the graphics module should be on version 1.0.0. The next few updates after that will bring the patch or minor version up, but it should not be anything major as it is only cleanup.