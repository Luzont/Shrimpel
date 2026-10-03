#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

float DeltaTime = 0.0f;
float LastTime = 0.0f;

const float DefaultYaw = -90.0f;
const float DefaultPitch = 0.0f;
const float DefaultSpeed = 2.5f;
const float DefaultSensitivity = 0.1f;
const float DefaultFOV = 45.0f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float FOV;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = DefaultYaw, float pitch = DefaultPitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(DefaultSpeed), MouseSensitivity(DefaultSensitivity), FOV(DefaultFOV)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        UpdateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(CameraMovement Direction)
    {
        float velocity = MovementSpeed * DeltaTime;
        if (Direction == FORWARD)
            Position += Front * velocity;
        if (Direction == BACKWARD)
            Position -= Front * velocity;
        if (Direction == LEFT)
            Position -= Right * velocity;
        if (Direction == RIGHT)
            Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean ConstrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (ConstrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        UpdateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        FOV -= (float)yoffset;
        if (FOV < 1.0f)
            FOV = 1.0f;
        if (FOV > 45.0f)
            FOV = 45.0f;
    }

    void ProcessInputs(GLFWwindow* Window)
    {
        DeltaTime = glfwGetTime() - LastTime;
        LastTime = glfwGetTime();

        if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
            ProcessKeyboard(FORWARD);
        if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
            ProcessKeyboard(BACKWARD);
        if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
            ProcessKeyboard(LEFT);
        if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
            ProcessKeyboard(RIGHT);
    }

private:
    void UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif