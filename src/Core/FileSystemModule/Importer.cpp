//#include "Importer.hpp"
//
//#include "File.hpp"
//
//#include <assimp/postprocess.h>
//#include <assimp/scene.h>
//
//void IE::Core::Importer::import(const aiScene **scene, IE::Core::File &file, unsigned int flags = 0) {
//    *scene = importer.ReadFile(file.path.string().c_str(), flags);
//    if (!(*scene) || (*scene)->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !(*scene)->mRootNode)
//        throw std::runtime_error("failed to prepare scene from file: " + file.path.string());
//}
//
//void IE::Core::Importer::import(std::string *string, IE::Core::File &file, unsigned int flags = 0) {
//    string->assign(file.read().data(), file.size);
//}
