#pragma once

/**
 * @class IEWindowUser
 * @brief Used to store pointers to various parts of the engine that need to be accessed through the window.
 */
class IEWindowUser {
public:
    void *IERenderEngine;
    void *IEKeyboard;
};