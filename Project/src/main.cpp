#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include "utils/CustomCamera.h"
#include "Shader.h"
#include "utils/Helper.h"
#include <memory>

//global values
std::shared_ptr<CustomCamera> camera_ptr = std::make_shared<CustomCamera>(
    glm::vec3(-0.2f, -3.5f, 1.66f), // 位置
    glm::vec3(0.0f, 0.0f, 1.0f),  // up向量
    -90.0f,                        // yaw（朝向+Y轴）
    90.0f                          // pitch
    );

float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool b_applyDistortion = false;
bool b_useLighting = false;
bool b_dualLighting = false;

// window size callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// mouse move callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera_ptr->ProcessMouseMovement(xoffset, yoffset);
}

// mouse scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera_ptr->ProcessMouseScroll(yoffset);
}

// process keyboard input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_ptr->ProcessKeyboard(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_ptr->ProcessKeyboard(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_ptr->ProcessKeyboard(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_ptr->ProcessKeyboard(GLFW_KEY_D, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_ptr->ProcessKeyboard(GLFW_KEY_SPACE, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera_ptr->ProcessKeyboard(GLFW_KEY_LEFT_SHIFT, deltaTime);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW Init Failed!!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    GLFWwindow* window = glfwCreateWindow(1200, 800, "VR Scene", NULL, NULL);
    if (window == NULL) {
        std::cerr << "CreateWindow Failed!!!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD Loaded Failed!!" << std::endl;
        return -1;
    }

    // enable depth test and multisample
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    // create shader
    unsigned int sceneShader = createShaderProgram(sceneVertexShader, sceneFragmentShader);
    unsigned int distortionShader = createShaderProgram(distortionVertexShader, distortionFragmentShader);

    // create ring  screen
    std::vector<float> ringVertices;
    std::vector<unsigned int> ringIndices;
    createRingScreenWithBezier(72, 72, ringVertices, ringIndices);
    
    unsigned int ringVAO, ringVBO, ringEBO;
    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);
    glGenBuffers(1, &ringEBO);

    glBindVertexArray(ringVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, ringVertices.size() * sizeof(float), ringVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ringEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ringIndices.size() * sizeof(unsigned int), ringIndices.data(), GL_STATIC_DRAW);

    // pos attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // nor attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // tex attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // generate dynamicTexture
    unsigned int dynamicTexture;
    glGenTextures(1, &dynamicTexture);
    glBindTexture(GL_TEXTURE_2D, dynamicTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 768, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // create (FBO)
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // create color attachments
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, 800, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Creating Render Buffer Objects (Depth and Template Attachments)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1200, 800);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Checking frame buffer integrity
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Frame buffer is un integrity!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Creating full-screen quads
    unsigned int quadVAO, quadVBO;
    createQuad(quadVAO, quadVBO);

    // Set shader uniform position
    glUseProgram(sceneShader);
    glUniform1i(glGetUniformLocation(sceneShader, "screenTexture"), 0);
    glUniform1i(glGetUniformLocation(sceneShader, "u_b_useLighting"), b_useLighting);
    glUniform1i(glGetUniformLocation(sceneShader, "u_b_dualLighting"), b_dualLighting);

    glUseProgram(distortionShader);
    glUniform1i(glGetUniformLocation(distortionShader, "screenTexture"), 0);
    glUniform1i(glGetUniformLocation(distortionShader, "u_b_applyDistortion"), b_applyDistortion);

    //Performance Counter
    int frameCount = 0;
    float fpsTime = 0.0f;

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate frame time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        frameCount++;
        fpsTime += deltaTime;
        if (fpsTime >= 1.0f) {
            std::string title = "VR Scene - FPS: " + std::to_string(frameCount) + "; Key-WSAD_LeftShift/Space And Mouse Scroll to Control Camera; 1-VR_Distortion; 2-Use_Light; 3-Dual_Lighing; Backspace-Disable_1&2";
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            fpsTime = 0.0f;
        }

        processInput(window);

        // Toggle Aberration Effect
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            b_applyDistortion = true;
            glUseProgram(distortionShader);
            glUniform1i(glGetUniformLocation(distortionShader, "u_b_applyDistortion"), b_applyDistortion);
        }
        if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
            b_applyDistortion = false;
            glUseProgram(distortionShader);
            glUniform1i(glGetUniformLocation(distortionShader, "u_b_applyDistortion"), b_applyDistortion);

            b_useLighting = false;
            glUseProgram(sceneShader);
            glUniform1i(glGetUniformLocation(sceneShader, "u_b_useLighting"), b_useLighting);
        }

        // Toggle lighting effects
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            b_useLighting = true;
            glUseProgram(sceneShader);
            glUniform1i(glGetUniformLocation(sceneShader, "u_b_useLighting"), b_useLighting);
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            b_dualLighting = !b_dualLighting;
            glUseProgram(sceneShader);
            glUniform1i(glGetUniformLocation(sceneShader, "u_b_dualLighting"), b_useLighting);
        }

        //Updating dynamic textures
        updateDynamicTexture(dynamicTexture);

        // Step 1: Render to frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Using Scene Shaders
        glUseProgram(sceneShader);

        glm::mat4 projection = glm::perspective(glm::radians(camera_ptr->GetZoom()), 1200.0f / 800.0f, 0.1f, 100.0f);
        glm::mat4 view = camera_ptr->GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(sceneShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(sceneShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(sceneShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(sceneShader, "viewPos"), 1, glm::value_ptr(camera_ptr->GetEye()));

        // Bind dynamic textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dynamicTexture);

        // Rendering the Ring Screen
        glBindVertexArray(ringVAO);
        glDrawElements(GL_TRIANGLES, ringIndices.size(), GL_UNSIGNED_INT, 0);

        //  Adding a reference coordinate system
        glUseProgram(sceneShader);
        glUniform1i(glGetUniformLocation(sceneShader, "useLighting"), 0);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        glUniformMatrix4fv(glGetUniformLocation(sceneShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Use solid color textures
        unsigned int whiteTexture;
        glGenTextures(1, &whiteTexture);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        unsigned char whitePixel[] = { 200, 200, 200, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);

        // Rendering a simple plane as a floor
        float floorVertices[] = {
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f
        };
        unsigned int floorIndices[] = {
            0, 1, 2,
            2, 3, 0
        };

        unsigned int floorVAO, floorVBO, floorEBO;
        glGenVertexArrays(1, &floorVAO);
        glGenBuffers(1, &floorVBO);
        glGenBuffers(1, &floorEBO);

        glBindVertexArray(floorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Restore light settings
        glUniform1i(glGetUniformLocation(sceneShader, "u_b_useLighting"), b_useLighting);
        glBindTexture(GL_TEXTURE_2D, dynamicTexture);

        // Step 2: Render to default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Using Aberration Shaders
        glUseProgram(distortionShader);
        glUniform1f(glGetUniformLocation(distortionShader, "time"), currentFrame);

        // Bind frame buffer texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

        // Render full-screen quads
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        camera_ptr->PrintParams();

        // Swap buffer and polling events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clearing resources
    glDeleteVertexArrays(1, &ringVAO);
    glDeleteBuffers(1, &ringVBO);
    glDeleteBuffers(1, &ringEBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &dynamicTexture);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteProgram(sceneShader);
    glDeleteProgram(distortionShader);

    glfwTerminate();
    return 0;
}