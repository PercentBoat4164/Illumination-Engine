#pragma once

#include "IEFile.hpp"
#include "stb_image.h"

#include <assimp/Importer.hpp>

class IETexture;

/*
 * A class used to import files into a program. One per filesystem.
 */
class IEImporter {
public:
	Assimp::Importer importer{};

	void import(const aiScene **, IEFile &, uint32_t = 0);
	
	static void import(std::string *, IEFile &, uint32_t);
	
	/**@todo Find a way around needing to define each variant of this function for user defined or IE types.*/
	friend void import(IETexture *, IEFile &, uint32_t);  // Needs to be implemented in IETexture.
};