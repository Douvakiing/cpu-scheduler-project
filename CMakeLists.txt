cmake_minimum_required(VERSION 3.16)
project(cpu-scheduler-project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Core scheduler sources (shared by console + GUI) ---
file(GLOB_RECURSE SCHEDULER_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --- Dependencies (downloaded at configure time) ---
include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9
)
FetchContent_MakeAvailable(glfw imgui)

set(IMGUI_DIR ${imgui_SOURCE_DIR})

add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

# --- Console executable ---
add_executable(cpu_scheduler main.cpp ${SCHEDULER_SOURCES})
target_include_directories(cpu_scheduler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- GUI executable ---
add_executable(cpu_scheduler_gui main_gui.cpp ${SCHEDULER_SOURCES})
target_include_directories(cpu_scheduler_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cpu_scheduler_gui PRIVATE imgui)

if(WIN32)
  target_link_libraries(cpu_scheduler_gui PRIVATE opengl32)
elseif(APPLE)
  target_link_libraries(cpu_scheduler_gui PRIVATE "-framework OpenGL")
else()
  find_package(OpenGL REQUIRED)
  target_link_libraries(cpu_scheduler_gui PRIVATE OpenGL::GL)
endif()
