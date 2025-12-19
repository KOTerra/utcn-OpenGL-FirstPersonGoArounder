#ifndef Scene_hpp
#define Scene_hpp

#include <vector>
#include <string>
#include <iostream>

// --- FIXED: Platform-specific OpenGL header inclusion ---
#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
// --------------------------------------------------------

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model3D.hpp"
#include "Shader.hpp"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
};

struct PointLightLocs {
    GLuint position;
    GLuint color;
    GLuint constant;
    GLuint linear;
    GLuint quadratic;
};

class Scene {
public:
    Scene();

    void Load();

    void InitLightUniforms(gps::Shader& shader);

    void UpdateLightPositions(gps::Shader& shader, const glm::mat4& view);

    void Draw(gps::Shader& shader);

private:
    gps::Model3D myModel;
    std::vector<PointLight> lights;
    std::vector<PointLightLocs> lightLocs;

    void addLight(glm::vec3 pos, glm::vec3 color, float constant, float linear, float quadratic);
};

#endif /* Scene_hpp */