#pragma once
#include <vector>
#include <glm/gtc/type_ptr.hpp>

// Compile shaders
unsigned int compileShader(unsigned int type, const char* source);
// create shader program
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);

// create full screen
void createQuad(unsigned int& quadVAO, unsigned int& quadVBO);

void createRingScreenWithBezier(
       /*glm::vec3 controlPoints[4][4],*/ 
        int segmentsU, int segmentsV,
        std::vector<float>& vertices,
        std::vector<unsigned int>& indices);

void generateDynamicTextureData(std::vector<unsigned char>& data, int width, int height);
void updateDynamicTexture(unsigned int textureID);


