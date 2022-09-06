#pragma once

#include "Core/AssetModule/IEAspect.hpp"

#include <sol/sol.hpp>
#include <filesystem>
#include <unordered_map>

class IEScript : public IEAspect{
private:
	const static std::unordered_map<std::string, std::string> extensionsToLanguage;

	sol::state

public:
	explicit IEScript(const std::filesystem::path &scriptPath) {

	}

	void compile() {

	}
};
