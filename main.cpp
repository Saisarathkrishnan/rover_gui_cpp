#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <iostream>
#include <stdio.h>
#include <string>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>

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

        // std::cout << "hi" << std::endl;
        while (!glfwJoystickPresent(GLFW_JOYSTICK_1))
        {
            glfwPollEvents();
            // std::cout << "connect controller" << std::endl;
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
            sendJ["A"] = std::to_string(5);
        }
        if (buttons[DPAD_RIGHT])
        {
            std::cout << "d_right pressed" << std::endl;
            // sendJ["A"]=70;
        }
        if (buttons[DPAD_DOWN])
        {
            std::cout << "d_down pressed" << std::endl;
            sendJ["A"] = std::to_string(6);
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
        sendJ["P"] = std::to_string(-int(controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * 11));

        if (controller.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > -1)
        {
            sendJ["S"] = std::to_string(-int((controller.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1) * 5));
        }
        else if (controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > -1)
        {
            sendJ["S"] = std::to_string(int((controller.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1) * 5));
        }

        fflush(stdout);

        string_update();
        // std::cout << skillIssue << "\n";
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

        std::cout << "UDP socket initialized for rover controller\n";
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

GLuint matToTexture(const cv::Mat &mat, GLuint tex_id)
{
    if (tex_id == 0)
    {
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            mat.cols,
            mat.rows,
            0,
            GL_BGR,
            GL_UNSIGNED_BYTE,
            nullptr);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, tex_id);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        mat.cols,
        mat.rows,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        mat.data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex_id;
}

void mystyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
}

bool isOnline(const std::string &ip)
{
    std::string cmd = "ping -c 1 -W 1 " + ip + " > /dev/null 2>&1";
    int ret = system(cmd.c_str());
    return ret == 0;
}

nlohmann::json check_online()
{
    std::cout << "L2U: " << (isOnline("10.0.0.7") ? "online" : "offline") << std::endl;
    std::cout << "Jetson: " << (isOnline("10.0.0.69") ? "online" : "offline") << std::endl;
    std::cout << "Analog: " << (isOnline("10.0.0.9") ? "online" : "offline") << std::endl;
}

void sendLL(int client_fd, nlohmann::json *data_tosend)
{
    std::cout << "send" << std::endl;
    std::cout << "sending this json-----------" << std::endl;
    for (auto &[key, value] : (*data_tosend).items())
    {
        std::cout << key << " : " << value << std::endl;
    }
    std::cout << "sendt this  json-----------" << std::endl;
    std::string mj_str = (*data_tosend).dump();
    uint32_t len = htonl(mj_str.size());
    send(client_fd, &len, sizeof(len), 0);
    send(client_fd, mj_str.data(), mj_str.size(), 0);
}
void resetLL(nlohmann::json *data_tosend)
{
    *data_tosend = {
        {"is_auto", false},
        {"navModeColourGPS", false}, // false is colour
        {"colourId", -1},
        {"to_skew", 0},
        {"skew", -1},
        {"goal_lat", "nan"},
        {"goal_lon", "nan"},
    };
    std::cout << "reset the json-----------" << std::endl;
    for (auto &[key, value] : (*data_tosend).items())
    {
        std::cout << key << " : " << value << std::endl;
    }
    std::cout << "reseted the json-----------" << std::endl;
}

void DrawUI(int client_fd,
            nlohmann::json *data_tosend,
            std::vector<cv::VideoCapture> &caps,
            std::vector<GLuint> &textures,
            const std::vector<int> &channels)
{
    nlohmann::json mj = {{"imu_x", "nan"},
                         {"imu_y", "nan"},
                         {"imu_z", "nan"},
                         {"marker_detect_bool", "nan"},
                         {"marker_detect_id", "nan"},
                         {"marker_detect_x", "nan"},
                         {"marker_detect_y", "nan"},
                         {"autostate_curr", "nan"},
                         {"gps_lat_curr", "nan"},
                         {"gps_lon_curr", "nan"},
                         {"gps_alt_curr", "nan"},
                         {"gps_vdops_curr", "nan"},
                         {"gps_hdops_curr", "nan"},
                         {"gps_fix_curr", "nan"},
                         {"gps_sattelites_curr", "nan"},
                         {"navmode", "nan"},
                         {"destYaw", "nan"},
                         {"currnYaw", "nan"},
                         {"destYawError", "nan"},
                         {"gpsGoal_condition", "nan"},
                         {"coneGoal_condition", "nan"}};

    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("MRM GUI", nullptr, flags);

    if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_Reorderable))
    {

        if (ImGui::BeginTabItem("Tab1"))
        {
            float W = ImGui::GetContentRegionAvail().x;
            float H = ImGui::GetContentRegionAvail().y - 13;

            float leftW = W * 0.2f;
            float midW = W * 0.6f;
            float rightW = W * 0.2f;
            ImGui::BeginChild("LeftGPS", ImVec2(leftW, H), true);
            ImGui::Text("Fix State");
            ImGui::Separator();
            ImGui::Text("Fix        : %s", mj["gps_fix_curr"].get<std::string>().c_str());
            ImGui::Text("Satellites : %s", mj["gps_sattelites_curr"].get<std::string>().c_str());
            ImGui::Text("HDOP       : %s", mj["gps_vdops_curr"].get<std::string>().c_str());
            ImGui::Text("VDOP       : %s", mj["gps_hdops_curr"].get<std::string>().c_str());
            ImGui::Separator();
            ImGui::Text("Current Position");
            ImGui::Text("Lat : %s", mj["gps_lat_curr"].get<std::string>().c_str());
            ImGui::Text("Lon : %s", mj["gps_lon_curr"].get<std::string>().c_str());
            ImGui::Text("Alt : %s", mj["gps_alt_curr"].get<std::string>().c_str());
            ImGui::Separator();
            ImGui::Text("Destination");

            ImGui::Text("Lat : %s", (*data_tosend)["goal_lat"].get<std::string>().c_str());
            ImGui::Text("Lon : %s", (*data_tosend)["goal_lon"].get<std::string>().c_str());
            ImGui::Text("Alt : %s", mj["gps_alt_curr"].get<std::string>().c_str());
            ImGui::Text("Distance : %.2f m", 12.4f);
            ImGui::Text("Curr Yaw : %s deg", mj["currnYaw"].get<std::string>().c_str());
            ImGui::Text("Dest Yaw : %s deg", mj["destYaw"].get<std::string>().c_str());
            ImGui::Text("Error : %s deg", mj["destYawError"].get<std::string>().c_str());
            ImGui::Separator();
            ImGui::Text("Colour Marker");
            std::string fgfg;
            ImGui::Text("Marker Detectd: %s", mj["gps_lat_curr"].get<std::string>().c_str());
            ImGui::Text("Marker ID: %s", mj["marker_detect_id"].get<std::string>().c_str());
            ImGui::Text("Marker x : %s", mj["marker_detect_x"].get<std::string>().c_str());
            ImGui::Text("Marker y : %s", mj["marker_detect_y"].get<std::string>().c_str());

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Middle", ImVec2(midW, H), false);

            float topH = H * 0.45f;
            float midH = H * 0.25f;

            ImGui::BeginChild("TopRow", ImVec2(0, topH), false);

            ImGui::BeginChild("ZED", ImVec2(midW * 0.6f, topH), true);
            ImGui::Text("ZED Camera");
            ImGui::Dummy(ImVec2(0, topH - 40));
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("NavState", ImVec2(0, topH), true);
            static int control_mode = 1;
            ImGui::Text("Nav State");
            ImGui::Text("Control Mode: %s", (*data_tosend)["is_auto"] ? "Auto" : "Manual");
            if (ImGui::Button((*data_tosend)["is_auto"] ? "ON" : "OFF"))
            {
                (*data_tosend)["is_auto"] = !(*data_tosend)["is_auto"];
            }

            ImGui::Text("Gps or COlour: %s", (*data_tosend)["navModeColourGPS"] ? "Colour" : "Gps");
            if (ImGui::Button((*data_tosend)["navModeColourGPS"] ? "Colour" : "Gps"))
            {
                (*data_tosend)["navModeColourGPS"] = !(*data_tosend)["navModeColourGPS"];
            }

            if (!(*data_tosend)["navModeColourGPS"])
            { // treu then gps
                std::string lat = (*data_tosend).value("goal_lat", "");
                std::string lon = (*data_tosend).value("goal_lon", "");

                ImGui::InputText("Lat##d", &lat);
                ImGui::InputText("Lon##d", &lon);
                (*data_tosend)["goal_lon"] = lon;
                (*data_tosend)["goal_lat"] = lat;

                if (ImGui::Button("Confirm"))
                {
                    sendLL(client_fd, data_tosend);
                }

                ImGui::SameLine();

                if (ImGui::Button("Reset"))
                {
                    resetLL(data_tosend);
                }
            }
            else
            {
                int targetConeId = (*data_tosend)["colourId"];
                int setSkew = (*data_tosend)["to_skew"]; // 0for no skew 1 for skew
                int skew = (*data_tosend)["skew"];       // search skew id

                ImGui::InputInt("coneID", &targetConeId);
                ImGui::InputInt("setSkew?", &setSkew);
                (*data_tosend)["colourId"] = targetConeId;
                (*data_tosend)["to_skew"] = setSkew;
                if (setSkew)
                {

                    ImGui::InputInt("skew", &skew);
                    (*data_tosend)["skew"] = skew;
                }
                if (ImGui::Button("Confirm"))
                {
                    sendLL(client_fd, data_tosend);
                }

                ImGui::SameLine();

                if (ImGui::Button("Reset"))
                {
                    resetLL(data_tosend);
                }
            }

            ImGui::Separator();
            ImGui::Text("Nav Mode : %s,");
            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::BeginChild("Mast", ImVec2(0, midH), true);
            ImGui::Text("Mast Camera");
            ImGui::Dummy(ImVec2(0, midH - 40));
            ImGui::EndChild();

            ImGui::BeginChild("Console", ImVec2(0, 0), true);
            ImGui::Text("Console Log");
            ImGui::Separator();
            ImGui::TextWrapped("[INFO] Autonomous enabled");
            ImGui::TextWrapped("[WARN] Cone detected");
            ImGui::EndChild();

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Right", ImVec2(rightW, H), true);

            // Draw rover top view
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float panelW = ImGui::GetContentRegionAvail().x;
            float panelH = ImGui::GetContentRegionAvail().y;

            // Scale down rover
            float scale = 0.6f;

            // Rover body (1:2 width:height ratio)
            float roverH = panelH * 0.4f * scale;
            float roverW = roverH / 2.0f;
            ImVec2 bodyTL(pos.x + (panelW - roverW) / 2, pos.y + 20);
            ImVec2 bodyBR(bodyTL.x + roverW, bodyTL.y + roverH);
            draw_list->AddRectFilled(bodyTL, bodyBR, IM_COL32(100, 150, 220, 255), 5.0f);

            // Wheels (3 left, 3 right) - rectangular and slightly outside
            float wheelW = 15.0f * scale;
            float wheelH = 30.0f * scale;
            float wheelSpacing = roverH / 3.2;

            for (int i = 0; i < 3; i++)
            {
                // Left wheels
                ImVec2 wlTL(bodyTL.x - wheelW - 10, bodyTL.y + i * wheelSpacing + wheelSpacing / 2 - wheelH / 2);
                ImVec2 wlBR(wlTL.x + wheelW + 3, wlTL.y + wheelH);
                draw_list->AddRectFilled(wlTL, wlBR, IM_COL32(200, 100, 100, 255), 3.0f);

                // Right wheels
                ImVec2 wrTL(bodyBR.x + 5, bodyTL.y + i * wheelSpacing + wheelSpacing / 2 - wheelH / 2);
                ImVec2 wrBR(wrTL.x + wheelW, wrTL.y + wheelH);
                draw_list->AddRectFilled(wrTL, wrBR, IM_COL32(200, 100, 100, 255), 3.0f);
            }

            char bufLeft[32] = "PWM L";  /// calue
            char bufRight[32] = "PWM R"; // calue
            draw_list->AddText(ImVec2(bodyTL.x - wheelW - 10, bodyBR.y + 5), IM_COL32(255, 255, 255, 255), bufLeft);
            draw_list->AddText(ImVec2(bodyBR.x + 5, bodyBR.y + 5), IM_COL32(255, 255, 255, 255), bufRight);

            ImGui::EndChild();

            float avail_width = ImGui::GetContentRegionAvail().x;
            float column_width = avail_width / 3.0f;
            std::string lmao = " online";
            std::string l2u_text = "L2U:" + lmao;
            ImGui::SetCursorPosX(column_width / 2 - ImGui::CalcTextSize(l2u_text.c_str()).x / 2);
            ImGui::Text("%s", l2u_text.c_str());
            ImGui::SameLine();
            std::string jetson_text = "Jetson:" + lmao;
            ImGui::SetCursorPosX(column_width + column_width / 2 - ImGui::CalcTextSize(jetson_text.c_str()).x / 2);
            ImGui::Text("%s", jetson_text.c_str());
            ImGui::SameLine();
            std::string analog_text = "Analog Cameras:" + lmao;
            ImGui::SetCursorPosX(2 * column_width + column_width / 2 - ImGui::CalcTextSize(analog_text.c_str()).x / 2);
            ImGui::Text("%s", analog_text.c_str());
            ImGui::SameLine();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tab2"))
        {
            ImGui::Text("Analog Camera");
            ImGui::Separator();

            const int GRID_ROWS = 2;
            const int GRID_COLS = 4;

            float availW = ImGui::GetContentRegionAvail().x;
            float availH = ImGui::GetContentRegionAvail().y;

            float cellW = availW / GRID_COLS;
            float cellH = availH / GRID_ROWS;

            cv::Mat frame;

            for (int i = 0; i < (int)caps.size(); i++)
            {
                if (!caps[i].read(frame) || frame.empty())
                    continue;

                // Correct mirror (left ↔ right)
                cv::flip(frame, frame, 0);

                textures[i] = matToTexture(frame, textures[i]);

                ImGui::BeginGroup();

                ImVec2 imgSize(cellW - 8, cellH - 32);
                ImVec2 pos = ImGui::GetCursorScreenPos();

                ImGui::Image(
                    (void *)(intptr_t)textures[i],
                    imgSize,
                    ImVec2(0, 1),
                    ImVec2(1, 0));

                // Blue border
                ImDrawList *draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRect(
                    pos,
                    ImVec2(pos.x + imgSize.x, pos.y + imgSize.y),
                    IM_COL32(41, 86, 128, 255), // Blue
                    0.0f,
                    0,
                    2.0f);

                // Channel label
                ImGui::Text("CH %d", channels[i]);

                ImGui::EndGroup();

                if ((i + 1) % GRID_COLS != 0)
                    ImGui::SameLine();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tab3"))
        {
            ImGui::Text("Perception debug and detections");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tab4"))
        {
            ImGui::Text("System status and diagnostics");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

int main()
{

    std::vector<int> channels = {101, 201, 301, 401, 501, 601, 701, 801};
    std::vector<cv::VideoCapture> caps;
    std::vector<GLuint> textures(8, 0);

    for (int ch : channels)
    {
        std::string pipeline =
            "rtspsrc location=rtsp://admin:mrmecs2025@10.0.0.9:554/Streaming/Channels/" + std::to_string(ch) + " "
                                                                                                               "protocols=tcp latency=0 buffer-mode=none drop-on-latency=true ! "
                                                                                                               "decodebin ! videoconvert ! video/x-raw,format=BGR ! "
                                                                                                               "appsink drop=true max-buffers=1 sync=false";

        cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

        if (cap.isOpened())
            caps.push_back(std::move(cap));
    }
    int client_fd = 0;
    nlohmann::json data_tosend;
    data_tosend = {
        {"is_auto", false},
        {"navModeColourGPS", false}, // false is colour
        {"colourId", -1},
        {"to_skew", 0}, // whether to skew or not
        {"skew", -1},
        {"goal_lat", "nan"},
        {"goal_lon", "nan"},
    };
    glfwInit();
    const double FPS = 70.0;
    const double frameTime = 1.0 / FPS;
    double lastTime = glfwGetTime();

    double fpsTimer = lastTime;
    int frameCount = 0;

    joystick_handler joyBoy;
    GLFWwindow *window = glfwCreateWindow(960, 540, "MRM GUI", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO(); // Can leave it, but not used
    ImGui::StyleColorsDark();
    mystyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {

        double now = glfwGetTime();
        double delta = now - lastTime;

        if (delta < frameTime)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(frameTime - delta));
            continue;
        }

        lastTime = now;
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawUI(client_fd, &data_tosend, caps, textures, channels);

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        static double lastJoy = 0;
        if (now - lastJoy > 0.02) // 50 Hz
        {
            joyBoy.update();
            lastJoy = now;
        }

        frameCount++;

        if (now - fpsTimer >= 1.0)
        {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            fpsTimer = now;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}
