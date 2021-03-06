
cmake_minimum_required(VERSION 3.8.0)
project(vmix VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_INCLUDE_CURRENTDIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )
include(MacroLogFeature)

if(UNIX)
    if (APPLE)
        add_definitions(-DAPPLE)

        set(OpenGL_DIR /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/)
        set(CMAKE_OSX_ARCHITECTURES x86_64)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")

    else()
        add_definitions(-DLINUX)

        set(OpenGL_GL_PREFERENCE "GLVND")

    endif()
    add_definitions(-DUNIX)
elseif(WIN32)
    add_definitions(-DWIN32)
    add_definitions(-DMINGW32)
endif(UNIX)


# Include the CMake RC module
include(CMakeRC)

#
# GSTREAMER
#

find_package(GStreamer 1.0.0 COMPONENTS base)
macro_log_feature(GSTREAMER_FOUND "GStreamer"
"Open Source Multiplatform Multimedia Framework"
"http://gstreamer.freedesktop.org/" TRUE "1.0.0")
macro_log_feature(GSTREAMER_BASE_LIBRARY_FOUND "GStreamer base library"
"${GSTREAMER_BASE_LIBRARY}"
"http://gstreamer.freedesktop.org/" FALSE "1.0.0")

find_package(GStreamerPluginsBase 1.0.0 COMPONENTS app audio video pbutils gl)
macro_log_feature(GSTREAMER_APP_LIBRARY_FOUND "GStreamer app library"
"${GSTREAMER_APP_LIBRARY}"
"http://gstreamer.freedesktop.org/" TRUE "1.0.0")

macro_log_feature(GSTREAMER_AUDIO_LIBRARY_FOUND "GStreamer audio library"
"${GSTREAMER_AUDIO_LIBRARY}"
"http://gstreamer.freedesktop.org/" TRUE "1.0.0")

macro_log_feature(GSTREAMER_VIDEO_LIBRARY_FOUND "GStreamer video library"
"${GSTREAMER_VIDEO_LIBRARY}"
"http://gstreamer.freedesktop.org/" TRUE "1.0.0")

macro_log_feature(GSTREAMER_PBUTILS_LIBRARY_FOUND "GStreamer pbutils library"
"${GSTREAMER_PBUTILS_LIBRARY}"
"http://gstreamer.freedesktop.org/" TRUE "1.0.0")

macro_log_feature(GSTREAMER_GL_LIBRARY_FOUND "GStreamer opengl library"
"${GSTREAMER_GL_LIBRARY}"
"http://gstreamer.freedesktop.org/" TRUE "1.0.0")


# Various preprocessor definitions for GST
add_definitions(-DGST_DISABLE_XML -DGST_DISABLE_LOADSAVE)

# Basics
find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "GLib" "GTK general-purpose utility library" "http://www.gtk.org/" TRUE)

find_package(GObject)
macro_log_feature(GOBJECT_FOUND "GObject" "GTK object-oriented framework" "http://www.gtk.org/" TRUE)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
set(THREAD_LIBRARY Threads::Threads)

find_package(PNG REQUIRED)
set(PNG_LIBRARY PNG::PNG)

#
# GLFW3
#
find_package(glfw3 3.2 REQUIRED)
macro_log_feature(glfw3_FOUND "GLFW3" "Open Source, multi-platform library for OpenGL" "http://www.glfw.org/" TRUE)
set(GLFW_LIBRARY glfw)

find_package(OpenGL REQUIRED)

#
# GLM
#
set(BUILD_STATIC_LIBS ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)
message(STATUS "Compiling 'GLM' OpenGL Mathematics https://glm.g-truc.net.")

#
# GLAD
#
set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glad/include)
add_library(GLAD "${CMAKE_CURRENT_SOURCE_DIR}/ext/glad/src/glad.c")
message(STATUS "Compiling 'GLAD' generated at https://glad.dav1d.de/ -- ${GLAD_INCLUDE_DIR}.")

#
# DEAR IMGUI
#
set(IMGUI_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui//imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui//imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui//imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/examples/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/examples/imgui_impl_opengl3.cpp
)
set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui)
add_library(IMGUI "${IMGUI_SRCS}")
target_compile_definitions(IMGUI PRIVATE "IMGUI_IMPL_OPENGL_LOADER_GLAD")
message(STATUS "Compiling 'Dear ImGui' from https://github.com/ocornut/imgui.git -- ${IMGUI_INCLUDE_DIR}.")

#
# ImGui Color Text Editor
#
set(IMGUITEXTEDIT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/ImGuiColorTextEdit)
set(IMGUITEXTEDIT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/ImGuiColorTextEdit/TextEditor.cpp
)
message(STATUS "Including 'ImGuiColorTextEdit' from https://github.com/BalazsJako/ImGuiColorTextEdit -- ${IMGUITEXTEDIT_INCLUDE_DIR}.")

#
# TINY XML 2
#
set(TINYXML2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinyxml2)
add_library(TINYXML2 "${CMAKE_CURRENT_SOURCE_DIR}/ext/tinyxml2/tinyxml2.cpp")
message(STATUS "Compiling 'TinyXML2' from https://github.com/leethomason/tinyxml2.git -- ${TINYXML2_INCLUDE_DIR}.")

#
# STB
#
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)
add_definitions(-DIMGUI_USE_STB_SPRINTF)
message(STATUS "Including 'STB Nothings' from https://github.com/nothings/stb -- ${STB_INCLUDE_DIR}.")

#
# DIRENT
#
if(WIN32)
    set(DIRENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext)
    message(STATUS "Including 'Dirent' from https://github.com/tronkko/dirent -- ${DIRENT_INCLUDE_DIR}.")
endif( WIN32 )

#
# TINY FILE DIALOG
#
set(TINYFD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tfd)
add_library(TINYFD "${CMAKE_CURRENT_SOURCE_DIR}/ext/tfd/tinyfiledialogs.c")
message(STATUS "Compiling 'TinyFileDialog' from https://github.com/native-toolkit/tinyfiledialogs.git -- ${TINYFD_INCLUDE_DIR}.")

#
# OBJ LOADER
#
#set(OBJLOADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/obj)
#add_library(OBJLOADER "${CMAKE_CURRENT_SOURCE_DIR}/ext/obj/ObjLoader.cpp")
#message(STATUS "Compiling 'ObjLoader' from https://github.com/mortennobel/OpenGL_3_2_Utils -- ${OBJLOADER_INCLUDE_DIR}.")

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

#
#  Application
#

# Setup the environment
include_directories(
    ${GSTREAMER_INCLUDE_DIR}
    ${GSTREAMER_AUDIO_INCLUDE_DIR}
    ${GSTREAMER_VIDEO_INCLUDE_DIR}
    ${GSTREAMER_BASE_INCLUDE_DIR}
    ${GSTREAMER_APP_INCLUDE_DIR}
    ${GSTREAMER_PBUTILS_INCLUDE_DIR}
    ${GSTREAMER_GL_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}/examples
    ${IMGUITEXTEDIT_INCLUDE_DIR}
    ${TINYXML2_INCLUDE_DIR}
    ${TINYFD_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${DIRENT_INCLUDE_DIR}
    ${OBJLOADER_INCLUDE_DIR}
)


set(VMIX_BINARY "vmix")
set(VMIX_SRCS
    main.cpp
    Log.cpp
    Shader.cpp
    ImageShader.cpp
    ImageProcessingShader.cpp
    Scene.cpp
    Primitives.cpp
    Mesh.cpp
    View.cpp
    Source.cpp
    Session.cpp
    SessionSource.cpp
    SessionVisitor.cpp
    GarbageVisitor.cpp
    SessionCreator.cpp
    Mixer.cpp
    Settings.cpp
    Screenshot.cpp
    Resource.cpp
    FileDialog.cpp
    MediaPlayer.cpp
    MediaSource.cpp
    FrameBuffer.cpp
    RenderingManager.cpp
    UserInterfaceManager.cpp
    PickingVisitor.cpp
    SearchVisitor.cpp
    ImGuiToolkit.cpp
    ImGuiVisitor.cpp
    GstToolkit.cpp
    GlmToolkit.cpp
    SystemToolkit.cpp
    tinyxml2Toolkit.cpp
)

set(VMIX_RSC_FILES
    ./rsc/shaders/simple.fs
    ./rsc/shaders/simple.vs
    ./rsc/shaders/image.fs
    ./rsc/shaders/image.vs
    ./rsc/shaders/imageprocessing.fs
    ./rsc/fonts/Hack-Regular.ttf
    ./rsc/fonts/Roboto-Regular.ttf
    ./rsc/fonts/Roboto-Bold.ttf
    ./rsc/fonts/Roboto-Italic.ttf
    ./rsc/fonts/fa-regular-400.ttf
    ./rsc/fonts/fa-solid-900.ttf
    ./rsc/images/mask_vignette.dds
    ./rsc/images/mask_halo.dds
    ./rsc/images/mask_circle.dds
    ./rsc/images/mask_roundcorner.dds
    ./rsc/images/mask_linear_top.dds
    ./rsc/images/mask_linear_bottom.dds
    ./rsc/images/mask_linear_left.dds
    ./rsc/images/mask_linear_right.dds
    ./rsc/images/v-mix_256x256.png
    ./rsc/images/rgb.png
    ./rsc/images/busy.png
    ./rsc/images/icons.dds
    ./rsc/images/seed_512.jpg
    ./rsc/images/transparencygrid.png
    ./rsc/images/shadow.png
    ./rsc/images/shadow_dark.png
    ./rsc/images/shadow_perspective.png
    ./rsc/mesh/point.ply
    ./rsc/mesh/disk.ply
    ./rsc/mesh/shadow.ply
    ./rsc/mesh/target.ply
    ./rsc/mesh/border_round.ply
    ./rsc/mesh/border_sharp.ply
    ./rsc/mesh/border_large_round.ply
    ./rsc/mesh/border_handles_rotation.ply
    ./rsc/mesh/border_handles_overlay.ply
    ./rsc/mesh/border_handles_sharp.ply
    ./rsc/mesh/border_large_sharp.ply
    ./rsc/mesh/border_vertical_overlay.ply
    ./rsc/mesh/perspective_layer.ply
    ./rsc/mesh/shadow_perspective.ply
    ./rsc/mesh/circle.ply
    ./rsc/mesh/icon_video.ply
    ./rsc/mesh/icon_image.ply
    ./rsc/mesh/icon_render.ply
    ./rsc/mesh/icon_clone.ply
    ./rsc/mesh/icon_vimix.ply
)

add_executable(${VMIX_BINARY}
    ${VMIX_SRCS}
    ${IMGUITEXTEDIT_SRC}
)

set_property(TARGET ${VMIX_BINARY} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${VMIX_BINARY} PROPERTY C_STANDARD 11)

target_compile_definitions(${VMIX_BINARY} PUBLIC "IMGUI_IMPL_OPENGL_LOADER_GLAD")

cmrc_add_resource_library(vmix-resources ALIAS vmix::rc NAMESPACE vmix WHENCE rsc ${VMIX_RSC_FILES})
message(STATUS "Using 'CMakeRC ' from https://github.com/vector-of-bool/cmrc.git -- ${CMAKE_MODULE_PATH}.")


target_link_libraries(${VMIX_BINARY} LINK_PRIVATE 
    ${GLFW_LIBRARY}
    ${OPENGL_LIBRARY}
    GLAD
    ${CMAKE_DL_LIBS}
    ${GOBJECT_LIBRARIES}
    ${GSTREAMER_LIBRARY}
    ${GSTREAMER_BASE_LIBRARY}
    ${GSTREAMER_APP_LIBRARY}
    ${GSTREAMER_AUDIO_LIBRARY}
    ${GSTREAMER_VIDEO_LIBRARY}
    ${GSTREAMER_PBUTILS_LIBRARY}
    ${GSTREAMER_GL_LIBRARY}
    ${NFD_LIBRARY}
    ${PNG_LIBRARY}
    ${THREAD_LIBRARY}
    TINYXML2
    TINYFD
    IMGUI
    glm::glm
    vmix::rc
)

macro_display_feature_log()
