#include <iostream>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include "Mesh.h"
#include "Model.h"
#include "GltfModel.h"
#include "Airplane.h"
#include "TextRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include "data.h"

const unsigned int width  = 1000;
const unsigned int height = 1000;

float skyboxVertices[] = {
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] = {
    1, 2, 6,  6, 5, 1,   // Right
    0, 4, 7,  7, 3, 0,   // Left
    4, 5, 6,  6, 7, 4,   // Top
    0, 3, 2,  2, 1, 0,   // Bottom
    0, 1, 5,  5, 4, 0,   // Back
    3, 7, 6,  6, 2, 3    // Front
};

int main() {
   
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "FlightSim", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    Shader shaderProgram("shaders/default.vert", "shaders/default.frag");
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader skyboxShader ("shaders/skybox.vert",  "shaders/skybox.frag");
    TextRenderer hud(width, height, "/System/Library/Fonts/Monaco.ttf", 16);

    shaderProgram.Activate();
    skyboxShader.Activate();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    //PA-28 Cherokee GLB
    GltfModel airplaneModel(
        "assets/piper-pa-28-cherokee-general-aviation-aircraft/source/object_0 (1).glb");

    Airplane airplane;
    //Piper Cherokee PA-28 specs
    airplane.mass          = 1000.0f;  
    airplane.engine.thrust = 1600.0f; 

    airplane.set_inertia(glm::mat3(
        948.0f,   0.0f,    0.0f,
          0.0f, 1346.0f,   0.0f,
          0.0f,   0.0f, 15000.0f
    ));

    Airfoil NACA_0012(NACA_0012_data);
    Airfoil NACA652415(NACA652415_data);

    const float wing_offset = -0.3f;
    const float tail_offset = -4.5f;

    glm::vec3 wing_normal     = Wing::calc_wing_normal(phi::UP,  0.0f);
    glm::vec3 elevator_normal = Wing::calc_wing_normal(phi::UP,  0.0f);

    //Main wings — NACA 65(2)-415
    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset, 0.0f, -2.0f), 4.57f, 1.73f, &NACA652415, wing_normal, 0.0f));
    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset, 0.0f,  2.0f), 4.57f, 1.73f, &NACA652415, wing_normal, 0.0f));

    //Ailerons
    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset - 1.5f, 0.0f, -3.2f), 2.0f, 0.4f, &NACA652415, wing_normal, 1.0f));
    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset - 1.5f, 0.0f,  3.2f), 2.0f, 0.4f, &NACA652415, wing_normal, 1.0f));

    //Elevator
    airplane.elements.push_back(Wing(
        glm::vec3(tail_offset, 0.0f, 0.0f), 3.35f, 0.9f, &NACA_0012, elevator_normal, 0.3f));

    //Rudder
    airplane.elements.push_back(Wing(
        glm::vec3(tail_offset, 0.0f, 0.0f), 1.5f, 0.9f, &NACA_0012, phi::RIGHT, 0.5f));

    airplane.position        = glm::vec3(0.0f, 2000.0f, 0.0f);
    airplane.velocity        = glm::vec3(65.0f, 0.0f, 0.0f);
    airplane.apply_gravity   = true;
    airplane.engine.throttle = 1.0f;

    //Rotate 90° around Y so the nose faces -Z
    const glm::quat modelCorrection =
        glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));


    //Coordinate system: phi X=fwd, Y=up, Z=right  →  GL X=right, Y=up, Z=back
    const glm::quat coordConversion = glm::angleAxis(glm::radians(90.0f), glm::vec3(0,1,0));

    Camera camera(width, height, glm::vec3(0.0f, 1015.0f, 60.0f));
    
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::string facesCubemap[6] = {
        "Skybox/right.jpg", "Skybox/left.jpg",
        "Skybox/top.jpg",   "Skybox/bottom.jpg",
        "Skybox/front.jpg", "Skybox/back.jpg"
    };

    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++) {
        int w, h, ch;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &w, &h, &ch, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Failed to load cubemap face: " << facesCubemap[i] << std::endl;
        }
    }

    std::vector<Vertex>  terrainVertices;
    std::vector<GLuint>  terrainIndices;
    const int   gridSize = 1000;
    const float spacing  = 20.0f;

    const float k = 10.0f * (0.05f / spacing);
    for (int x = 0; x < gridSize; x++) {
        for (int z = 0; z < gridSize; z++) {
            float y    = sinf(x * 0.05f) * cosf(z * 0.05f) * 10.0f;
            float dydx = cosf(x * 0.05f) * cosf(z * 0.05f) * k;
            float dydz = -sinf(x * 0.05f) * sinf(z * 0.05f) * k;
            terrainVertices.push_back({
                glm::vec3(x * spacing, y, z * spacing),
                glm::normalize(glm::vec3(-dydx, 1.0f, -dydz)),
                glm::vec3(0.3f, 0.6f, 0.2f),
                glm::vec2(x * 0.001f, z * 0.001f)
            });
        }
    }
    for (int x = 0; x < gridSize - 1; x++) {
        for (int z = 0; z < gridSize - 1; z++) {
            int s = x * gridSize + z;
            terrainIndices.push_back(s);
            terrainIndices.push_back(s + gridSize);
            terrainIndices.push_back(s + 1);
            terrainIndices.push_back(s + 1);
            terrainIndices.push_back(s + gridSize);
            terrainIndices.push_back(s + gridSize + 1);
        }
    }

    Texture grassTex("textures/grass.jpg", "diffuse", 0, GL_RGB, GL_UNSIGNED_BYTE);
    std::vector<Texture> terrainTextures = { grassTex };
    Mesh terrainMesh(terrainVertices, terrainIndices, terrainTextures);

    glEnable(GL_DEPTH_TEST);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        float dt  = glm::min(now - lastTime, 0.016f);
        lastTime  = now;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            airplane.pitch_control =  0.3f;
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            airplane.pitch_control = -0.3f;
        else
            airplane.pitch_control =  0.0f;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            airplane.roll_control = -1.0f;    
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            airplane.roll_control =  1.0f;    
        else
            airplane.roll_control =  0.0f;

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            airplane.yaw_control = -1.0f;
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            airplane.yaw_control =  1.0f;
        else
            airplane.yaw_control =  0.0f;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            airplane.engine.throttle = glm::min(airplane.engine.throttle + 0.4f * dt, 1.0f);
        else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            airplane.engine.throttle = glm::max(airplane.engine.throttle - 0.4f * dt, 0.0f);

        airplane.update(dt);

        glm::vec3 renderPos = glm::vec3(
             airplane.position.z,
             airplane.position.y,
            -airplane.position.x
        );

        glm::quat renderRot = coordConversion * airplane.rotation * modelCorrection;
        
        glm::vec3 flatFwd = glm::normalize(glm::vec3(
            airplane.velocity.z,
            0.0f,
            -airplane.velocity.x
        ));

        flatFwd = -flatFwd; 

        glm::vec3 targetCamPos = renderPos
            + flatFwd * 60.0f
            + glm::vec3(0.0f, 25.0f, 0.0f); 

        float lerpFactor = glm::clamp(dt * 5.0f, 0.0f, 1.0f);
        camera.Position    = glm::mix(camera.Position, targetCamPos, lerpFactor);
        glm::vec3 lookTarget = renderPos + flatFwd * 30.0f + glm::vec3(0.0f, 5.0f, 0.0f);  
        camera.Orientation = glm::normalize(lookTarget - camera.Position);


        camera.updateMatrix(45.0f, 0.1f, 50000.0f);

        glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.Activate();
        camera.Matrix(shaderProgram, "camMatrix");

        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"),
                    camera.Position.x, camera.Position.y, camera.Position.z);

        glm::vec3 sunDir = glm::normalize(glm::vec3(1.0f, 1.0f, 0.5f));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightPos"),
                     1, glm::value_ptr(sunDir));
        glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"),
                    1.0f, 1.0f, 0.95f, 1.0f);

        glm::mat4 airplaneMat = glm::mat4(1.0f);
        airplaneMat = glm::translate(airplaneMat, renderPos);
        airplaneMat = airplaneMat * glm::mat4_cast(renderRot);
        airplaneMat = glm::scale(airplaneMat, glm::vec3(8.0f));
        glDepthFunc(GL_LEQUAL);
        skyboxShader.Activate();

        glm::mat4 skyView = glm::mat4(glm::mat3(
            glm::lookAt(camera.Position,
                        camera.Position + camera.Orientation,
                        camera.Up)));
        glm::mat4 skyProj = glm::perspective(
            glm::radians(45.0f), (float)width / height, 0.1f, 50000.0f);

        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"),
                           1, GL_FALSE, glm::value_ptr(skyView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"),
                           1, GL_FALSE, glm::value_ptr(skyProj));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        terrainShader.Activate();
        camera.Matrix(terrainShader, "camMatrix");
        glUniform3f(glGetUniformLocation(terrainShader.ID, "camPos"),
                    camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3fv(glGetUniformLocation(terrainShader.ID, "lightPos"),
                     1, glm::value_ptr(sunDir));
        glUniform4f(glGetUniformLocation(terrainShader.ID, "lightColor"),
                    1.0f, 1.0f, 0.95f, 1.0f);
        glUniform1i(glGetUniformLocation(terrainShader.ID, "tex0"), 0);

        float tX = roundf(renderPos.x / (gridSize * spacing)) * (gridSize * spacing);
        float tZ = roundf(renderPos.z / (gridSize * spacing)) * (gridSize * spacing);

        glm::mat4 terrainMat = glm::translate(glm::mat4(1.0f),
            glm::vec3(tX - gridSize * spacing * 0.5f, 0.0f, tZ - gridSize * spacing * 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.ID, "model"),
                           1, GL_FALSE, glm::value_ptr(terrainMat));

        grassTex.Bind();
        terrainMesh.VAO.Bind();
        glDrawElements(GL_TRIANGLES, terrainMesh.indices.size(), GL_UNSIGNED_INT, 0);

        shaderProgram.Activate();
        camera.Matrix(shaderProgram, "camMatrix");
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"),
                    camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightPos"),
                     1, glm::value_ptr(sunDir));
        glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"),
                    1.0f, 1.0f, 0.95f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"),
                           1, GL_FALSE, glm::value_ptr(airplaneMat));
        airplaneModel.Draw(shaderProgram, camera);

        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            float speed    = glm::length(airplane.velocity);
            float alt_m    = airplane.position.y;
            float vs_ms    = airplane.velocity.y;
            float throttle = airplane.engine.throttle * 100.0f;
            float yaw_rate = glm::degrees(airplane.angular_velocity.y);

            glm::vec3 rgt  = airplane.rotation * phi::RIGHT;
            float roll_deg = glm::degrees(std::atan2(rgt.y,
                                 glm::length(glm::vec2(rgt.x, rgt.z))));

            char buf[64];
            const float x   = 14.0f;
            const float top = (float)height - 24.0f;
            const float dy  = 20.0f;
            const glm::vec3 white(1.0f);
            const glm::vec3 gray(0.15f, 0.15f, 0.18f);

            //background panel — drawn before text
            hud.DrawRect(6.0f, top - dy*5 - 8.0f, 230.0f, dy*5 + 32.0f, gray, 0.72f);

            snprintf(buf, sizeof(buf), "SPD  %6.1f m/s", speed);
            hud.RenderText(buf, x, top,       1.0f, white);

            snprintf(buf, sizeof(buf), "ALT  %7.0f m",
                     alt_m);
            hud.RenderText(buf, x, top-dy,    1.0f, white);

            snprintf(buf, sizeof(buf), "V/S  %+7.1f m/s", vs_ms);
            hud.RenderText(buf, x, top-dy*2,  1.0f, white);

            snprintf(buf, sizeof(buf), "THR  %5.1f%%", throttle);
            hud.RenderText(buf, x, top-dy*3,  1.0f, white);

            snprintf(buf, sizeof(buf), "ROLL %+6.1f deg", roll_deg);
            hud.RenderText(buf, x, top-dy*4,  1.0f, white);

            snprintf(buf, sizeof(buf), "YAW  %+6.1f d/s", yaw_rate);
            hud.RenderText(buf, x, top-dy*5,  1.0f, white);

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shaderProgram.Delete();
    terrainShader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}