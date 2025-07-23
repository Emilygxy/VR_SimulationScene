#include "utils/CustomCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

 CustomCamera::CustomCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : mFront(glm::vec3(0.0f, 0.0f, -1.0f)), mMovementSpeed(2.5f),
        mSensitivity(0.1f), mZoom(90.0f) 
    {
        mPosition = position;
        mWorldUp = up;
        mYaw = yaw;
        mPitch = pitch;

        updateCameraParams();
    }

 void CustomCamera::SetEye(const glm::vec3& val)
 {
     mPosition = val;
     updateCameraParams();
 }

 glm::vec3 CustomCamera::GetEye() const noexcept
 {
     return mPosition;
 }

 void CustomCamera::SetZoom(float val)
 {
     mZoom = val;
     updateCameraParams();
 }

 float CustomCamera::GetZoom() const noexcept
 {
     return mZoom;
 }

CustomCamera::~CustomCamera()
{
}

glm::mat4 CustomCamera::GetViewMatrix() {
        return glm::lookAt(mPosition, mPosition + mFront, mUp);
    }

    void CustomCamera::ProcessKeyboard(int direction, float deltaTime) {
        float velocity = mMovementSpeed * deltaTime;
        if (direction == GLFW_KEY_W)
            mPosition += mFront * velocity;
        if (direction == GLFW_KEY_S)
            mPosition -= mFront * velocity;
        if (direction == GLFW_KEY_A)
            mPosition -= mRight * velocity;
        if (direction == GLFW_KEY_D)
            mPosition += mRight * velocity;
        if (direction == GLFW_KEY_SPACE)
            mPosition += mWorldUp * velocity;
        if (direction == GLFW_KEY_LEFT_SHIFT)
            mPosition -= mWorldUp * velocity;
    }

    void CustomCamera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
        xoffset *= mSensitivity;
        yoffset *= mSensitivity;

        mYaw += xoffset;
        mPitch += yoffset;

        //Limit the tilt angle within [-89, 89] to prevent the camera from flipping over
        if (constrainPitch) {
            if (mPitch > 89.0f)
                mPitch = 89.0f;
            if (mPitch < -89.0f)
                mPitch = -89.0f;
        }

        updateCameraParams();
    }

    void CustomCamera::ProcessMouseScroll(float yoffset) {
        mZoom -= yoffset;
        if (mZoom < 1.0f)
            mZoom = 1.0f;
        if (mZoom > 120.0f)
            mZoom = 120.0f;
    }

    void CustomCamera::updateCameraParams() {
        glm::vec3 front;
        front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
        front.y = sin(glm::radians(mPitch));
        front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
        mFront = glm::normalize(front);

        mRight = glm::normalize(glm::cross(mFront, mWorldUp));
        mUp = glm::normalize(glm::cross(mRight, mFront));
    }

    void CustomCamera::PrintParams()
    {
        std::cout << "Current Camera pos is:(" << GetEye().x << ", " << GetEye().y << ", " << GetEye().z << ")" << std::endl;
        std::cout << "Current Camera Up is:(" << mUp.x <<", " << mUp.y << ", " << mUp.z << ")" << std::endl;
        std::cout << "Current Camera Yaw is: " << mYaw << std::endl;
        std::cout << "Current Camera Pitch is: " << mPitch << std::endl;

    }
