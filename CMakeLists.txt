cmake_minimum_required(VERSION 3.10)
project(gui)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(gui
    main.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp 
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/misc/cpp/imgui_stdlib.cpp

)

target_include_directories(gui PRIVATE imgui imgui/backends ${OpenCV_INCLUDE_DIRS})
target_link_libraries(gui glfw OpenGL::GL ${OpenCV_LIBS})

add_executable(controller
    controller.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(controller PRIVATE
    imgui
    imgui/backends
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(controller
    glfw
    OpenGL::GL
    ${OpenCV_LIBS}
)