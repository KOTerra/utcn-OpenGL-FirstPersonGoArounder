cmake_minimum_required(VERSION 3.28)

# --- Project setup ---
set(PROJECT_NAME project)
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)

# --- Executable ---
add_executable(${PROJECT_NAME} main.cpp
        Shader.cpp
        Camera.cpp
        Model3D.cpp
        tiny_obj_loader.cpp
        stb_image.cpp
        Mesh.cpp
        Window.cpp
        SkyBox.cpp
)

# --- macOS ---
if (APPLE)
    # Detect Homebrew prefix (Apple Silicon vs Intel)
    if (EXISTS /opt/homebrew)
        set(HB_PREFIX /opt/homebrew)
    elseif (EXISTS /usr/local)
        set(HB_PREFIX /usr/local)
    else ()
        set(HB_PREFIX "")
    endif ()

    # Find GLFW
    find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h
            PATHS ${HB_PREFIX}/include /usr/local/include /opt/homebrew/include /usr/include
            NO_DEFAULT_PATH)
    find_library(GLFW_LIBRARY NAMES glfw glfw3
            PATHS ${HB_PREFIX}/lib /usr/local/lib /opt/homebrew/lib /usr/lib
            NO_DEFAULT_PATH)

    if (NOT GLFW_INCLUDE_DIR OR NOT GLFW_LIBRARY)
        message(FATAL_ERROR "❌ GLFW not found. Try: brew install glfw")
    endif ()

    message(STATUS "✅ Found GLFW include: ${GLFW_INCLUDE_DIR}")
    message(STATUS "✅ Found GLFW lib: ${GLFW_LIBRARY}")

    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY} "-framework OpenGL")

    # --- Windows ---
elseif (WIN32)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(GLEW REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw GLEW::GLEW opengl32)
endif ()
