#pragma once
#include <glm/glm.hpp>

//Custom camera class
class CustomCamera
{
public:
    CustomCamera() = delete;
    CustomCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f);
    ~CustomCamera();

    void SetEye(const glm::vec3& val);
    glm::vec3 GetEye() const noexcept;

    void SetZoom(float val);
    float GetZoom() const noexcept;

    glm::mat4 GetViewMatrix();

    void ProcessKeyboard(int direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void PrintParams();

private:
    void updateCameraParams();

private:
    glm::vec3 mPosition;
    glm::vec3 mFront;
    glm::vec3 mUp;
    glm::vec3 mRight;
    glm::vec3 mWorldUp;

    float mYaw;
    float mPitch;

    float mMovementSpeed;
    float mSensitivity;
    float mZoom;
};


    

    

    

