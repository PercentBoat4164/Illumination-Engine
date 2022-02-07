#pragma once

class IERenderEngine;

class IEKeyboard;

class IEJoyStick;

class IEMouse;

class IETouchScreen;

struct IEWindowUserPointer {
    void* keyboard;
    void* renderEngine;
};