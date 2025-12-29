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
#include "SkyBox.hpp"
#include "Scene.hpp"
#include "Player.hpp"

#include <iostream>
#include <string>
#include <vector>
#include "stb_image.h"

gps::Window myWindow;

glm::mat4 model;
GLuint modelLoc;

glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap = false;

gps::Model3D screenQuad;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

glm::vec3 lightDir;
glm::mat4 lightRotation;
GLfloat lightAngle;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

gps::Shader rainShader;
GLuint rainVAO, rainVBO;
GLuint rainTexture;
bool rainEnabled = false;

Scene myScene;
Player myPlayer;

gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 2.5f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

#define BASE_CAMERA_SPEED 0.1f
#define SPRINT_CAMERA_SPEED 0.7f
float cameraSpeed = BASE_CAMERA_SPEED;

bool pressedKeys[1024];
float angleY = 0.0f;

gps::Shader myCustomShader;

int glRenderMode = 0;

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

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void initRain() {
    rainShader.loadShader("shaders/rain.vert", "shaders/rain.frag");

    rainTexture = loadTexture("textures/rain2.png");

    float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

    glGenVertexArrays(1, &rainVAO);
    glGenBuffers(1, &rainVBO);
    glBindVertexArray(rainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void drawRain() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    rainShader.useShaderProgram();

    glUniform1f(glGetUniformLocation(rainShader.shaderProgram, "time"), (float)glfwGetTime());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rainTexture);
    glUniform1i(glGetUniformLocation(rainShader.shaderProgram, "rainTexture"), 0);

    glBindVertexArray(rainVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        glRenderMode = (glRenderMode + 1) % 3;
        switch (glRenderMode) {
        case 0: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case 1: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 2: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        }
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        rainEnabled = !rainEnabled;
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}


bool firstMouse = true;
double lastX = 640;
double lastY = 360;
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

    myScene.UpdateLightPositions(myCustomShader, view);
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

    int retina_width, retina_height;
    glfwGetFramebufferSize(window, &retina_width, &retina_height);


    myWindow.setWindowDimensions(WindowDimensions{retina_width, retina_height});

    if (!showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);
    }

    int newWidth, newHeight;
    glfwGetWindowSize(window, &newWidth, &newHeight);
    glfwSetCursorPos(window, newWidth / 2.0, newHeight / 2.0);

    lastX = newWidth / 2.0;
    lastY = newHeight / 2.0;

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

    if (pressedKeys[GLFW_KEY_J]) {
        myPlayer.Rotate(2.0f);
    }
    if (pressedKeys[GLFW_KEY_L]) {
        myPlayer.Rotate(-2.0f);
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

        myScene.UpdateLightPositions(myCustomShader, view);
    }

    if (pressedKeys[GLFW_KEY_P]) {
        glm::mat4 currentView = myCamera.getViewMatrix();
        glm::mat4 inverseView = glm::inverse(currentView);
        glm::vec3 worldPos = glm::vec3(inverseView[3]);

        std::cout << "Camera World Coords: X="
            << worldPos.x << " Y="
            << worldPos.y << " Z="
            << worldPos.z << std::endl;
    }
}


void initOpenGLState() {
    glClearColor(0.4f, 0.015f, 0.01, 1.0);

    WindowDimensions dims = myWindow.getWindowDimensions();
    glViewport(0, 0, dims.width, dims.height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);

    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
                           0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView = myCamera.getViewMatrix();


    const GLfloat near_plane = -20.0f, far_plane = 200.0f;
    const GLfloat projection_size = 10.0f;

    glm::mat4 lightProjection = glm::ortho(-projection_size, projection_size, -projection_size, projection_size,
                                           near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void initShaders() {
    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();

    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
}

void initSkyBox() {
    std::vector<const GLchar*> skyBoxFaces;
    skyBoxFaces.push_back("skybox/redplanet_rt.tga");
    skyBoxFaces.push_back("skybox/redplanet_lf.tga");
    skyBoxFaces.push_back("skybox/redplanet_up.tga");
    skyBoxFaces.push_back("skybox/redplanet_dn.tga");
    skyBoxFaces.push_back("skybox/redplanet_bk.tga");
    skyBoxFaces.push_back("skybox/redplanet_ft.tga");
    mySkyBox.Load(skyBoxFaces);
}

void initUniforms() {
    myCustomShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    WindowDimensions dims = myWindow.getWindowDimensions();
    projection = glm::perspective(glm::radians(45.0f), (float)dims.width / (float)dims.height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightDir = glm::vec3(0.0f, 5.0f, 5.0f);
    lightAngle = 0.0f;
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    myScene.InitLightUniforms(myCustomShader);
}

void initObjects() {
    myScene.Load();
    myPlayer.Load("models/player/player.obj");
    screenQuad.LoadModel("models/quad.obj");
}

void renderScene() {
    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 inverseView = glm::inverse(view);
    glm::vec3 cameraPosition = glm::vec3(inverseView[3]);

    glDisable(GL_CULL_FACE);
    myPlayer.Draw(depthMapShader, cameraPosition, view);
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    if (showDepthMap) {
        WindowDimensions dims = myWindow.getWindowDimensions();
        glViewport(0, 0, dims.width, dims.height);
        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {
        WindowDimensions dims = myWindow.getWindowDimensions();
        glViewport(0, 0, dims.width, dims.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myCustomShader.useShaderProgram();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(computeLightSpaceTrMatrix()));

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

        inverseView = glm::inverse(view);
        cameraPosition = glm::vec3(inverseView[3]);

        myPlayer.Draw(myCustomShader, cameraPosition, view);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        myScene.UpdateLightPositions(myCustomShader, view);
        myScene.Draw(myCustomShader);

        mySkyBox.Draw(skyboxShader, view, projection);

        if (rainEnabled) {
            drawRain();
        }
    }
}

void mainLoop() {
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());
    }
}

int main(int argc, const char* argv[]) {
    try {
        myWindow.Create(1280, 720, "Bulevardul Oltenia 34");
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    initOpenGLState();

    initObjects();

    initShaders();
    initUniforms();
    initFBO();
    initSkyBox();

    initRain();

    int newWidth, newHeight;
    glfwGetWindowSize(myWindow.getWindow(), &newWidth, &newHeight);
    glfwSetCursorPos(myWindow.getWindow(), newWidth / 2.0, newHeight / 2.0);
    lastX = newWidth / 2.0;
    lastY = newHeight / 2.0;


    mainLoop();

    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);

    myWindow.Delete();

    return 0;
}
