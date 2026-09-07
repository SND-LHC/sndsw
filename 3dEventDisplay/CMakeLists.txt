cmake_minimum_required(VERSION 3.15)
project(SNDDisplay LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- LOCAL DEPENDENCIES ---
set(GLAD_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/external/glad")
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${GLAD_DIR}/include"
)

# --- DEPENDENCIES DOWNLOAD ---
include(FetchContent)

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)        # Don't build docs
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)           # Don't install the library on the system, keep it into the build folder
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)      # Enable Wayland support
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)          # Enable X11 support
set(GLFW_BUILD_LIB_STATIC ON CACHE BOOL "" FORCE)   # Compile and include GLFW into the final executable
set(GLFW_BUILD_LIB_SHARED OFF CACHE BOOL "" FORCE)  # Don't build GLFW as a dynamic library

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)

# ASSIMP
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.3
)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)              # Don't interpret warnings as errors
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)                        # Disable all file exporters
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)  # Disable all file importers
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)               # Enable only OBJ importer
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)              # Disable extra tool building
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "" FORCE)               # Disable assimp gui
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)                   # Disable assimp tutorials
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)                     # Disable assimp test suuite building
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)                         # Don't install the library on the system, keep it into the build folder
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)                       # Build assimp zlib version
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)              # Enable glTF importer

# STB IMAGE
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        2c980bb59875b0d32144a71867fbdebb2f77cd20 # pin: stb has no formal tags/releases
)

# IMGUI FILE DIALOG
FetchContent_Declare(
    imguifiledialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG        v0.6.8
)

FetchContent_MakeAvailable(glfw glm assimp stb imguifiledialog)

# --- PROJECT SOURCES ---
set(PROJECT_SOURCES
    # core
    src/core/App.cpp
    src/core/AppSettings.cpp
    src/core/Callbacks.cpp
    src/core/WindowManager.cpp
    src/core/state/AppStateManager.cpp

    # io
    src/io/DetectorData.cpp
    src/io/EventData.cpp
    src/io/GeometryMismatchException.cpp
    src/io/SndswEventManager.cpp

    # main
    src/main.cpp

    # rendering
    src/rendering/AxisWidget.cpp
    src/rendering/Viewport.cpp
    src/rendering/camera/BasicProjection.cpp
    src/rendering/camera/Camera.cpp
    src/rendering/camera/OrthographicProjection.cpp
    src/rendering/camera/PerspectiveProjection.cpp
    src/rendering/engine/GpuMesh.cpp
    src/rendering/engine/GpuMeshFactory.cpp
    src/rendering/engine/Material.cpp
    src/rendering/engine/Shader.cpp
    src/rendering/engine/ShaderMaker.cpp
    src/rendering/engine/Texture.cpp
    src/rendering/engine/TextureMaker.cpp

    # scene
    src/scene/Mesh.cpp
    src/scene/Node.cpp
    src/scene/Object.cpp
    src/scene/ObjectFactory.cpp
    src/scene/PointLight.cpp
    src/scene/Scene.cpp
    src/scene/colors/ClockCycleVariableGetter.cpp
    src/scene/colors/ColorPalette.cpp
    src/scene/colors/EnergyVariableGetter.cpp
    src/scene/colors/FixedValueMapper.cpp
    src/scene/colors/LinearValueMapper.cpp
    src/scene/colors/LogarithmicValueMapper.cpp
    src/scene/colors/TimeVariableGetter.cpp

    # ui
    src/ui/Gui.cpp

    # utils
    src/utils/StbImpl.cpp
)

set(LINK_DIRECTORIES
    ${ROOT_LIBRARY_DIR}
    ${FAIRROOT_LIBRARY_DIR}
    ${GEANE_LIBRARY_DIR}
    $ENV{ROOTSYS}/lib
    $ENV{FAIRROOTPATH}/lib
)

link_directories(${LINK_DIRECTORIES})

set(BINARY_NAME "3dEventDisplay")

add_executable(${BINARY_NAME} ${PROJECT_SOURCES} "${GLAD_DIR}/src/glad.c")

target_include_directories(${BINARY_NAME} PRIVATE
    "include"
    "${stb_SOURCE_DIR}"
    "${imguifiledialog_SOURCE_DIR}"
    "${GLAD_DIR}/include"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    ${ROOT_INCLUDE_DIRS}
    ${FAIRROOT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/shipdata
    ${CMAKE_SOURCE_DIR}/shipLHC
    ${CMAKE_SOURCE_DIR}/passive
    ${CMAKE_SOURCE_DIR}/analysis/tools
)

# --- LINKING ---
find_package(OpenGL REQUIRED)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
target_link_libraries(ImGuiFileDialog PUBLIC imgui)

target_link_libraries(${BINARY_NAME} PRIVATE
    ImGuiFileDialog
    imgui
    glm::glm
    assimp::assimp
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    snd_analysis_tools
    shipLHC
    ShipPassive
    ShipData
    veto
    ${ROOT_LIBRARIES}
    ${FAIRROOT_LIBRARIES}
)

# --- ASSETS ---
set(ASSETS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(EXISTS "${ASSETS_SRC}")
    add_custom_command(TARGET ${BINARY_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SRC}"
        "$<TARGET_FILE_DIR:${BINARY_NAME}>/assets"
        COMMENT "Copying assets to build directory..."
    )
endif()
