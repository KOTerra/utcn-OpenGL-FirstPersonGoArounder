#include "Scene.hpp"

Scene::Scene() {
}

void Scene::addLight(glm::vec3 pos, glm::vec3 color, float constant, float linear, float quadratic) {
    PointLight light;
    light.position = pos;
    light.color = color;
    light.constant = constant;
    light.linear = linear;
    light.quadratic = quadratic;
    lights.push_back(light);
}

void Scene::Load() {
    myModel.LoadModel("models/scene/craioveclipsa.obj", "models/scene/");

    // Light 0: Headlight
    addLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);

    // Light 1: Scene Light (Red)
    addLight(glm::vec3(20.0f, 20.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.09f, 0.032f);
    addLight(glm::vec3(15.59f, 44.59f, 34.14f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.09f, 0.032f);

    // Light 2: Scene Light (Blue)
    addLight(glm::vec3(-13.2f, 43.0f, .2f), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.09f, 0.032f);

    // Light 3: Scene Light (Green)
    addLight(glm::vec3(-20.0f, 20.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.09f, 0.032f);

    // Light 4
    addLight(glm::vec3(80.0f, 80.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.05f, 0.032f);

    // Light 5
    addLight(glm::vec3(0.0f, 80.0f, -80.0f), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.05f, 0.032f);

    // Light 6
    addLight(glm::vec3(-80.0f, 80.0f, 0.0f), glm::vec3(0.7f, .3f, 0.1f), 1.0f, 0.05f, 0.032f);

    // Blue lights
    const int numLights = 12;
    float y_positions[numLights] = {43.0f, 46.0f, 49.0f, 52.0f, 55.0f, 58.0f, 61.0f, 64.0f, 67.0f, 70.f, 73.f, 76.f};
    float z_positions[numLights] = {0.2f, 4.0f, 7.8f, 11.0f, 14.8f, 17.0f, 20.0f, 23.8f, 27.0f, 30.8f, 34.f, 37.8f};

    for (int i = 0; i < numLights; i++) {
        addLight(
            glm::vec3(-13.2f, y_positions[i], z_positions[i]),
            glm::vec3(0.014f, 0.1f, 0.894f),
            1.0f, 0.009f, 0.032f
        );
    }

    // Easter lights
    const int numEasterLights = 8;
    float x_easter_positions[numEasterLights] = {
            156.4f, 163.0f, -106.876f, -177.618f, 74.427f, -192.695f, -186.862f, -8.47f
        };
    float y_easter_positions[numEasterLights] = {
            47.0f, 56.7f, 44.178f, 48.346f, 31.52f, 78.42f, 55.27f, 46.27f
        };
    float z_easter_positions[numEasterLights] = {
            -109.f, 81.12f, 210.57f, 128.19f, 220.386f, 82.363f, -121.111f, -255.49f
        };

    for (int i = 0; i < numEasterLights; i++) {
        addLight(
            glm::vec3(x_easter_positions[i], y_easter_positions[i], z_easter_positions[i]),
            glm::vec3(0.9f, 0.9f, 0.9f),
            1.0f, 0.009f, 0.016f
        );
    }

    //star destroyer
    addLight(glm::vec3(-99.0f, 95.72f, 21.91f), glm::vec3(0.3f, .0f, 0.7f), 1.0f, 0.05f, 0.032f);


    //explosion
    const int numExplosionLights = 3;
    float x_explosion_positions[numExplosionLights] = {
            -54.8339f, -27.441, -19.391
        };
    float y_explosion_positions[numExplosionLights] = {
            49.573f, 35.401f, 14.433f
        };
    float z_explosion_positions[numExplosionLights] = {
            54.727f, 38.194f, 33.635f
        };

    for (int i = 0; i < numExplosionLights; i++) {
        addLight(
            glm::vec3(x_explosion_positions[i], y_explosion_positions[i], z_explosion_positions[i]),
            glm::vec3(0.9f, 0.9f, 0.9f),
            1.0f, 0.009f, 0.016f
        );
    }
}

void Scene::InitLightUniforms(gps::Shader& shader) {
    shader.useShaderProgram();

    lightLocs.resize(lights.size());

    for (size_t i = 0; i < lights.size(); i++) {
        std::string i_str = std::to_string(i);
        std::string posName = "lights[" + i_str + "].position";
        std::string colName = "lights[" + i_str + "].color";
        std::string constName = "lights[" + i_str + "].constant";
        std::string linName = "lights[" + i_str + "].linear";
        std::string quadName = "lights[" + i_str + "].quadratic";

        lightLocs[i].position = glGetUniformLocation(shader.shaderProgram, posName.c_str());
        lightLocs[i].color = glGetUniformLocation(shader.shaderProgram, colName.c_str());
        lightLocs[i].constant = glGetUniformLocation(shader.shaderProgram, constName.c_str());
        lightLocs[i].linear = glGetUniformLocation(shader.shaderProgram, linName.c_str());
        lightLocs[i].quadratic = glGetUniformLocation(shader.shaderProgram, quadName.c_str());

        glUniform3fv(lightLocs[i].color, 1, glm::value_ptr(lights[i].color));
        glUniform1f(lightLocs[i].constant, lights[i].constant);
        glUniform1f(lightLocs[i].linear, lights[i].linear);
        glUniform1f(lightLocs[i].quadratic, lights[i].quadratic);
    }

    GLuint numLightsLoc = glGetUniformLocation(shader.shaderProgram, "numLights");
    glUniform1i(numLightsLoc, static_cast<int>(lights.size()));
}

void Scene::UpdateLightPositions(gps::Shader& shader, const glm::mat4& view) {
    for (size_t i = 0; i < lights.size(); i++) {
        if (i == 0) {
            glUniform3fv(lightLocs[0].position, 1, glm::value_ptr(glm::vec3(0.0f)));
        }
        else {
            glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(lights[i].position, 1.0f));
            glUniform3fv(lightLocs[i].position, 1, glm::value_ptr(lightPosEye));
        }
    }
}

void Scene::Draw(gps::Shader& shader) {
    myModel.Draw(shader);
}
