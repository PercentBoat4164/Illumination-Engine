#include "RenderEngine.hpp"
#include "GameObject.hpp"
//#include "Physics.hpp"
#include "Settings.hpp"

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto* RenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {glfwSetWindowShouldClose(window, 1);}
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        RenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        RenderEngine->updateSettings(RenderEngine->settings, true);
    }
    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        RenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
        RenderEngine->updateSettings(RenderEngine->settings, true);
    }
}

int main() {
    Settings settings{};
    std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n'p': Run Physics\n";
    char input = 'v';
    std::cin >> input;
    if (input == 'v') {
        try {
            settings.validationLayers = {"VK_LAYER_KHRONOS_validation"};
            VulkanRenderEngine RenderEngine(settings);
            glfwSetKeyCallback(RenderEngine.window, keyCallback);
            settings.findMaxSettings(RenderEngine.physicalDevice);
            settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
            settings.resolution = {1280, 720};
            settings.fullscreen = false;
            RenderEngine.updateSettings(settings, true);
            while (RenderEngine.update() != 1) {
                glfwPollEvents();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else if (input == 'o') {
        try {
            OpenGLRenderEngine RenderEngine(settings);
            glfwSetKeyCallback(RenderEngine.window, keyCallback);
            while (RenderEngine.update() != 1) {
                glfwPollEvents();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
//    else if (input == 'p') {
//        SphereBody myBody = SphereBody(0.0f, 0.0f, 0.0f, 1.0f,2.0f);
//        SphereBody otherBody = SphereBody(5.0f, 0.0f, 0.0f, 1.0f,2.0f);
//        otherBody.applyImpulse(0,1,0);
//        World myWorld;
//        myWorld.addBody(&myBody);
//        myWorld.addBody(&otherBody);
//        do {
//            std::cin >> input;
//            switch(input) {
//                case 'w':
//                    myBody.applyImpulse(0,1,0);
//                    break;
//                case 'a':
//                    myBody.applyImpulse(-1,0,0);
//                    break;
//                case 's':
//                    myBody.applyImpulse(0,-1,0);
//                    break;
//                case 'd':
//                    myBody.applyImpulse(1,0,0);
//                    break;
//                case 'z':
//                    myBody.applyImpulse(0,0,1);
//                    break;
//                case 'x':
//                    myBody.applyImpulse(0,0,-1);
//                    break;
//                default:
//                    myWorld.step();
//                    break;
//            }
//            std::cout << "--Sphere 1--";
//            std::cout << "\nVx: " << myBody.v.x << "  Vy: " << myBody.v.y << "  Vz: " << myBody.v.z;
//            std::cout << "\nPos x: " << myBody.pos.x << " Pos y: " << myBody.pos.y << " Pos z: " << myBody.pos.z;
//            std::cout << "\n--Sphere 2--";
//            std::cout << "\nVx: " << otherBody.v.x << "  Vy: " << otherBody.v.y << "  Vz: " << otherBody.v.z;
//            std::cout << "\nPos x: " << otherBody.pos.x << " Pos y: " << otherBody.pos.y << " Pos z: " << otherBody.pos.z;
//            std::cout << "\n--Closest Points on Sphere 1's line--";
//            std::cout << "\n" << distLineLine(myBody.pos,myBody.v,otherBody.pos,otherBody.v);
//            std::cout << "\nX: " << (getClosestPoints(myBody.pos,myBody.v,otherBody.pos,otherBody.v))[0].x;
//            std::cout << "  Y: " << (getClosestPoints(myBody.pos,myBody.v,otherBody.pos,otherBody.v))[0].y;
//            std::cout << "  Z: " << (getClosestPoints(myBody.pos,myBody.v,otherBody.pos,otherBody.v))[0].z;
//            if(World::checkCollision(myBody, otherBody)) {
//                std::cout << "\nIntersecting";
//            } else {
//                std::cout << "\nNot intersecting";
//            }
//            std::cout << "\n";
//        } while(input != 'e');
//        return 0;
//    }
}