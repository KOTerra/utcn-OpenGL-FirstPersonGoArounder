#include "Player.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <cmath>

Player::Player() {
}

void Player::Load(const std::string& path) {
    model.LoadModel(path, "models/player/");
}

void Player::Rotate(float angle) {
    userRotation += angle;
}

void Player::Draw(gps::Shader& shader, glm::vec3 cameraPosition, glm::mat4 viewMatrix) {
    shader.useShaderProgram();

    // yaw
    glm::mat4 inverseView = glm::inverse(viewMatrix);

    // camera fwd vector = -Z
    glm::vec3 forward = -glm::vec3(inverseView[2]);
    forward.y = 0.0f; // ignore vertical
    forward = glm::normalize(forward);

    float cameraAngleDegrees = glm::degrees(std::atan2(forward.x, forward.z));

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    modelMatrix = glm::translate(modelMatrix, cameraPosition);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(cameraAngleDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, offset);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationOffset + userRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    model.Draw(shader);
}
