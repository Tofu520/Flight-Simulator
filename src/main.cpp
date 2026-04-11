#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include "Mesh.h"
#include "Model.h"
#include "Airplane.h"
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
    Shader skyboxShader ("shaders/skybox.vert",  "shaders/skybox.frag");
    Shader lightShader  ("shaders/light.vert",   "shaders/light.frag");

    shaderProgram.Activate();
    skyboxShader.Activate();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    Model airplaneModel("assets/Airplane/test.obj");

    Airplane airplane;
    airplane.mass           = 10000.0f;
    airplane.engine.thrust  = 50000.0f;

   airplane.set_inertia(glm::mat3(
            48531.0f, -1320.0f,     0.0f,
            -1320.0f,  256608.0f,    0.0f,
            0.0f,     0.0f,      211333.0f
    ));


    Airfoil NACA_0012(NACA_0012_data);
    Airfoil NACA_2412(NACA_2412_data);

    const float wing_offset = -0.2f;
    const float tail_offset = -6.5f;

    glm::vec3 wing_normal     = Wing::calc_wing_normal(phi::UP,  5.0f);

    glm::vec3 elevator_normal = Wing::calc_wing_normal(phi::UP, -1.0f);

    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset, 0.0f, -2.75f), 6.96f, 2.50f, &NACA_2412, wing_normal, 0.0f));
    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset, 0.0f,  2.75f), 6.96f, 2.50f, &NACA_2412, wing_normal, 0.0f));

    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset - 1.5f, 0.0f, -2.0f), 3.80f, 1.26f, &NACA_0012, wing_normal, 1.0f));
    airplane.elements.push_back(Wing(
        glm::vec3(wing_offset - 1.5f, 0.0f,  2.0f), 3.80f, 1.26f, &NACA_0012, wing_normal, 1.0f));

   airplane.elements.push_back(Wing(
    glm::vec3(tail_offset, 0.0f, 0.0f), 3.50f, 1.80f, &NACA_0012, elevator_normal, 1.0f));

    airplane.elements.push_back(Wing(
        glm::vec3(tail_offset, 0.0f, 0.0f), 5.31f, 3.10f, &NACA_0012, phi::RIGHT, 1.0f));

    airplane.position = glm::vec3(0.0f, 100.0f, 0.0f);
    airplane.velocity = glm::vec3(170.0f, 0.0f, 0.0f);
    airplane.apply_gravity = true;

    //rotate model due to blender
    const glm::quat modelCorrection =
        glm::angleAxis(glm::radians(90.0f), glm::vec3(0,1,0)) *
        glm::angleAxis(glm::radians(90.0f), glm::vec3(0,0,1));


    //Coordinate system: phi X=fwd, Y=up, Z=right  →  GL X=right, Y=up, Z=back
    const glm::quat coordConversion = glm::angleAxis(glm::radians(90.0f), glm::vec3(0,1,0));

    Camera camera(width, height, glm::vec3(0.0f, 115.0f, 60.0f));
    
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
    for (unsigned int i = 0; i < 6; i++) {
        int w, h, ch;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &w, &h, &ch, 0);
        if (data) {
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Failed to load cubemap face: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }

    std::vector<Vertex>  terrainVertices;
    std::vector<GLuint>  terrainIndices;
    const int   gridSize = 1000;
    const float spacing  = 20.0f;

    for (int x = 0; x < gridSize; x++) {
        for (int z = 0; z < gridSize; z++) {
            float y = sinf(x * 0.05f) * cosf(z * 0.05f) * 10.0f;
            terrainVertices.push_back({
                glm::vec3(x * spacing, y, z * spacing),
                glm::vec3(0, 1, 0),
                glm::vec3(0.3f, 0.6f, 0.2f),
                glm::vec2(x * 0.1f, z * 0.1f)
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

         if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            airplane.pitch_control =  1.0f;
         }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            airplane.pitch_control =  -1.0f;   
        else
            airplane.pitch_control =  0.0f;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            airplane.roll_control = -1.0f;    
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            airplane.roll_control =  1.0f;    
        else
            airplane.roll_control =  0.0f;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            airplane.engine.throttle = glm::min(airplane.engine.throttle + 0.01f, 1.0f);
        else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            airplane.engine.throttle = glm::max(airplane.engine.throttle - 0.01f, 0.0f);

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

        glm::vec3 sunDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -0.5f));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightPos"),
                     1, glm::value_ptr(sunDir));
        glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"),
                    1.0f, 1.0f, 0.95f, 1.0f);

        glm::mat4 airplaneMat = glm::mat4(1.0f);
        airplaneMat = glm::translate(airplaneMat, renderPos);
        airplaneMat = airplaneMat * glm::mat4_cast(renderRot);
        airplaneMat = glm::scale(airplaneMat, glm::vec3(0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"),
                           1, GL_FALSE, glm::value_ptr(airplaneMat));
        airplaneModel.Draw(shaderProgram, camera);

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

        shaderProgram.Activate();
        camera.Matrix(shaderProgram, "camMatrix");

        float tX = roundf(renderPos.x / (gridSize * spacing)) * (gridSize * spacing);
        float tZ = roundf(renderPos.z / (gridSize * spacing)) * (gridSize * spacing);
        float tY = 0.0f;

        glm::mat4 terrainMat = glm::translate(glm::mat4(1.0f),
        glm::vec3(tX - gridSize * spacing * 0.5f, tY, tZ - gridSize * spacing * 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"),
                           1, GL_FALSE, glm::value_ptr(terrainMat));
    
        terrainMesh.Draw(shaderProgram, camera);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"),
                           1, GL_FALSE, glm::value_ptr(airplaneMat));
        airplaneModel.Draw(shaderProgram, camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shaderProgram.Delete();
    lightShader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}