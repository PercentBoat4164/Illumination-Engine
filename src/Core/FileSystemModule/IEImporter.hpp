#include "IEFile.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/*
 * A class used to import files into a program.
 */

class IEImporter {
public:

    // create a map to map each importer function to its datatype(s)
    typedef std::string (*importerFunction)(IEFile, unsigned int);
    std::unordered_map<std::string, importerFunction> importerMap;

    IEImporter() {
        addDefaultImporters();
    }

    static std::string import3dModel(IEFile file, unsigned int flags) {
        std::cout << file.name << " was imported as a 3d model\n";
        return "";
    }

    static std::string importText(IEFile file, unsigned int flags) {
        std::cout << "The file " <<file.name << " was imported as a TXT\n";
        return file.read();
    }

    static std::string importTexture(IEFile file, unsigned int flags) {
        std::cout << file.name << " was imported as a texture\n";
        return "";
    }

    void addDefaultImporters() {
        importerMap[".txt"] = &IEImporter::importText;
        importerMap[".obj"] = &IEImporter::import3dModel;
        importerMap[".png"] = &IEImporter::importTexture;
    }

    std::string import(IEFile file, unsigned int flags) {
        return importerMap[file.extension](file, flags);
    }
};