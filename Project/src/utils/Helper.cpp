#include "utils/Helper.h"
#include <glad/glad.h>
#include <random>
#include <iostream>

namespace {

    // bezier surface

    void GenerateControlPoints4x4(glm::vec3 controlPoints[4][4])
    {
        float R = 2.0f, H = 1.0f, angle = glm::half_pi<float>(); // 90°
        //glm::vec3 controlPoints[4][4];

        for (int i = 0; i < 4; ++i) {
            float u = float(i) / 3;
            float theta = -angle / 2 + u * angle; // -45°到+45°
            float x = R * sin(theta);
            for (int j = 0; j < 4; ++j) {
                float v = float(j) / 3;
                float y = -H / 2 + v * H;
                float z = R * (1 - cos(theta));
                controlPoints[i][j] = glm::vec3(x, y, z);
            }
        }
    }


    float bernstein(int i, float t) {
        switch(i) {
            case 0: return (1-t)*(1-t)*(1-t);
            case 1: return 3*t*(1-t)*(1-t);
            case 2: return 3*t*t*(1-t);
            case 3: return t*t*t;
        }
        return 0;
    }

    glm::vec3 bezierSurfacePoint(const glm::vec3 cp[4][4], float u, float v) {
        glm::vec3 p(0.0f);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                p += bernstein(i, u) * bernstein(j, v) * cp[i][j];
        return p;
    }

    float bernsteinDeriv(int i, float t) {
        switch(i) {
            case 0: return -3*(1-t)*(1-t);
            case 1: return 3*(1-t)*(1-t) - 6*t*(1-t);
            case 2: return 6*t*(1-t) - 3*t*t;
            case 3: return 3*t*t;
        }
        return 0;
    }
    
    glm::vec3 bezierSurfaceTangentU(const glm::vec3 cp[4][4], float u, float v) {
        glm::vec3 p(0.0f);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                p += bernsteinDeriv(i, u) * bernstein(j, v) * cp[i][j];
        return p;
    }

    glm::vec3 bezierSurfaceTangentV(const glm::vec3 cp[4][4], float u, float v) {
        glm::vec3 p(0.0f);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                p += bernstein(i, u) * bernsteinDeriv(j, v) * cp[i][j];
        return p;
    }
}

// Compile shaders
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Compile Shader Failed at :\n" << infoLog << std::endl;
    }
    return shader;
}

// create shader program
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ShaderProgram Compile error ! which is :\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// create full screen
void createQuad(unsigned int& quadVAO, unsigned int& quadVBO) {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void generateDynamicTextureData(std::vector<unsigned char>& data, int width, int height) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);

    data.resize(width * height * 4);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;

            // desktop background
            data[idx] = 30;     // R
            data[idx + 1] = 30;   // G
            data[idx + 2] = 40;   // B
            data[idx + 3] = 255;  // A

            // simulate window
            if (x > width / 4 && x < 3 * width / 4 && y > height / 4 && y < 3 * height / 4) {
                data[idx] = 200;
                data[idx + 1] = 200;
                data[idx + 2] = 210;

                // content
                if (x > width / 4 + 20 && x < 3 * width / 4 - 20 &&
                    y > height / 4 + 40 && y < 3 * height / 4 - 20) {
                    // text
                    if (y % 20 < 15 && x > width / 4 + 40 && x < 3 * width / 4 - 40) {
                        data[idx] = 0;
                        data[idx + 1] = 0;
                        data[idx + 2] = 0;
                    }

                    // button
                    if (y > 3 * height / 4 - 50 && y < 3 * height / 4 - 30) {
                        if (x > width / 2 - 40 && x < width / 2 + 40) {
                            data[idx] = 70;
                            data[idx + 1] = 130;
                            data[idx + 2] = 200;
                        }
                    }
                }
            }

            // random noise
            if (dis(gen) > 250) {
                data[idx] = 255;
                data[idx + 1] = 255;
                data[idx + 2] = 255;
            }
        }
    }
}

void updateDynamicTexture(unsigned int textureID) {
    static int frameCounter = 0;
    frameCounter++;

    const int texWidth = 1024;
    const int texHeight = 768;
    static std::vector<unsigned char> textureData;

    generateDynamicTextureData(textureData, texWidth, texHeight);

    // add dynamic ele
    int centerX = texWidth / 2 + static_cast<int>(50 * sin(frameCounter * 0.05f));
    int centerY = texHeight / 2 + static_cast<int>(30 * cos(frameCounter * 0.03f));
    int radius = 20 + static_cast<int>(5 * sin(frameCounter * 0.1f));

    for (int y = centerY - radius; y <= centerY + radius; ++y) {
        for (int x = centerX - radius; x <= centerX + radius; ++x) {
            if (x >= 0 && x < texWidth && y >= 0 && y < texHeight) {
                float dist = (float)sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
                if (dist <= radius) {
                    int idx = (y * texWidth + x) * 4;
                    textureData[idx] = 220;     // R
                    textureData[idx + 1] = 80;    // G
                    textureData[idx + 2] = 60;     // B
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight,
        GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
}

void createRingScreenWithBezier(
    /*glm::vec3 controlPoints[4][4],*/ 
    int segmentsU, int segmentsV,
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices)
    {
        vertices.clear();
        indices.clear();

        glm::vec3 controlPoints[4][4];
        GenerateControlPoints4x4(controlPoints);

        for (int i = 0; i <= segmentsU; ++i) {
            float u = float(i) / segmentsU;
            for (int j = 0; j <= segmentsV; ++j) {
                float v = float(j) / segmentsV;

                glm::vec3 pos = bezierSurfacePoint(controlPoints, u, v);
                glm::vec3 du = bezierSurfaceTangentU(controlPoints, u, v);
                glm::vec3 dv = bezierSurfaceTangentV(controlPoints, u, v);
                glm::vec3 normal = glm::normalize(glm::cross(du, dv));

                vertices.push_back(pos.x);
                vertices.push_back(pos.y);
                vertices.push_back(pos.z);
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        int rowVerts = segmentsV + 1;
        for (int i = 0; i < segmentsU; ++i) {
            for (int j = 0; j < segmentsV; ++j) {
                int idx = i * rowVerts + j;
                indices.push_back(idx);
                indices.push_back(idx + rowVerts);
                indices.push_back(idx + rowVerts + 1);

                indices.push_back(idx);
                indices.push_back(idx + rowVerts + 1);
                indices.push_back(idx + 1);
            }
        }
    }