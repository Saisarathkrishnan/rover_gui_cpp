#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
#include <string>
#include <arpa/inet.h>
#include <nlohmann/json.hpp>
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

void sendLL(int client_fd,nlohmann::json *data_tosend)
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

void DrawUI(int client_fd, nlohmann::json *data_tosend)
{

    uint32_t len_net = 0;
    int r = recv(client_fd, &len_net, sizeof(len_net), MSG_WAITALL);
    if (r <= 0)
    {
        return;
    }

    uint32_t len = ntohl(len_net);
    if (len == 0 || len > 1024 * 1024)
    {
        return;
    }

    std::string data(len, '\0');
    r = recv(client_fd, data.data(), len, MSG_WAITALL);
    if (r <= 0)
    {
        return;
    }

    nlohmann::json mj;
    try
    {
        mj = nlohmann::json::parse(data);
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        std::cerr << "RAW DATA: [" << data << "]\n";
        return;
    }
    /*
    std::cout << "---- dota ----\n";

    for (auto &[key, value] : mj.items())
    {
        std::cout << key << " : ";

        if (value.is_string())
            std::cout << value.get<std::string>();
        else
            std::cout << value.dump();

        std::cout << std::endl;
    }

    std::cout << "-----------------------\n";

    */
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
                    sendLL(client_fd,data_tosend);
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
                    sendLL(client_fd,data_tosend);
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
            ImGui::Text("Arm and gripper UI");
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

    nlohmann::json data_tosend;
    data_tosend = {
        {"is_auto", false},
        {"navModeColourGPS", false}, // false is colour
        {"colourId", -1},
        {"to_skew", 0}, //whether to skew or not
        {"skew", -1},
        {"goal_lat", "nan"},
        {"goal_lon", "nan"},
    };

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    listen(server_fd, 1);
    std::cout << "Waiting for connection...\n";
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0)
    {
        perror("accept");
        return 1;
    }

    glfwInit();
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
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawUI(client_fd, &data_tosend);

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}
