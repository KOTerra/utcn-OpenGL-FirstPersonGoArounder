//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright © 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include <string>

#define MAX_LIGHTS 19

int glWindowWidth = 1280;
int glWindowHeight = 720;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

// Struct to hold world-space light data
struct PointLight {
    glm::vec3 position; // World Space
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
};

// Array of lights
PointLight pointLights[MAX_LIGHTS];

// Struct to hold uniform locations for one light
struct PointLightLocs {
    GLuint position;
    GLuint color;
    GLuint constant;
    GLuint linear;
    GLuint quadratic;
};

// Array of uniform locations
PointLightLocs pointLightLocs[MAX_LIGHTS];


gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 2.5f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

#define BASE_CAMERA_SPEED 0.1f
#define SPRINT_CAMERA_SPEED 0.7f
float cameraSpeed = BASE_CAMERA_SPEED;

bool pressedKeys[1024];
float angleY = 0.0f;

gps::Model3D myModel;
gps::Shader myCustomShader;

GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM: error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}


bool firstMouse = true;
double lastX = glWindowWidth / 2.0;
double lastY = glWindowHeight / 2.0;
float yaw = -90.0f;
float pitch = 0.0f;


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos);
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // --- CHANGED ---
    // Update only the scene lights (starting from i = 1)
    // Light 0 is the headlight and stays at (0,0,0) in Eye Space
    for (int i = 1; i < MAX_LIGHTS; i++) {
        glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(pointLights[i].position, 1.0f));
        glUniform3fv(pointLightLocs[i].position, 1, glm::value_ptr(lightPosEye));
    }
    // --- END CHANGE ---
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    glViewport(0, 0, width, height);
    glfwGetFramebufferSize(window, &retina_width, &retina_height);

    int newWidth, newHeight;
    glfwGetWindowSize(window, &newWidth, &newHeight);
    glfwSetCursorPos(window, newWidth / 2.0, newHeight / 2.0);

    lastX = newWidth / 2.0;
    lastY = newHeight / 2.0;

    // Using 45.0f from initUniforms, not 105.0f
    projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(retina_width) / static_cast<float>(retina_height),
        0.1f,
        1000.0f
    );

    myCustomShader.useShaderProgram();
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}


void processMovement() {
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
        cameraSpeed = SPRINT_CAMERA_SPEED;
    }
    else {
        cameraSpeed = BASE_CAMERA_SPEED;
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angleY -= 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angleY += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    bool cameraMoved = false;
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        cameraMoved = true;
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        cameraMoved = true;
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        cameraMoved = true;
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        cameraMoved = true;
    }

    if (cameraMoved) {
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        // --- CHANGED ---
        // Update only the scene lights (starting from i = 1)
        for (int i = 1; i < MAX_LIGHTS; i++) {
            glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(pointLights[i].position, 1.0f));
            glUniform3fv(pointLightLocs[i].position, 1, glm::value_ptr(lightPosEye));
        }
        // --- END CHANGE ---
    }
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1);

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
    return true;
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0);
    glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
    myModel.LoadModel("models/scene/craioveclipsa.obj", "models/scene/");
}

void initShaders() {
    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();
}

void initUniforms() {
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // --- CHANGED ---
    // Light 0: Headlight (White)
    // Its world position is irrelevant, we'll set its eye position to (0,0,0)
    pointLights[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    pointLights[0].color = glm::vec3(1.0f, 1.0f, 1.0f); // White
    pointLights[0].constant = 1.0f;
    pointLights[0].linear = 0.09f;
    pointLights[0].quadratic = 0.032f;

    // Light 1: Scene Light (Red)
    pointLights[1].position = glm::vec3(20.0f, 20.0f, 0.0f); // World Space
    pointLights[1].color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
    pointLights[1].constant = 1.0f;
    pointLights[1].linear = 0.09f;
    pointLights[1].quadratic = 0.032f;

    // Light 2: Scene Light (Blue)
    pointLights[2].position = glm::vec3(-13.2f, 43.0f, .2f); // World Space
    pointLights[2].color = glm::vec3(0.0f, 0.0f, 1.0f); // Blue
    pointLights[2].constant = 1.0f;
    pointLights[2].linear = 0.09f;
    pointLights[2].quadratic = 0.032f;

    // Light 3: Scene Light (Green)
    pointLights[3].position = glm::vec3(-20.0f, 20.0f, 0.0f); // World Space
    pointLights[3].color = glm::vec3(0.0f, 1.0f, 0.0f); // Green
    pointLights[3].constant = 1.0f;
    pointLights[3].linear = 0.09f;
    pointLights[3].quadratic = 0.032f;


    pointLights[4].position = glm::vec3(80.0f, 80.0f, 0.0f); // World Space
    pointLights[4].color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
    pointLights[4].constant = 1.0f;
    pointLights[4].linear = 0.05f;
    pointLights[4].quadratic = 0.032f;

    pointLights[5].position = glm::vec3(0.0f, 80.0f, -80.0f); // World Space
    pointLights[5].color = glm::vec3(0.0f, 0.0f, 1.0f); // Blue
    pointLights[5].constant = 1.0f;
    pointLights[5].linear = 0.05f;
    pointLights[5].quadratic = 0.032f;

    pointLights[6].position = glm::vec3(-80.0f, 80.0f, 0.0f); // World Space
    pointLights[6].color = glm::vec3(0.7f, .3f, 0.1f); // Orange
    pointLights[6].constant = 1.0f;
    pointLights[6].linear = 0.05f;
    pointLights[6].quadratic = 0.032f;


    const int numLights = 12;
    float y_positions[numLights] = {43.0f, 46.0f, 49.0f, 52.0f, 55.0f, 58.0f, 61.0f, 64.0f, 67.0f, 70.f, 73.f, 76.f};
    float z_positions[numLights] = {0.2f, 4.0f, 7.8f, 11.0f, 14.8f, 17.0f, 20.0f, 23.8f, 27.0f, 30.8f, 34.f, 37.8f};

    for (int i = 0; i < numLights; i++) {
        int lightIndex = i + 7;

        // Set the unique position
        pointLights[lightIndex].position = glm::vec3(
            -13.2f, // Constant X
            y_positions[i], // Y from our array
            z_positions[i] // Z from our array
        );

        // Set the common properties
        pointLights[lightIndex].color = glm::vec3(0.0f, 0.0f, 1.0f); // Blue
        pointLights[lightIndex].constant = 1.0f;
        pointLights[lightIndex].linear = 0.09f;
        pointLights[lightIndex].quadratic = 0.032f;
    }


    for (int i = 0; i < MAX_LIGHTS; i++) {
        std::string i_str = std::to_string(i);
        std::string posName = "lights[" + i_str + "].position";
        std::string colName = "lights[" + i_str + "].color";
        std::string constName = "lights[" + i_str + "].constant";
        std::string linName = "lights[" + i_str + "].linear";
        std::string quadName = "lights[" + i_str + "].quadratic";

        pointLightLocs[i].position = glGetUniformLocation(myCustomShader.shaderProgram, posName.c_str());
        pointLightLocs[i].color = glGetUniformLocation(myCustomShader.shaderProgram, colName.c_str());
        pointLightLocs[i].constant = glGetUniformLocation(myCustomShader.shaderProgram, constName.c_str());
        pointLightLocs[i].linear = glGetUniformLocation(myCustomShader.shaderProgram, linName.c_str());
        pointLightLocs[i].quadratic = glGetUniformLocation(myCustomShader.shaderProgram, quadName.c_str());

        // Send data that doesn't change every frame
        glUniform3fv(pointLightLocs[i].color, 1, glm::value_ptr(pointLights[i].color));
        glUniform1f(pointLightLocs[i].constant, pointLights[i].constant);
        glUniform1f(pointLightLocs[i].linear, pointLights[i].linear);
        glUniform1f(pointLightLocs[i].quadratic, pointLights[i].quadratic);

        // Send the initial eye-space position
        if (i == 0) {
            // Light 0 is the headlight, fixed at (0,0,0) in Eye Space
            glUniform3fv(pointLightLocs[0].position, 1, glm::value_ptr(glm::vec3(0.0f)));
        }
        else {
            // Other lights are in the scene, transform their world pos to eye pos
            glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(pointLights[i].position, 1.0f));
            glUniform3fv(pointLightLocs[i].position, 1, glm::value_ptr(lightPosEye));
        }
    }
    // --- END CHANGE ---
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    myModel.Draw(myCustomShader);
}

void cleanup() {
    glfwDestroyWindow(glWindow);
    glfwTerminate();
}

int main(int argc, const char* argv[]) {
    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initShaders();
    initUniforms(); // Must be after initShaders

    int newWidth, newHeight;
    glfwGetWindowSize(glWindow, &newWidth, &newHeight);
    glfwSetCursorPos(glWindow, newWidth / 2.0, newHeight / 2.0);
    lastX = newWidth / 2.0;
    lastY = newHeight / 2.0;

    while (!glfwWindowShouldClose(glWindow)) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}
