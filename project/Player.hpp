#ifndef Player_hpp
#define Player_hpp

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "Model3D.hpp"
#include "Shader.hpp"

class Player {
public:
    Player();

    void Load(const std::string& path);

    void Draw(gps::Shader& shader, glm::vec3 cameraPosition, glm::mat4 viewMatrix);

    void Rotate(float angle);

private:
    gps::Model3D model;


    glm::vec3 offset = glm::vec3(0.0f, -1.0f, 2.5f);

    float scale = .5f;

    float rotationOffset = 0.0f;

    float userRotation = 0.0f;
};

#endif
