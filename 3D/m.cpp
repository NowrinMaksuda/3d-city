#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"
#include <iostream>
#include <cmath>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ============================================================
//  CONTROLS GUIDE:
//  WASD        - Free camera movement (mode 1)
//  Mouse       - Free camera look (mode 1)
//  Arrow UP/DN - Player car accelerate / brake + reverse
//  Arrow LT/RT - Player car steer left / right
//  1           - Free camera mode (WASD + mouse)
//  2           - Top-down bird's eye view
//  3           - Follow-car (chase cam behind player car)
//  4           - Side-view
//  ESC         - Quit
// ============================================================

// --- VAO/VBO globals ---
unsigned int cubeVAO = 0, cubeVBO = 0;

// --- Screen ---
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 800;

// --- Camera ---
Camera camera(glm::vec3(0.0f, 18.0f, 65.0f));
float lastX = SCR_WIDTH  / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;
float deltaTime  = 0.0f;
float lastFrame  = 0.0f;

enum CameraMode { FREE_CAM = 0, TOP_DOWN, FOLLOW_CAR, SIDE_VIEW };
CameraMode camMode = FREE_CAM;

// --- Player Car ---
float playerZ     = 0.0f;
float playerX     = -4.5f;
float playerSpeed = 0.0f;          // current forward speed (units/sec)
const float MAX_SPEED     = 70.0f;
const float ACCEL         = 40.0f;
const float BRAKE_DECEL   = 60.0f;
const float NATURAL_DECEL = 0.97f;
const float STEER_SPEED   = 16.0f;
const float LANE_LEFT     = -9.5f;
const float LANE_RIGHT    =  9.5f;

// --- AI Cars ---
struct AICar {
    float z, x, speed;
    glm::vec3 color;
};
AICar aiCars[] = {
    { -150.0f, 4.5f,  22.0f, {0.0f, 0.25f, 0.85f} },
    {  -80.0f, 4.5f,  30.0f, {0.0f, 0.65f, 0.15f} },
    {   20.0f, -4.5f, 18.0f, {0.85f, 0.55f, 0.0f} },
};
const int NUM_AI = 3;

// --- Traffic light ---
float trafficTimer = 0.0f;
int   trafficState = 0; // 0=red, 1=yellow, 2=green
const float TRAFFIC_DURATIONS[] = { 4.0f, 1.2f, 4.0f };

// --- One-shot key helpers ---
bool prevKey1 = false, prevKey2 = false, prevKey3 = false, prevKey4 = false;

// ============================================================
//  PROTOTYPES
// ============================================================
void     processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void     renderCube();

// Draw helpers — all geometry is cubes only
void setModel(Shader& s, glm::vec3 center, glm::vec3 halfExtents,
              float rotY = 0.0f, glm::vec3 pivot = glm::vec3(0.0f));
void drawBox(Shader& s, glm::vec3 center, glm::vec3 halfExtents,
             glm::vec3 color, float rotY = 0.0f);

void drawGround     (Shader& s);
void drawRoad       (Shader& s, unsigned int roadTex);
void drawBuilding   (Shader& s, glm::vec3 origin, float w, float totalH, float d,
                     glm::vec3 wallColor, int floors);
void drawTree       (Shader& s, glm::vec3 root, unsigned int leafTex);
void drawTrafficLight(Shader& s, glm::vec3 base, int state);
void drawCar        (Shader& s, glm::vec3 base, glm::vec3 bodyColor, bool isPlayer);
void drawHUD        (Shader& s);   // on-screen speed bar (cube strip)

// ============================================================
//  MOUSE CALLBACK (lambda stored at file scope via function)
// ============================================================
void mouseCallback(GLFWwindow* /*w*/, double xpos, double ypos) {
    if (camMode != FREE_CAM) { firstMouse = true; return; }
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float dx =  (float)xpos - lastX;
    float dy =  lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;
    camera.ProcessMouseMovement(dx, dy);
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    // --- Init GLFW ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "3D City | Arrows=Drive  1-4=Camera  ESC=Quit", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("vertexShader.vs", "fragmentShader.fs");

    unsigned int roadTex = loadTexture("textures/rastar texture.jpg");
    unsigned int leafTex = loadTexture("textures/images.jpg");

    // ========================================================
    //  MAIN LOOP
    // ========================================================
    while (!glfwWindowShouldClose(window)) {
        float now   = (float)glfwGetTime();
        deltaTime   = now - lastFrame;
        lastFrame   = now;

        processInput(window);

        // --- Traffic light cycle ---
        trafficTimer += deltaTime;
        if (trafficTimer >= TRAFFIC_DURATIONS[trafficState]) {
            trafficTimer = 0.0f;
            trafficState = (trafficState + 1) % 3;
        }

        // --- AI cars loop around ---
        for (int i = 0; i < NUM_AI; i++) {
            aiCars[i].z -= aiCars[i].speed * deltaTime;
            if (aiCars[i].z < -310.0f) aiCars[i].z = 310.0f;
        }

        // --- Player car physics ---
        playerZ -= playerSpeed * deltaTime;
        if (playerZ < -310.0f) playerZ =  310.0f;
        if (playerZ >  310.0f) playerZ = -310.0f;
        playerX = glm::clamp(playerX, LANE_LEFT, LANE_RIGHT);

        // --- Clear ---
        glClearColor(0.45f, 0.72f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        // --- Projection ---
        glm::mat4 proj = glm::perspective(
            glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 1200.0f);
        shader.setMat4("projection", proj);

        // --- View (camera modes) ---
        glm::mat4 view;
        switch (camMode) {
            case TOP_DOWN:
                view = glm::lookAt(
                    glm::vec3(playerX, 90.0f, playerZ),
                    glm::vec3(playerX,  0.0f, playerZ),
                    glm::vec3(0, 0, -1));
                break;
            case FOLLOW_CAR:
                view = glm::lookAt(
                    glm::vec3(playerX,  9.0f, playerZ + 22.0f),
                    glm::vec3(playerX,  2.5f, playerZ -  5.0f),
                    glm::vec3(0, 1, 0));
                break;
            case SIDE_VIEW:
                view = glm::lookAt(
                    glm::vec3(90.0f, 18.0f, playerZ),
                    glm::vec3( 0.0f,  4.0f, playerZ),
                    glm::vec3(0, 1, 0));
                break;
            default:
                view = camera.GetViewMatrix();
                break;
        }
        shader.setMat4("view", view);

        // =============================================
        //  SCENE
        // =============================================

        // 1. Ground (grass)
        drawGround(shader);

        // 2. Road + markings + sidewalks
        drawRoad(shader, roadTex);

        // 3. Buildings — left side (x < 0) and right side (x > 0)
        struct BSpec { float x, z, w, h, d; glm::vec3 col; int floors; };
        BSpec bspecs[] = {
            // Left side
            {-46, -200, 15, 32, 14, {0.80f,0.28f,0.20f}, 5},
            {-46, -148, 13, 22, 13, {0.20f,0.48f,0.82f}, 3},
            {-46,  -96, 16, 42, 15, {0.72f,0.60f,0.18f}, 6},
            {-46,  -44, 12, 26, 12, {0.38f,0.38f,0.62f}, 4},
            {-46,    8, 14, 18, 14, {0.18f,0.70f,0.38f}, 2},
            {-46,   60, 11, 36, 11, {0.60f,0.28f,0.52f}, 5},
            {-46,  112, 15, 28, 13, {0.80f,0.50f,0.18f}, 4},
            // Right side
            { 46, -200, 14, 26, 14, {0.30f,0.54f,0.74f}, 4},
            { 46, -148, 12, 38, 12, {0.74f,0.22f,0.32f}, 6},
            { 46,  -96, 16, 20, 16, {0.42f,0.62f,0.42f}, 3},
            { 46,  -44, 13, 44, 13, {0.52f,0.50f,0.80f}, 7},
            { 46,    8, 15, 30, 15, {0.80f,0.40f,0.18f}, 4},
            { 46,   60, 11, 24, 11, {0.22f,0.64f,0.72f}, 3},
            { 46,  112, 14, 34, 14, {0.78f,0.30f,0.56f}, 5},
        };
        for (auto& b : bspecs)
            drawBuilding(shader,
                glm::vec3(b.x, 0.0f, b.z),
                b.w, b.h, b.d, b.col, b.floors);

        // 4. Trees along sidewalks
        for (int i = 0; i < 18; i++) {
            float zp = -250.0f + i * 30.0f;
            drawTree(shader, glm::vec3(-28.0f, 0.0f, zp), leafTex);
            drawTree(shader, glm::vec3( 28.0f, 0.0f, zp), leafTex);
        }

        // 5. Traffic lights
        drawTrafficLight(shader, glm::vec3(-19.0f, 0.0f, -35.0f), trafficState);
        drawTrafficLight(shader, glm::vec3( 19.0f, 0.0f, -35.0f), trafficState);
        drawTrafficLight(shader, glm::vec3(-19.0f, 0.0f,  35.0f), trafficState);
        drawTrafficLight(shader, glm::vec3( 19.0f, 0.0f,  35.0f), trafficState);

        // 6. AI cars
        shader.setBool("useTexture", false);
        for (int i = 0; i < NUM_AI; i++)
            drawCar(shader,
                glm::vec3(aiCars[i].x, 0.0f, aiCars[i].z),
                aiCars[i].color, false);

        // 7. Player car (red)
        drawCar(shader,
            glm::vec3(playerX, 0.0f, playerZ),
            glm::vec3(0.95f, 0.08f, 0.08f), true);

        // 8. Speed HUD (small bar in front of camera)
        drawHUD(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ============================================================
//  PROCESS INPUT
// ============================================================
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera mode switches (one-shot on press)
    auto oneShot = [&](int key, bool& prev, CameraMode mode) {
        bool cur = (glfwGetKey(window, key) == GLFW_PRESS);
        if (cur && !prev) camMode = mode;
        prev = cur;
    };
    oneShot(GLFW_KEY_1, prevKey1, FREE_CAM);
    oneShot(GLFW_KEY_2, prevKey2, TOP_DOWN);
    oneShot(GLFW_KEY_3, prevKey3, FOLLOW_CAR);
    oneShot(GLFW_KEY_4, prevKey4, SIDE_VIEW);

    // Free camera — WASD
    if (camMode == FREE_CAM) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);
    }

    // Player car — Arrow keys
    bool upHeld   = glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS;
    bool downHeld = glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS;
    bool leftHeld = glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS;
    bool rightHeld= glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

    if (upHeld)
        playerSpeed = glm::min(playerSpeed + ACCEL * deltaTime, MAX_SPEED);
    else if (downHeld)
        playerSpeed = glm::max(playerSpeed - BRAKE_DECEL * deltaTime, -MAX_SPEED * 0.4f);
    else
        playerSpeed *= NATURAL_DECEL;  // coast to stop

    if (leftHeld)  playerX -= STEER_SPEED * deltaTime;
    if (rightHeld) playerX += STEER_SPEED * deltaTime;
}

// ============================================================
//  HELPERS
// ============================================================

// Set model matrix: center is world-space center of the box,
// halfExtents is the (x,y,z) half-sizes.
void setModel(Shader& s, glm::vec3 center, glm::vec3 half, float rotY, glm::vec3 /*pivot*/) {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, center);
    if (rotY != 0.0f) m = glm::rotate(m, rotY, glm::vec3(0,1,0));
    m = glm::scale(m, half);
    s.setMat4("model", m);
}

void drawBox(Shader& s, glm::vec3 center, glm::vec3 half, glm::vec3 color, float rotY) {
    s.setBool("useTexture", false);
    s.setVec3("uColor", color);
    setModel(s, center, half, rotY);
    renderCube();
}

// ============================================================
//  GROUND
// ============================================================
void drawGround(Shader& s) {
    drawBox(s, {0,-1.6f,0}, {250,0.6f,500}, {0.22f,0.55f,0.14f});
}

// ============================================================
//  ROAD
// ============================================================
void drawRoad(Shader& s, unsigned int roadTex) {
    // Tarmac
    s.setBool("useTexture", true);
    s.setBool("isRoad", true);
    glBindTexture(GL_TEXTURE_2D, roadTex);
    s.setVec3("uColor", {1,1,1});
    setModel(s, {0,-0.5f,0}, {13.5f,0.12f,500.0f});
    renderCube();
    s.setBool("isRoad", false);
    s.setBool("useTexture", false);

    // Sidewalks
    glm::vec3 swCol{0.74f, 0.72f, 0.68f};
    drawBox(s, {-18.5f,-0.3f,0}, {4.5f,0.22f,500}, swCol);
    drawBox(s, { 18.5f,-0.3f,0}, {4.5f,0.22f,500}, swCol);

    // Curbs (raised edge)
    glm::vec3 curbCol{0.52f,0.52f,0.52f};
    drawBox(s, {-14.0f,-0.15f,0}, {0.4f,0.38f,500}, curbCol);
    drawBox(s, { 14.0f,-0.15f,0}, {0.4f,0.38f,500}, curbCol);

    // Center divider (solid yellow)
    drawBox(s, {0,-0.36f,0}, {0.22f,0.02f,500}, {0.95f,0.90f,0.0f});

    // Left-lane dash  (white dashes between lanes)
    for (int i = -50; i <= 50; i++) {
        if (i % 2 == 0) {
            float z = i * 9.5f;
            drawBox(s, {-4.5f,-0.36f,z}, {0.15f,0.02f,3.8f}, {1,1,1});
            drawBox(s, { 4.5f,-0.36f,z}, {0.15f,0.02f,3.8f}, {1,1,1});
        }
    }

    // Pedestrian crossing near traffic lights
    for (float tz : {-42.0f, 42.0f}) {
        for (int strip = -6; strip <= 6; strip += 2) {
            drawBox(s, {(float)strip * 1.0f, -0.36f, tz},
                    {0.7f, 0.02f, 1.8f}, {0.95f,0.95f,0.95f});
        }
    }
}

// ============================================================
//  BUILDING
// ============================================================
void drawBuilding(Shader& s, glm::vec3 origin, float w, float totalH, float d,
                  glm::vec3 wallCol, int floors) {
    float hw = w * 0.5f, hh = totalH * 0.5f, hd = d * 0.5f;
    glm::vec3 center = origin + glm::vec3(0, hh, 0);

    // Main body
    drawBox(s, center, {hw, hh, hd}, wallCol);

    // Darker base strip
    drawBox(s, origin + glm::vec3(0, 0.6f, 0), {hw + 0.05f, 0.6f, hd + 0.05f},
            wallCol * 0.7f);

    // Floor-line grooves
    float fh = totalH / floors;
    for (int f = 1; f < floors; f++) {
        float ly = f * fh;
        drawBox(s, origin + glm::vec3(0, ly, 0),
                {hw + 0.05f, 0.08f, hd + 0.05f}, wallCol * 0.65f);
    }

    // Windows on front (+z face) and back (-z face)
    int wCols = (int)(w / 3.2f);
    float wSpacingX = w / (wCols + 1);
    glm::vec3 winCol{0.55f, 0.80f, 0.98f};
    for (int f = 0; f < floors; f++) {
        float wy = f * fh + fh * 0.45f;
        for (int c = 1; c <= wCols; c++) {
            float wx = origin.x - hw + wSpacingX * c;
            // front face
            drawBox(s, {wx, origin.y + wy, origin.z + hd + 0.06f},
                    {0.55f, fh * 0.28f, 0.05f}, winCol);
            // back face
            drawBox(s, {wx, origin.y + wy, origin.z - hd - 0.06f},
                    {0.55f, fh * 0.28f, 0.05f}, winCol);
        }
    }
    // Windows on left/right faces
    int wColsD = (int)(d / 3.2f);
    float wSpacingZ = d / (wColsD + 1);
    for (int f = 0; f < floors; f++) {
        float wy = f * fh + fh * 0.45f;
        for (int c = 1; c <= wColsD; c++) {
            float wz = origin.z - hd + wSpacingZ * c;
            drawBox(s, {origin.x + hw + 0.06f, origin.y + wy, wz},
                    {0.05f, fh * 0.28f, 0.55f}, winCol);
            drawBox(s, {origin.x - hw - 0.06f, origin.y + wy, wz},
                    {0.05f, fh * 0.28f, 0.55f}, winCol);
        }
    }

    // Rooftop parapet
    glm::vec3 topCenter = origin + glm::vec3(0, totalH + 0.5f, 0);
    drawBox(s, topCenter, {hw * 0.95f, 0.5f, hd * 0.95f}, wallCol * 0.60f);

    // Rooftop water tower / antenna
    drawBox(s, topCenter + glm::vec3(0, 2.5f, 0), {0.22f, 2.0f, 0.22f},
            {0.35f, 0.35f, 0.35f});
    drawBox(s, topCenter + glm::vec3(0, 5.5f, 0), {0.08f, 1.0f, 0.08f},
            {0.55f, 0.55f, 0.55f});

    // Rooftop AC units
    for (float rx : {hw * 0.4f, -hw * 0.4f}) {
        drawBox(s, topCenter + glm::vec3(rx, 1.1f, hd * 0.35f),
                {0.8f, 0.6f, 0.8f}, wallCol * 0.50f);
    }
}

// ============================================================
//  TREE  (all cubes)
// ============================================================
void drawTree(Shader& s, glm::vec3 root, unsigned int leafTex) {
    s.setBool("useTexture", false);
    // Trunk
    drawBox(s, root + glm::vec3(0, 2.2f, 0), {0.45f, 2.2f, 0.45f},
            {0.36f, 0.20f, 0.07f});
    // Layered cube canopy
    s.setBool("useTexture", true);
    glBindTexture(GL_TEXTURE_2D, leafTex);
    float sizes[] = { 2.8f, 2.2f, 1.5f };
    float yoffs[] = { 5.0f, 7.4f, 9.2f };
    glm::vec3 cols[] = {
        {0.12f, 0.68f, 0.12f},
        {0.09f, 0.58f, 0.09f},
        {0.06f, 0.48f, 0.06f}
    };
    for (int j = 0; j < 3; j++) {
        s.setVec3("uColor", cols[j]);
        setModel(s, root + glm::vec3(0, yoffs[j], 0),
                 {sizes[j], 1.45f, sizes[j]});
        renderCube();
    }
    s.setBool("useTexture", false);
}

// ============================================================
//  TRAFFIC LIGHT
// ============================================================
void drawTrafficLight(Shader& s, glm::vec3 base, int state) {
    // Pole
    drawBox(s, base + glm::vec3(0, 8.0f, 0),  {0.28f, 8.0f, 0.28f}, {0.28f,0.28f,0.28f});
    // Arm
    drawBox(s, base + glm::vec3(0, 16.5f, 0), {0.28f, 0.28f, 2.5f}, {0.28f,0.28f,0.28f});
    // Housing box
    drawBox(s, base + glm::vec3(0, 17.5f, 0), {0.85f, 4.0f, 0.85f}, {0.06f,0.06f,0.06f});

    // Light colors (on/off)
    glm::vec3 rOn{1.0f,0.05f,0.05f}, rOff{0.30f,0.0f,0.0f};
    glm::vec3 yOn{1.0f,0.95f,0.0f}, yOff{0.30f,0.28f,0.0f};
    glm::vec3 gOn{0.05f,1.0f,0.05f}, gOff{0.0f,0.28f,0.0f};

    drawBox(s, base + glm::vec3(0, 20.5f, 0.75f), {0.5f,0.5f,0.12f}, state==0 ? rOn : rOff);
    drawBox(s, base + glm::vec3(0, 18.6f, 0.75f), {0.5f,0.5f,0.12f}, state==1 ? yOn : yOff);
    drawBox(s, base + glm::vec3(0, 16.7f, 0.75f), {0.5f,0.5f,0.12f}, state==2 ? gOn : gOff);

    // Visor shades above each light
    for (float ly : {20.5f, 18.6f, 16.7f})
        drawBox(s, base + glm::vec3(0, ly + 0.45f, 0.65f), {0.55f,0.1f,0.25f}, {0.04f,0.04f,0.04f});
}

// ============================================================
//  CAR (all cubes, much more detailed)
// ============================================================
void drawCar(Shader& s, glm::vec3 base, glm::vec3 bodyCol, bool /*isPlayer*/) {
    s.setBool("useTexture", false);

    // Convenience: draw a box relative to 'base'
    auto box = [&](glm::vec3 off, glm::vec3 half, glm::vec3 col) {
        drawBox(s, base + off, half, col);
    };

    glm::vec3 dark  = bodyCol * 0.55f;
    glm::vec3 black = {0.05f,0.05f,0.05f};
    glm::vec3 glassCol = {0.15f,0.22f,0.32f};
    glm::vec3 chrome = {0.72f,0.72f,0.72f};
    glm::vec3 wheelCol = {0.08f,0.08f,0.08f};
    glm::vec3 hubCol   = {0.60f,0.60f,0.60f};

    // ---- BODY ----
    // Underbody / floor
    box({0, 0.35f, 0},    {2.1f, 0.35f, 4.8f}, dark);
    // Main body mid section
    box({0, 1.15f, 0},    {2.1f, 0.45f, 4.2f}, bodyCol);
    // Hood (front slope)
    box({0, 0.90f, -3.2f},{2.0f, 0.30f, 1.6f}, bodyCol);
    // Trunk (rear)
    box({0, 0.90f,  3.2f},{2.0f, 0.30f, 1.6f}, bodyCol);
    // Fenders (side bulges)
    box({-2.05f, 0.85f, -2.0f},{0.12f, 0.55f, 2.5f}, bodyCol);
    box({ 2.05f, 0.85f, -2.0f},{0.12f, 0.55f, 2.5f}, bodyCol);
    box({-2.05f, 0.85f,  2.0f},{0.12f, 0.55f, 2.5f}, bodyCol);
    box({ 2.05f, 0.85f,  2.0f},{0.12f, 0.55f, 2.5f}, bodyCol);

    // ---- CABIN ----
    box({0, 2.15f, 0.3f},  {1.85f, 0.72f, 2.3f}, bodyCol);
    // Windshield (front glass)
    box({0, 2.20f,-1.3f},  {1.78f, 0.62f, 0.08f}, glassCol);
    // Rear glass
    box({0, 2.20f, 2.0f},  {1.78f, 0.62f, 0.08f}, glassCol);
    // Side windows (left)
    box({-1.82f, 2.18f, 0.25f},{0.08f, 0.55f, 1.45f}, glassCol);
    // Side windows (right)
    box({ 1.82f, 2.18f, 0.25f},{0.08f, 0.55f, 1.45f}, glassCol);
    // Roof
    box({0, 2.92f, 0.3f},  {1.80f, 0.10f, 2.3f}, dark);
    // A-pillar left
    box({-1.68f, 2.5f,-1.15f},{0.14f, 0.48f, 0.14f}, black);
    // A-pillar right
    box({ 1.68f, 2.5f,-1.15f},{0.14f, 0.48f, 0.14f}, black);

    // ---- BUMPERS ----
    box({0, 0.60f,-4.7f},{2.05f, 0.42f, 0.45f}, black);
    box({0, 0.60f, 4.7f},{2.05f, 0.42f, 0.45f}, black);
    // Chrome strips on bumpers
    box({0, 0.82f,-4.72f},{1.80f, 0.08f, 0.44f}, chrome);
    box({0, 0.82f, 4.72f},{1.80f, 0.08f, 0.44f}, chrome);

    // ---- HEADLIGHTS ----
    box({-1.40f, 1.05f,-4.6f},{0.55f, 0.28f, 0.12f}, {1.0f,1.0f,0.85f});
    box({ 1.40f, 1.05f,-4.6f},{0.55f, 0.28f, 0.12f}, {1.0f,1.0f,0.85f});
    // DRL strip
    box({0, 0.70f,-4.68f},{1.60f, 0.08f, 0.10f}, {0.9f,0.9f,1.0f});
    // Taillights
    box({-1.40f, 1.05f, 4.6f},{0.55f, 0.28f, 0.12f}, {1.0f,0.06f,0.06f});
    box({ 1.40f, 1.05f, 4.6f},{0.55f, 0.28f, 0.12f}, {1.0f,0.06f,0.06f});
    // Tail light bar
    box({0, 1.05f, 4.68f},{1.60f, 0.08f, 0.10f}, {0.80f,0.03f,0.03f});

    // ---- GRILLE ----
    box({0, 0.88f,-4.62f},{1.60f, 0.32f, 0.10f}, black);
    // Grille bars
    for (int gi = -3; gi <= 3; gi++)
        box({gi * 0.4f, 0.88f,-4.65f},{0.05f, 0.28f, 0.08f}, chrome);

    // ---- SIDE MIRRORS ----
    box({-2.24f, 2.05f,-1.2f},{0.28f, 0.18f, 0.42f}, bodyCol);
    box({ 2.24f, 2.05f,-1.2f},{0.28f, 0.18f, 0.42f}, bodyCol);
    // Mirror glass
    box({-2.34f, 2.05f,-1.2f},{0.04f, 0.14f, 0.32f}, glassCol);
    box({ 2.34f, 2.05f,-1.2f},{0.04f, 0.14f, 0.32f}, glassCol);

    // ---- WHEELS (4) ----
    float wxArr[] = {-2.18f,  2.18f};
    float wzArr[] = {-2.90f,  2.90f};
    for (float wx : wxArr) {
        for (float wz : wzArr) {
            // Tyre
            box({wx, 0.52f, wz},{0.42f, 0.85f, 0.92f}, wheelCol);
            // Wheel hub
            float hx = (wx < 0) ? wx - 0.38f : wx + 0.38f;
            box({hx, 0.52f, wz},{0.12f, 0.65f, 0.65f}, hubCol);
            // Spoke cross
            box({hx, 0.52f, wz},{0.10f, 0.55f, 0.14f}, black);
            box({hx, 0.52f, wz},{0.10f, 0.14f, 0.55f}, black);
            // Lug nut center
            box({hx, 0.52f, wz},{0.10f, 0.20f, 0.20f}, chrome);
        }
    }

    // ---- EXHAUST ----
    box({1.50f, 0.28f, 4.78f},{0.22f, 0.16f, 0.12f}, chrome);
    box({-1.50f,0.28f, 4.78f},{0.22f, 0.16f, 0.12f}, chrome);
}

// ============================================================
//  SPEED HUD  — small cube bar at fixed screen-ish position
// ============================================================
void drawHUD(Shader& s) {
    // Place in front of camera in world space for any camera mode
    // It always appears at a fixed world pos near the player car
    float ratio = glm::abs(playerSpeed) / MAX_SPEED;
    int   bars  = (int)(ratio * 12);
    glm::vec3 hudBase = {playerX - 5.0f, 6.0f, playerZ + 18.0f};

    for (int i = 0; i < 12; i++) {
        float t      = i / 11.0f;
        glm::vec3 c  = (i < bars)
            ? glm::mix(glm::vec3(0.1f,0.9f,0.1f), glm::vec3(0.9f,0.1f,0.1f), t)
            : glm::vec3(0.2f,0.2f,0.2f);
        drawBox(s, hudBase + glm::vec3(i * 0.8f, 0, 0), {0.3f,0.3f,0.05f}, c);
    }
}

// ============================================================
//  RENDER CUBE  (unit cube, ±1 on each axis)
// ============================================================
void renderCube() {
    if (cubeVAO == 0) {
        float verts[] = {
            // position          // texcoord
            // back face
            -1,-1,-1, 0,0,   1,-1,-1, 1,0,   1,1,-1, 1,1,
             1,1,-1, 1,1,  -1,1,-1, 0,1,  -1,-1,-1, 0,0,
            // front face
            -1,-1,1, 0,0,   1,-1,1, 1,0,   1,1,1, 1,1,
             1,1,1, 1,1,  -1,1,1, 0,1,  -1,-1,1, 0,0,
            // left face
            -1,1,1, 1,0,  -1,1,-1, 1,1,  -1,-1,-1, 0,1,
            -1,-1,-1, 0,1, -1,-1,1, 0,0, -1,1,1, 1,0,
            // right face
             1,1,1, 1,0,   1,1,-1, 1,1,   1,-1,-1, 0,1,
             1,-1,-1, 0,1,  1,-1,1, 0,0,  1,1,1, 1,0,
            // bottom face
            -1,-1,-1, 0,1,  1,-1,-1, 1,1,  1,-1,1, 1,0,
             1,-1,1, 1,0,  -1,-1,1, 0,0, -1,-1,-1, 0,1,
            // top face
            -1,1,-1, 0,1,   1,1,-1, 1,1,   1,1,1, 1,0,
             1,1,1, 1,0,  -1,1,1, 0,0,  -1,1,-1, 0,1
        };
        glGenVertexArrays(1, &cubeVAO);
        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// ============================================================
//  LOAD TEXTURE
// ============================================================
unsigned int loadTexture(const char* path) {
    unsigned int id;
    glGenTextures(1, &id);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    if (data) {
        GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cerr << "Texture load failed: " << path << "\n";
        stbi_image_free(data);
    }
    return id;
}
