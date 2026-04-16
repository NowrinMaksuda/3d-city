#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Screen dimensions
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 750;

// Camera
Camera camera(glm::vec3(0.0f, 12.0f, 35.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Bus movement
float busZ = -20.0f;

// Texture loading function
unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        cout << "Texture failed at: " << path << endl;
        stbi_image_free(data);
    }
    return textureID;
}

// Cube vertices with Texture Coordinates
float vertices[] = {
    // Positions          // Texture Coords
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

void drawCube(Shader &s, unsigned int vao, glm::vec3 pos, glm::vec3 scale, glm::vec3 color, unsigned int texID, bool useTex) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, scale);
    s.setMat4("model", model);
    s.setVec3("objectColor", color);
    s.setBool("useTexture", useTex);
    
    if(useTex) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D City Final", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    Shader cityShader("vertexShader.vs", "fragmentShader.fs");

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture loading using your files
    unsigned int buildTex = loadTexture("textures/building.jpg");
    unsigned int roadTex = loadTexture("textures/rastar texture.jpg");

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cityShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f);
        cityShader.setMat4("projection", projection);
        cityShader.setMat4("view", camera.GetViewMatrix());

        // 1. ROAD
        drawCube(cityShader, VAO, glm::vec3(0, 0, 0), glm::vec3(10, 0.1, 100), glm::vec3(1.0f), roadTex, true);
        drawCube(cityShader, VAO, glm::vec3(0, 0.05, 0), glm::vec3(100, 0.1, 10), glm::vec3(1.0f), roadTex, true);

        // 2. BUILDINGS
        for(int i=0; i<6; i++) {
            drawCube(cityShader, VAO, glm::vec3(12, 6, -30 + i*15), glm::vec3(8, 12, 8), glm::vec3(1.0f), buildTex, true);
            drawCube(cityShader, VAO, glm::vec3(-12, 6, -30 + i*15), glm::vec3(8, 12, 8), glm::vec3(1.0f), buildTex, true);
        }

        // 3. BUSES
        busZ += deltaTime * 8.0f;
        if(busZ > 40) busZ = -40;
        drawCube(cityShader, VAO, glm::vec3(-2.5, 1.2, busZ), glm::vec3(3, 2, 6), glm::vec3(1.0, 0.9, 0.0), 0, false);
        drawCube(cityShader, VAO, glm::vec3(-2.5, 1.2, busZ+10), glm::vec3(3, 2, 6), glm::vec3(1.0, 0.9, 0.0), 0, false);

        // 4. TREES
        for(int i=0; i<8; i++) {
            glm::vec3 p = glm::vec3(20, 0, -40 + i*10);
            drawCube(cityShader, VAO, p + glm::vec3(0, 2, 0), glm::vec3(0.5, 4, 0.5), glm::vec3(0.4, 0.2, 0.1), 0, false); // Trunk
            drawCube(cityShader, VAO, p + glm::vec3(0, 5, 0), glm::vec3(3, 3, 3), glm::vec3(0.0, 0.5, 0.0), 0, false); // Leaves
        }

        // 5. TRAFFIC LIGHT
        drawCube(cityShader, VAO, glm::vec3(6, 4, 6), glm::vec3(0.3, 8, 0.3), glm::vec3(0.2), 0, false); // Pole
        int lightIdx = ((int)glfwGetTime() % 3);
        glm::vec3 lCol = (lightIdx==0) ? glm::vec3(1,0,0) : (lightIdx==1 ? glm::vec3(1,1,0) : glm::vec3(0,1,0));
        drawCube(cityShader, VAO, glm::vec3(6, 8.5, 6.2), glm::vec3(0.6, 0.6, 0.1), lCol, 0, false); // Light

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}