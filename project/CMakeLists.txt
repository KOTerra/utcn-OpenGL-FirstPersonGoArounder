cmake_minimum_required(VERSION 3.28)

# --- Project setup ---
set(PROJECT_NAME project)
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)

# --- Executable ---
add_executable(${PROJECT_NAME}
        main.cpp
        Shader.cpp
        Camera.cpp
        Model3D.cpp
        tiny_obj_loader.cpp
        stb_image.cpp
        Mesh.cpp
        Window.cpp
        SkyBox.cpp
        Scene.cpp
        Player.cpp
)

# --- macOS ---
if(APPLE)
    # Detect Homebrew prefix (Apple Silicon vs Intel)
    if(EXISTS /opt/homebrew)
        set(HB_PREFIX /opt/homebrew)
    elseif(EXISTS /usr/local)
        set(HB_PREFIX /usr/local)
    else()
        set(HB_PREFIX "")
    endif()

    # Find GLFW
    find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h
            PATHS ${HB_PREFIX}/include /usr/local/include /opt/homebrew/include /usr/include
            NO_DEFAULT_PATH)
    find_library(GLFW_LIBRARY NAMES glfw glfw3
            PATHS ${HB_PREFIX}/lib /usr/local/lib /opt/homebrew/lib /usr/lib
            NO_DEFAULT_PATH)

    if(NOT GLFW_INCLUDE_DIR OR NOT GLFW_LIBRARY)
        message(FATAL_ERROR "❌ GLFW not found. Try: brew install glfw")
    endif()

    message(STATUS "✅ Found GLFW include: ${GLFW_INCLUDE_DIR}")
    message(STATUS "✅ Found GLFW lib: ${GLFW_LIBRARY}")

    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY} "-framework OpenGL")
endif()

# --- Windows ---
if(WIN32)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(GLEW REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw GLEW::GLEW opengl32)
endif()

# --- Copy assets to the executable directory (Debug & Release) ---
foreach(asset_dir models shaders textures skybox)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/${asset_dir}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/${asset_dir}
    )
endforeach()

# --- Optional: set working directory for IDEs like CLion/Xcode ---
set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
