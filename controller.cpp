#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>

const char *button_names[GLFW_GAMEPAD_BUTTON_LAST + 1] = {
    "A", "B", "X", "Y",
    "LB", "RB",
    "BACK", "START", "GUIDE",
    "L_STICK", "R_STICK",
    "DPAD_UP", "DPAD_RIGHT",
    "DPAD_DOWN", "DPAD_LEFT"};

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
    joystick_handler() : speedNo{0},
                         prev_lb_pressed{false},
                         prev_rb_pressed{false}
    {
        sendJ = {
            {"M", "0"},
            {"X", "0"},
            {"Y", "0"},
            {"P", "0"},
            {"Q", "0"},
            {"A", "0"},
            {"S", "0"},
            {"R", "0"},
            {"D", "0"},
            {"E", "0"}};
        string_initialize();
        initializeSock();

        std::cout << "hi" << std::endl;
        while (!glfwJoystickPresent(GLFW_JOYSTICK_1))
        {
            glfwPollEvents();
            std::cout << "connect controller" << std::endl;
        }
        const char *guid = glfwGetJoystickGUID(GLFW_JOYSTICK_1);
        const char *name = glfwGetJoystickName(GLFW_JOYSTICK_1);
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
                 "lefttrigger:a2,righttrigger:a5,"
                 "platform:Linux",
                 guid);

        glfwUpdateGamepadMappings(mapping);

        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &controller))
        {
            std::cout << "controller paired" << std::endl;
        }

        for (int i = 0; i < buttonsNo; i++)
        {
            buttons[i] = false;
        }
    }
    ~joystick_handler()
    {
        close(sockfd);
    }

    void string_initialize()
    {
        skillIssue = "M" + std::string("0") + "X" + "0" + "Y" + "0" + "P" + "0" + "Q" + "0" + "A" + "0" + "S" + "0" + "R" + "0" + "D" + "0" + "E" + "0";
    }
    void string_update()
    {
        skillIssue = "M" + sendJ["M"].get<std::string>() + "X" + sendJ["X"].get<std::string>() + "Y" + sendJ["Y"].get<std::string>() + "P" + sendJ["P"].get<std::string>() + "Q" + sendJ["Q"].get<std::string>() + "A" + sendJ["A"].get<std::string>() + "S" + sendJ["S"].get<std::string>() + "R" + sendJ["R"].get<std::string>() + "D" + sendJ["D"].get<std::string>() + "E" + sendJ["E"].get<std::string>();
    }
    void update()
    {
        string_initialize();
        refresh();

        glfwGetGamepadState(GLFW_JOYSTICK_1, &controller);
        if (!glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
        {
            std::cout << "connect joystick\r";
            return;
        }

        for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++)
        {
            if (controller.buttons[i] == GLFW_PRESS)
            {
                buttons[i] = true;
            }
            else
            {
                buttons[i] = false;
                if (!buttons[LB])
                    prev_lb_pressed = false;
                if (!buttons[RB])
                    prev_rb_pressed = false;
            }
        }

        if (buttons[A])
        {
            std::cout << "a pressed" << std::endl;
            sendJ["A"] = std::to_string(2);
        }
        if (buttons[B])
        {
            std::cout << "b pressed" << std::endl;
            sendJ["A"] = std::to_string(3);
        }
        if (buttons[X])
        {
            std::cout << "x pressed" << std::endl;
            sendJ["A"] = std::to_string(4);
        }
        if (buttons[Y])
        {
            std::cout << "y pressed" << std::endl;
            sendJ["A"] = std::to_string(1);
        }
        if (buttons[LB])
        {
            if (!prev_lb_pressed)
            {
                prev_lb_pressed = true;
                std::cout << "lb pressed" << std::endl;
                if (speedNo > 0)
                {
                    speedNo -= 1;
                }
            }
        }
        if (buttons[RB])
        {
            if (!prev_rb_pressed)
            {
                prev_rb_pressed = true;
                std::cout << "rb pressed" << std::endl;
                if (speedNo < 9)
                {
                    speedNo += 1;
                }
            }
        }
        if (buttons[BACK])
        {
            std::cout << "back pressed" << std::endl;
        }
        if (buttons[START])
        {
            std::cout << "start pressed" << std::endl;
        }
        if (buttons[GUIDE])
        {
            std::cout << "guide pressed" << std::endl;
        }
        if (buttons[L_STICK])
        {
            std::cout << "l_stick pressed" << std::endl;
        }
        if (buttons[R_STICK])
        {
            std::cout << "r_stick pressed" << std::endl;
        }
        if (buttons[DPAD_UP])
        {
            std::cout << "d_up pressed" << std::endl;
            sendJ["A"] = std::to_string(6);
        }
        if (buttons[DPAD_RIGHT])
        {
            std::cout << "d_right pressed" << std::endl;
            // sendJ["A"]=70;
        }
        if (buttons[DPAD_DOWN])
        {
            std::cout << "d_down pressed" << std::endl;
            sendJ["A"] = std::to_string(5);
        }
        if (buttons[DPAD_LEFT])
        {
            std::cout << "d_l pressed" << std::endl;
            sendJ["R"] = std::to_string(1);
        }

        sendJ["M"] = std::to_string(speedNo);
        sendJ["X"] = std::to_string(int(controller.axes[GLFW_GAMEPAD_AXIS_LEFT_X] * 1123));
        sendJ["Y"] = std::to_string(int(controller.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] * 1023));
        sendJ["Q"] = std::to_string(int(controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] * 11));
        sendJ["P"] = std::to_string(int(controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * 11));

        if (controller.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > -1)
        {
            sendJ["S"] = std::to_string(-int((controller.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1) * 5));
        }
        else if (controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > -1)
        {
            sendJ["S"] = std::to_string(int((controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1) * 5));
        }

        printf(
            "LX:%d LY:%d | RX:%d RY:%d | LT:%d RT:%d\n",
            int(controller.axes[GLFW_GAMEPAD_AXIS_LEFT_X] * 1123),
            int(controller.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] * 1023),
            int(controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] * 11),
            int(controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * 11),
            int(controller.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]),
            int(controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]));

        fflush(stdout);

        string_update();
        std::cout << skillIssue << "\n";
        sendSock();
    }

    void initializeSock()
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("Socket creation failed");
            return;
        }

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(5005);

        if (inet_pton(AF_INET, "10.0.0.7", &serverAddr.sin_addr) <= 0)
        {
            perror("Invalid address");
            close(sockfd);
            return;
        }

        std::cout << "UDP socket initialized\n";
        sleep(1);
    }

    void sendSock()
    {
        const char *msg = skillIssue.c_str();

        sendto(sockfd,
               msg,
               strlen(msg),
               0,
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr));
    }

    void refresh()
    {
        sendJ = {
            {"M", "0"},
            {"X", "0"},
            {"Y", "0"},
            {"P", "0"},
            {"Q", "0"},
            {"A", "0"},
            {"S", "0"},
            {"R", "0"},
            {"D", "0"},
            {"E", "0"}};
    }

private:
    GLFWgamepadstate controller;
    int sockfd;
    struct sockaddr_in serverAddr;

private:
    const int buttonsNo = 15;
    bool buttons[15];
    nlohmann::json sendJ;
    int speedNo;
    std::string skillIssue;

    bool prev_lb_pressed;
    bool prev_rb_pressed;
};

#include <thread>
#include <chrono>

int main()
{
    if (!glfwInit())
        return -1;

    joystick_handler joyBoy;

    const double FPS = 60.0;
    const double frameTime = 1.0 / FPS;
    double lastTime = glfwGetTime();

    double fpsTimer = lastTime;
    int frameCount = 0;

    while (1)
    {
        double now = glfwGetTime();
        double delta = now - lastTime;

        if (delta < frameTime)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(frameTime - delta)
            );
            continue;
        }

        lastTime = now;

        glfwPollEvents();
        joyBoy.update();

        frameCount++;

        if (now - fpsTimer >= 1.0)
        {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            fpsTimer = now;
        }
    }

    glfwTerminate();
    return 0;
}
