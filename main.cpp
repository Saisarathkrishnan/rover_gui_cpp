#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
void mystyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
}

void DrawUI()
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    // Flags to make it act like the main window
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
            ImGui::Text("Fix        : %d", 1);
            ImGui::Text("Satellites : %d", 22);
            ImGui::Text("HDOP       : %.2f", 0.9f);
            ImGui::Text("VDOP       : %.2f", 1.1f);
            ImGui::Separator();
            ImGui::Text("Current Position");
            ImGui::Text("Lat : %.6f", 13.345441);
            ImGui::Text("Lon : %.6f", 74.794537);
            ImGui::Text("Alt : %.2f", 79.7f);
            ImGui::Separator();
            ImGui::Text("Destination");
            static float dlat = 13.346f, dlon = 74.795f, dalt = 80.0f;
            ImGui::InputFloat("Lat##d", &dlat, 0, 0, "%.6f");
            ImGui::InputFloat("Lon##d", &dlon, 0, 0, "%.6f");
            ImGui::InputFloat("Alt##d", &dalt, 0, 0, "%.2f");
            ImGui::Text("Distance : %.2f m", 12.4f);
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Middle", ImVec2(midW, H), false);

            float topH = H * 0.35f;
            float midH = H * 0.30f;

            ImGui::BeginChild("TopRow", ImVec2(0, topH), false);

            ImGui::BeginChild("ZED", ImVec2(midW * 0.6f, topH), true);
            ImGui::Text("ZED Camera");
            ImGui::Dummy(ImVec2(0, topH - 30));
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("NavState", ImVec2(0, topH), true);
            static int control_mode = 1;
            ImGui::Text("Nav State");
            ImGui::RadioButton("Manual", &control_mode, 0);
            ImGui::RadioButton("Autonomous", &control_mode, 1);
            ImGui::Separator();
            ImGui::Text("Nav Mode : WAYPOINT");
            ImGui::EndChild();

            ImGui::EndChild();

            ImGui::BeginChild("Mast", ImVec2(0, midH), true);
            ImGui::Text("Mast Camera");
            ImGui::Dummy(ImVec2(0, midH - 30));
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
            std::string lmao=" online";
            std::string l2u_text = "L2U:"+lmao;
            ImGui::SetCursorPosX(column_width / 2 - ImGui::CalcTextSize(l2u_text.c_str()).x / 2);
            ImGui::Text("%s", l2u_text.c_str());
            ImGui::SameLine();
            std::string jetson_text = "Jetson:"+lmao;
            ImGui::SetCursorPosX(column_width + column_width / 2 - ImGui::CalcTextSize(jetson_text.c_str()).x / 2);
            ImGui::Text("%s", jetson_text.c_str());
            ImGui::SameLine();
            std::string analog_text = "Analog Cameras:"+lmao;
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

        DrawUI();

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
