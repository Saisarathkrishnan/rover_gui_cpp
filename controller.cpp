#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>

#include <iostream>
const char* button_names[GLFW_GAMEPAD_BUTTON_LAST + 1] = {
    "A", "B", "X", "Y",
    "LB", "RB",
    "BACK", "START", "GUIDE",
    "L_STICK", "R_STICK",
    "DPAD_UP", "DPAD_RIGHT",
    "DPAD_DOWN", "DPAD_LEFT" };
enum butts
{
    A,
    B,
    X,
    Y,
    LB,
    RB,
    BACK,
    START,
    GUIDE,
    L_STICK,
    R_STICK,
    DPAD_UP,
    DPAD_RIGHT,
    DPAD_DOWN,
    DPAD_LEFT

};
struct vec2
{
    float x;
    float y;
};

class joystick_handler
{
public:
    joystick_handler()
    {
        std::cout << "hi" << std::endl;
        while (!glfwJoystickPresent(GLFW_JOYSTICK_1))
        {
            glfwPollEvents();
        }
        const char* guid = glfwGetJoystickGUID(GLFW_JOYSTICK_1);
        const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
        printf("Controlelr connected:\n");
        printf("  Name : %s\n", name);
        printf("  GUID : %s\n", guid);
        char mapping[1024];
        snprintf(mapping, sizeof(mapping),
            "%s,"
            "Xbox Series X Controller,"
            "a:b0,b:b1,x:b2,y:b3,"
            "back:b6,start:b7,guide:b8,"
            "leftshoulder:b4,rightshoulder:b5,"
            "leftstick:b9,rightstick:b10,"
            "dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,"
            "leftx:a0,lefty:a1,"
            "rightx:a3,righty:a4,"
            "lefttrigger:a2,righttrigger:a5,",
            guid);

        glfwUpdateGamepadMappings(mapping);

        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &controller))
        {
            std::cout << "controller connected" << std::endl;
        }
    }

    void update()
    {

        
        glfwGetGamepadState(GLFW_JOYSTICK_1, &controller);
        if (!glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
        {
            return;
        }

        for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++)
        {

            if (controller.buttons[i] == GLFW_PRESS)
            {
                printf("Button pressed: %s\n", button_names[i]);
            }

        }
        printf(
            "LX:% .2f LY:% .2f | RX:% .2f RY:% .2f | LT:%.2f RT:%.2f\r",
            controller.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            controller.axes[GLFW_GAMEPAD_AXIS_LEFT_Y],
            controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y],
            controller.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER],
            controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);

        fflush(stdout);
    }

private:
    GLFWgamepadstate controller;

private:
    bool x;
    bool y;
    bool a;
    bool b;
    bool d_p_up;
    bool d_p_down;
    bool d_p_left;
    bool d_p_right;
    bool l_b;
    bool r_b;
    bool leftStickBut;
    bool rightStickBut;

    float j_x1;
    float j_y1;
    float j_x2;
    float j_y2;
};

int main()
{
    if (!glfwInit())
    {
        printf("GLFW init failed\n");
        return -1;
    }
    joystick_handler joyBoy;

    while (1)
    {
        glfwPollEvents();
        joyBoy.update();

    }

    glfwTerminate();
    return 0;
}