#pragma once

#include <memory>

class IERenderEngine;

/**
 * @class IEWindowUser
 * @brief Used to store pointers to various parts of the engine that need to be accessed through the window.
 */
class IEWindowUser {
public:
	std::weak_ptr<IERenderEngine> renderEngine;
	void *IEKeyboard;
};