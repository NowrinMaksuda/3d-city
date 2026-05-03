#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "basic_camera.h"

#include <iostream>

using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void drawCube(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float tx, float ty, float tz,
    float sx, float sy, float sz,
    float r, float g, float b);

void drawGround(Shader& ourShader, unsigned int VAO, unsigned int VBO);
void drawRoad(Shader& ourShader, unsigned int VAO, unsigned int VBO);
void drawLaneMarkings(Shader& ourShader, unsigned int VAO, unsigned int VBO);
void drawBuilding(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float posX, float posY, float posZ,
    float scX, float scY, float scZ,
    float r, float g, float b);
void drawWindows(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float bx, float by, float bz,
    float bsx, float bsy, int numRows);
void drawTree(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float posX, float posY, float posZ);
void drawTrafficLight(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float posX, float posY, float posZ);


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float rotateAngle_X = 0.0f;
float rotateAngle_Y = 0.0f;
float rotateAngle_Z = 0.0f;
float rotateAxis_X = 0.0f;
float rotateAxis_Y = 0.0f;
float rotateAxis_Z = 1.0f;
float translate_X = 0.0f;
float translate_Y = 0.0f;
float translate_Z = 0.0f;
float scale_X = 1.0f;
float scale_Y = 1.0f;
float scale_Z = 1.0f;

Camera camera(glm::vec3(0.0f, 1.5f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;


float eyeX = 0.0f, eyeY = 3.5f, eyeZ = 10.0f;
float lookAtX = 0.0f, lookAtY = 1.0f, lookAtZ = 0.0f;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);

float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "CSE 4208: Computer Graphics Laboratory", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader ourShader("vertexShader.vs", "fragmentShader.fs");

    
   
    float cube_vertices[] = {
       
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,

        0.5f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.5f, 0.0f, 0.5f,  0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.5f,  0.0f, 0.0f, 1.0f,
        0.5f, 0.0f, 0.5f,  0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 0.5f,  1.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.5f,  1.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f,  1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f,  0.0f, 1.0f, 1.0f,
        0.0f, 0.5f, 0.0f,  0.0f, 1.0f, 1.0f,
        0.0f, 0.5f, 0.5f,  0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
        0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
        0.5f, 0.0f, 0.5f,  1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.5f,  1.0f, 0.0f, 1.0f
    };
    unsigned int cube_indices[] = {
        0, 3, 2,   2, 1, 0,
        4, 5, 7,   7, 6, 4,
        8, 9,10,  10,11, 8,
       12,13,14,  14,15,12,
       16,17,18,  18,19,16,
       20,21,22,  22,23,20
    };
   
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

   
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

       
        glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        glm::mat4 view = basic_camera.createViewMatrix();
        ourShader.setMat4("view", view);

        
        drawGround(ourShader, VAO, VBO);

        
        drawRoad(ourShader, VAO, VBO);

        
        drawLaneMarkings(ourShader, VAO, VBO);


        
        drawBuilding(ourShader, VAO, VBO,
            -4.5f, -0.45f, -8.5f,
            2.0f, 5.0f, 1.8f,
            0.72f, 0.28f, 0.20f);    
        drawWindows(ourShader, VAO, VBO,
            -4.5f, -0.45f, -8.5f,
            2.0f, 5.0f, 3);

        
        drawBuilding(ourShader, VAO, VBO,
            -4.5f, -0.45f, -5.0f,
            2.0f, 8.0f, 1.8f,
            0.85f, 0.75f, 0.38f);     
        drawWindows(ourShader, VAO, VBO,
            -4.5f, -0.45f, -5.0f,
            2.0f, 8.0f, 4);

        
        drawBuilding(ourShader, VAO, VBO,
            -4.5f, -0.45f, -1.5f,
            2.0f, 3.5f, 1.8f,
            0.32f, 0.45f, 0.68f);    
        drawWindows(ourShader, VAO, VBO,
            -4.5f, -0.45f, -1.5f,
            2.0f, 3.5f, 2);

        drawBuilding(ourShader, VAO, VBO,
            2.5f, -0.45f, -8.5f,
            2.0f, 9.0f, 1.8f,
            0.42f, 0.55f, 0.32f);     
        drawWindows(ourShader, VAO, VBO,
            2.5f, -0.45f, -8.5f,
            2.0f, 9.0f, 4);

        
        drawBuilding(ourShader, VAO, VBO,
            2.5f, -0.45f, -5.0f,
            2.0f, 3.5f, 1.8f,
            0.80f, 0.42f, 0.22f);    
        drawWindows(ourShader, VAO, VBO,
            2.5f, -0.45f, -5.0f,
            2.0f, 3.5f, 2);

        
        drawBuilding(ourShader, VAO, VBO,
            2.5f, -0.45f, -1.5f,
            2.0f, 6.0f, 1.8f,
            0.25f, 0.55f, 0.58f);     
        drawWindows(ourShader, VAO, VBO,
            2.5f, -0.45f, -1.5f,
            2.0f, 6.0f, 3);


       
        drawTree(ourShader, VAO, VBO, -2.1f, -0.45f, -7.5f);
        drawTree(ourShader, VAO, VBO, -2.1f, -0.45f, -4.0f);
        drawTree(ourShader, VAO, VBO, -2.1f, -0.45f, -0.8f);

        drawTree(ourShader, VAO, VBO, 1.3f, -0.45f, -7.5f);
        drawTree(ourShader, VAO, VBO, 1.3f, -0.45f, -4.0f);
        drawTree(ourShader, VAO, VBO, 1.3f, -0.45f, -0.8f);
        drawTrafficLight(ourShader, VAO, VBO, -1.3f, -0.45f, -5.0f);  
        drawTrafficLight(ourShader, VAO, VBO, 0.9f, -0.45f, -7.5f);  


        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}

void drawCube(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float tx, float ty, float tz,
    float sx, float sy, float sz,
    float r, float g, float b)
{
    
    float colorPatch[24 * 6];

    
    float positions[] = {
        0.0f,0.0f,0.0f,  0.5f,0.0f,0.0f,  0.5f,0.5f,0.0f,  0.0f,0.5f,0.0f,  
        0.5f,0.0f,0.0f,  0.5f,0.5f,0.0f,  0.5f,0.0f,0.5f,  0.5f,0.5f,0.5f,  
        0.0f,0.0f,0.5f,  0.5f,0.0f,0.5f,  0.5f,0.5f,0.5f,  0.0f,0.5f,0.5f,  
        0.0f,0.0f,0.5f,  0.0f,0.5f,0.5f,  0.0f,0.5f,0.0f,  0.0f,0.0f,0.0f,  
        0.5f,0.5f,0.5f,  0.5f,0.5f,0.0f,  0.0f,0.5f,0.0f,  0.0f,0.5f,0.5f,  
        0.0f,0.0f,0.0f,  0.5f,0.0f,0.0f,  0.5f,0.0f,0.5f,  0.0f,0.0f,0.5f   
    };

    for (int i = 0; i < 24; i++)
    {
        colorPatch[i * 6 + 0] = positions[i * 3 + 0]; 
        colorPatch[i * 6 + 1] = positions[i * 3 + 1];  
        colorPatch[i * 6 + 2] = positions[i * 3 + 2];  
        colorPatch[i * 6 + 3] = r;                      
        colorPatch[i * 6 + 4] = g;                      
        colorPatch[i * 6 + 5] = b;                      
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colorPatch), colorPatch);

    
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;

    translateMatrix = glm::translate(identityMatrix, glm::vec3(tx, ty, tz));
    rotateXMatrix = glm::rotate(identityMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(identityMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(identityMatrix, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(sx, sy, sz));
    model = translateMatrix * rotateXMatrix * rotateYMatrix * rotateZMatrix * scaleMatrix;

    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); 
}

void drawGround(Shader& ourShader, unsigned int VAO, unsigned int VBO)
{
    drawCube(ourShader, VAO, VBO,
        -8.0f, -0.50f, -11.0f,
        24.0f, 0.10f, 26.0f,
        0.55f, 0.57f, 0.55f);  
}
void drawRoad(Shader& ourShader, unsigned int VAO, unsigned int VBO)
{
    drawCube(ourShader, VAO, VBO,
        -1.0f, -0.44f, -11.0f,
        2.0f, 0.06f, 26.0f,
        0.10f, 0.10f, 0.10f);   
}


void drawLaneMarkings(Shader& ourShader, unsigned int VAO, unsigned int VBO)
{
    float mx = -0.12f;   
    float my = -0.41f;   
    float msx = 0.24f;
    float msy = 0.02f;
    float msz = 0.45f;

    float zPositions[] = { -10.0f, -8.5f, -7.0f, -5.5f, -4.0f, -2.5f, -1.0f, 0.5f };
    for (int i = 0; i < 8; i++)
    {
        drawCube(ourShader, VAO, VBO,
            mx, my, zPositions[i],
            msx, msy, msz,
            1.0f, 1.0f, 1.0f); 
    }
}



void drawBuilding(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float posX, float posY, float posZ,
    float scX, float scY, float scZ,
    float r, float g, float b)
{
    drawCube(ourShader, VAO, VBO,
        posX, posY, posZ,
        scX, scY, scZ,
        r, g, b);
}

void drawWindows(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float bx, float by, float bz,
    float bsx, float bsy, int numRows)
{
    float actualW = bsx * 0.5f;   
    float actualH = bsy * 0.5f;   

    float wsx = 0.42f;   
    float wsy = 0.30f;   
    float wsz = 0.06f;  

   
    float wr = 0.20f, wg = 0.60f, wb = 1.00f;

   
    float wz = bz - 0.03f;

   
    float col1 = bx + actualW * 0.08f;
    float col2 = bx + actualW * 0.52f;

    
    float marginBottom = actualH * 0.10f;
    float marginTop = actualH * 0.05f;
    float usableH = actualH - marginBottom - marginTop - wsy * 0.5f;

    for (int row = 0; row < numRows; row++)
    {
        float rowY;
        if (numRows == 1)
            rowY = by + actualH * 0.40f;
        else
            rowY = by + marginBottom + (usableH / (numRows - 1)) * row;

        
        drawCube(ourShader, VAO, VBO, col1, rowY, wz, wsx, wsy, wsz, wr, wg, wb);
        drawCube(ourShader, VAO, VBO, col2, rowY, wz, wsx, wsy, wsz, wr, wg, wb);
    }
}



void drawTree(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float posX, float posY, float posZ)
{
    
    float tW = 0.22f, tH = 1.00f, tD = 0.22f;
   
    float leaf1W = 1.00f;
    float leaf1H = 0.90f;
    float leaf1D = 1.00f;

   
    float offX = (leaf1W * 0.5f - tW * 0.5f) * 0.5f;
    float offZ = (leaf1D * 0.5f - tD * 0.5f) * 0.5f;

    drawCube(ourShader, VAO, VBO,
        posX + offX, posY, posZ + offZ,
        tW, tH, tD,
        0.45f, 0.25f, 0.05f);  

   
    float leaf1Y = posY + tH * 0.5f;
    drawCube(ourShader, VAO, VBO,
        posX, leaf1Y, posZ,
        leaf1W, leaf1H, leaf1D,
        0.10f, 0.60f, 0.10f);  

   
    float leaf2W = 0.65f, leaf2H = 0.65f, leaf2D = 0.65f;
    float leaf2OffX = (leaf1W - leaf2W) * 0.5f * 0.5f;
    float leaf2OffZ = (leaf1D - leaf2D) * 0.5f * 0.5f;
    float leaf2Y = leaf1Y + leaf1H * 0.5f;
    drawCube(ourShader, VAO, VBO,
        posX + leaf2OffX, leaf2Y, posZ + leaf2OffZ,
        leaf2W, leaf2H, leaf2D,
        0.10f, 0.60f, 0.10f);   
}



void drawTrafficLight(Shader& ourShader, unsigned int VAO, unsigned int VBO,
    float posX, float posY, float posZ)
{
  
    float poleW = 0.14f, poleH = 2.8f, poleD = 0.14f;
    drawCube(ourShader, VAO, VBO,
        posX, posY, posZ,
        poleW, poleH, poleD,
        0.08f, 0.08f, 0.08f);  

    
    float lw = 0.32f, lh = 0.26f, ld = 0.32f;

    
    float lOffX = -(lw * 0.5f - poleW * 0.5f) * 0.5f;
    float lOffZ = -(ld * 0.5f - poleD * 0.5f) * 0.5f;

    
    float poleTop = posY + poleH * 0.5f;      
    float redY = poleTop - lh * 0.5f - 0.05f;
    float yellowY = redY - lh * 0.5f - 0.20f;
    float greenY = yellowY - lh * 0.5f - 0.20f;
    drawCube(ourShader, VAO, VBO,
        posX + lOffX, redY, posZ + lOffZ,
        lw, lh, ld,
        0.95f, 0.05f, 0.05f);   
    drawCube(ourShader, VAO, VBO,
        posX + lOffX, yellowY, posZ + lOffZ,
        lw, lh, ld,
        1.00f, 0.85f, 0.00f);  
    drawCube(ourShader, VAO, VBO,
        posX + lOffX, greenY, posZ + lOffZ,
        lw, lh, ld,
        0.05f, 0.85f, 0.15f);   
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (rotateAxis_X) rotateAngle_X -= 1;
        else if (rotateAxis_Y) rotateAngle_Y -= 1;
        else rotateAngle_Z -= 1;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) translate_Y += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) translate_Y -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) translate_X += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) translate_X -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) translate_Z += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) translate_Z -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) scale_X += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) scale_X -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) scale_Y += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) scale_Y -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) scale_Z += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) scale_Z -= 0.01f;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        rotateAngle_X += 1;
        rotateAxis_X = 1.0f; rotateAxis_Y = 0.0f; rotateAxis_Z = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        rotateAngle_Y += 1;
        rotateAxis_X = 0.0f; rotateAxis_Y = 1.0f; rotateAxis_Z = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        rotateAngle_Z += 1;
        rotateAxis_X = 0.0f; rotateAxis_Y = 0.0f; rotateAxis_Z = 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        eyeX += 2.5f * deltaTime; basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        eyeX -= 2.5f * deltaTime; basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        eyeZ += 2.5f * deltaTime; basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        eyeZ -= 2.5f * deltaTime; basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyeY += 2.5f * deltaTime; basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        eyeY -= 2.5f * deltaTime; basic_camera.changeEye(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        lookAtX += 2.5f * deltaTime; basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        lookAtX -= 2.5f * deltaTime; basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        lookAtY += 2.5f * deltaTime; basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        lookAtY -= 2.5f * deltaTime; basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        lookAtZ += 2.5f * deltaTime; basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
    {
        lookAtZ -= 2.5f * deltaTime; basic_camera.changeLookAt(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        basic_camera.changeViewUpVector(glm::vec3(1.0f, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        basic_camera.changeViewUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
        basic_camera.changeViewUpVector(glm::vec3(0.0f, 0.0f, 1.0f));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
