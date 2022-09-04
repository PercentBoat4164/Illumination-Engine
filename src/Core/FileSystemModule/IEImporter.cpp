#include "IEImporter.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

void IEImporter::import(const aiScene **scene, IEFile &file, unsigned int flags=0) {
	*scene = importer.ReadFile(file.path.string().c_str(), flags);
	if (!(*scene) || (*scene)->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !(*scene)->mRootNode) {
		throw std::runtime_error("failed to prepare scene from file: " + file.path.string());
	}
}

void IEImporter::import(std::string *string, IEFile &file, unsigned int flags=0) {
	string->assign(file.read().data(), file.size);
}
