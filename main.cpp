#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

void mystyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

void DrawUI()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Main", nullptr, flags);

    if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown))
    {
        if (ImGui::BeginTabItem("Tab1"))
        {
            float W = ImGui::GetContentRegionAvail().x;
            float H = ImGui::GetContentRegionAvail().y;

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
            ImGui::Text("Reserved");
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("tab2"))
        {
            ImGui::Text("Arm and gripper UI");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("tab3"))
        {
            ImGui::Text("Perception debug and detections");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("tab4"))
        {
            ImGui::Text("System status and diagnostics");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::Text("L2U: ONLINE   Jetson: ONLINE   Analog Cam: ONLINE");

    ImGui::End();
}

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(960, 540, "MRM GUI", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}
