#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <regex>
#include <iostream>

class IEFile {
public:
	IEFile() = default;

	explicit IEFile(const std::string &initialPath);

	std::streamsize length{};
	std::string path{};
	std::fstream file{};

	std::string getDirectory() const;

	void createDirectory() const;

	bool open();

	void close();

	virtual std::string read(size_t numBytes, std::streamsize startPosition = -1);

	void insert(const std::string &data, std::streamsize startPosition = -1);

	void overwrite(const std::string &data, std::streamsize startPosition = -1);

	void erase(std::streamsize numBytes, std::streamsize startPosition = -1);

	std::vector<std::string> extensions() const;

	~IEFile();

	IEFile &operator=(const IEFile &other);
};