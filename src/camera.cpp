#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

#include "camera.h"
#include "physics.h"

Camera *currentCam = nullptr;
std::vector<Camera *> cameras;
int currentCamIdx = 0;
int curr_cam_id = 1;

void setCurrentCamera(Camera *camera)
{
    if (currentCam)
    {
        currentCam->setForward(false);
        currentCam->setBackward(false);
        currentCam->setLeftward(false);
        currentCam->setRightward(false);
        currentCam->setSpeedMode(false);
    }

    currentCam = camera;
    std::cout << "Current Camera: " << currentCam->Id << std::endl;
}

void toggleCurrentCamera()
{
    currentCamIdx = ++currentCamIdx % cameras.size();
    setCurrentCamera(cameras.at(currentCamIdx));
}

Camera::Camera(int screenwidth, int screenHeight, World *worldPtr)
{
    world = worldPtr;
    lastX = screenwidth / 2;
    lastY = screenHeight / 2;

    deltaTime = 0.0f;
    lastTick = 0.0f;

    speedMode = false;
    height = 1.4f;
    isJumping = false;

    Id = curr_cam_id;
    curr_cam_id++;

    cameraPos = glm::vec3(0.0f, 150.0f, 3.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    firstLook = true;
    yaw = 0.0f;
    pitch = 0.0f;

    baseSpeed = 12.0f;
    maxSpeed = 6.0f;
    velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::mat4 Camera::getView()
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    return view;
}

glm::vec3 Camera::getFront()
{
    return cameraFront;
}

void Camera::tick(float currentTime)
{

    deltaTime = currentTime - lastTick;
    lastTick = currentTime;

    // Update camera front direction based on yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    // std::cout << "front: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ") " << std::endl;
    // std::cout << "pos: (" << cameraPos.x << ", \t\t" << cameraPos.y << ", \t\t" << cameraPos.z << ") " << std::endl;

    // Update camera right vector (cross product of cameraFront and world up)
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Constants for movement and gravity
    float acceleration = 50.0f;       // Controls how quickly velocity increases
    const float deceleration = 80.0f; // Controls how quickly velocity decreases
    const float gravity = -40.0f;     // Gravity acceleration
    glm::vec3 accelerationVector(0.0f);

    float useSpeed = maxSpeed;
    if (!physics)
    {
        useSpeed = useSpeed * 4;
    }

    if (speedMode)
    {
        acceleration = acceleration * 1.5;
        useSpeed = maxSpeed * 1.3;
    }

    // Movement input along the x-z plane
    if (movingForward)
    {
        accelerationVector += glm::vec3(cameraFront.x, 0.0f, cameraFront.z); // No vertical movement
    }
    if (movingBackward)
    {
        accelerationVector -= glm::vec3(cameraFront.x, 0.0f, cameraFront.z); // No vertical movement
    }
    if (movingLeftward)
    {
        accelerationVector -= glm::vec3(cameraRight.x, 0.0f, cameraRight.z); // No vertical movement
    }
    if (movingRightward)
    {
        accelerationVector += glm::vec3(cameraRight.x, 0.0f, cameraRight.z); // No vertical movement
    }
    if (movingDownward && !physics)
    {
        accelerationVector += -cameraUp;
    }

    // Apply acceleration based on input
    if (glm::length(accelerationVector) > 0.0f)
    {
        // Normalize accelerationVector to maintain direction, then apply acceleration
        accelerationVector = glm::normalize(accelerationVector);
        velocity += accelerationVector * acceleration * (float)deltaTime;

        // Clamp the velocity magnitude to the maximum speed
        auto length = glm::length(glm::vec3(velocity.x, 0.0f, velocity.z));
        if (length > useSpeed)
        {
            auto normalized = glm::normalize(glm::vec3(velocity.x, 0.0f, velocity.z)) * useSpeed;
            velocity = glm::vec3(normalized.x, velocity.y, normalized.z);
        }
    }

    // Always apply gravity to the vertical velocity
    if (physics)
    {
        auto gravityCalc = velocity.y + gravity * (float)deltaTime;
        //std::cout << "gravity calc: " << gravityCalc << std::endl;

        velocity.y = gravityCalc;
        velocity.y = std::clamp(velocity.y, -20.0f, 100.0f);
    }

    float rayDist = 0.3f;
    float xzThreshold = 0.2;
    bool xPosHit = false;
    bool xNegHit = false;
    bool yPosHit = false;
    bool yNegHit = false;
    bool zNegHit = false;
    bool zPosHit = false;

    // Perform raycast checks for positive and negative directions along each axis
    RayCastInfo rayInfoXPos = {*world, cameraPos, glm::vec3(0.998f, 0.001f, 0.001f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoXPosLow = {*world, glm::vec3(cameraPos.x, cameraPos.y - height, cameraPos.z), glm::vec3(0.998f, 0.001f, 0.001f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoXNeg = {*world, cameraPos, glm::vec3(-0.998f, 0.001f, 0.001f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoXNegLow = {*world, glm::vec3(cameraPos.x, cameraPos.y - height, cameraPos.z), glm::vec3(-0.998f, 0.001f, 0.001f), rayDist, doNothingIfHit};

    RayCastInfo rayInfoYPos = {*world, cameraPos, glm::vec3(0.001f, 0.998f, 0.001f), rayDist, doNothingIfHit};

    RayCastInfo rayInfoYNeg = {*world, glm::vec3(cameraPos.x - xzThreshold, cameraPos.y - height, cameraPos.z), glm::vec3(0.001f, -0.998f, 0.001f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoYNeg2 = {*world, glm::vec3(cameraPos.x + xzThreshold, cameraPos.y - height, cameraPos.z), glm::vec3(0.001f, -0.998f, 0.001f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoYNeg3 = {*world, glm::vec3(cameraPos.x, cameraPos.y - height, cameraPos.z - xzThreshold), glm::vec3(0.001f, -0.998f, 0.001f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoYNeg4 = {*world, glm::vec3(cameraPos.x, cameraPos.y - height, cameraPos.z + xzThreshold), glm::vec3(0.001f, -0.998f, 0.001f), rayDist, doNothingIfHit};

    RayCastInfo rayInfoZPos = {*world, cameraPos, glm::vec3(0.001f, 0.001f, 0.998f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoZPosLow = {*world, glm::vec3(cameraPos.x, cameraPos.y - height, cameraPos.z), glm::vec3(0.001f, 0.001f, 0.998f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoZNeg = {*world, cameraPos, glm::vec3(0.001f, 0.001f, -0.998f), rayDist, doNothingIfHit};
    RayCastInfo rayInfoZNegLow = {*world, glm::vec3(cameraPos.x, cameraPos.y - height, cameraPos.z), glm::vec3(0.001f, 0.001f, -0.998f), rayDist, doNothingIfHit};

    if (shoot_ray(rayInfoYNeg) || shoot_ray(rayInfoYNeg2) || shoot_ray(rayInfoYNeg3) || shoot_ray(rayInfoYNeg4))
    {
        yNegHit = true;
    }
    if (shoot_ray(rayInfoXPosLow) || shoot_ray(rayInfoXPos))
    {
        xPosHit = true;
    }
    if (shoot_ray(rayInfoXNeg) || shoot_ray(rayInfoXNegLow))
    {
        xNegHit = true;
    }
    if (shoot_ray(rayInfoZNeg) || shoot_ray(rayInfoZNegLow))
    {
        zNegHit = true;
    }
    if (shoot_ray(rayInfoZPos) || shoot_ray(rayInfoZPosLow))
    {
        zPosHit = true;
    }

    if (isJumping && yNegHit)
    {
        isJumping = false;
    }

    if (physics)
    {
        // If ray hits something, stop movement in that direction
        if (velocity.x > 0.0f && xPosHit)
            velocity.x = 0.0f; // Positive X
        if (velocity.x < 0.0f && xNegHit)
            velocity.x = 0.0f; // Negative X
        if (velocity.y > 0.0f && shoot_ray(rayInfoYPos))
            velocity.y = 0.0f; // Positive Y
        if (velocity.y < 0.0f && yNegHit)
            velocity.y = 0.0f; // Negative Y
        if (velocity.z > 0.0f && zPosHit)
            velocity.z = 0.0f; // Positive Z
        if (velocity.z < 0.0f && zNegHit)
            velocity.z = 0.0f; // Negative Z
    }

    // Vertical movement (upward and downward)
    if (movingUpward)
    {
        if (!isJumping && yNegHit)
        {
            isJumping = true;
            if (velocity.y == 0.0f)
                velocity.y += 10.0f;
        }
        if (!physics)
        {
            if (velocity.y == 0.0f)
                velocity.y += 10.0f;
        }
    }

    // Decelerate smoothly if no input (only for X and Z components)
    if (glm::length(accelerationVector) == 0.0f)
    {
        if (glm::length(velocity) > 0.0f)
        {
            glm::vec3 decelerationVector = glm::normalize(velocity) * deceleration * (float)deltaTime;

            // Only apply deceleration to the X and Z components
            if (glm::length(decelerationVector) > glm::length(glm::vec3(velocity.x, 0.0f, velocity.z)))
            {
                velocity.x = 0.0f;
                velocity.z = 0.0f;
            }
            else
            {
                // Apply deceleration to X and Z only
                velocity.x -= decelerationVector.x;
                velocity.z -= decelerationVector.z;
            }
        }
    }
    if (!physics)
    {
        if (glm::length(accelerationVector) == 0.0f)
        {
            if (glm::length(velocity) > 0.0f)
            {
                glm::vec3 decelerationVector = glm::normalize(velocity) * deceleration * (float)deltaTime;

                // Only apply deceleration to the X and Z components
                if (glm::length(decelerationVector) > glm::length(velocity))
                {
                    velocity.x = 0.0f;
                    velocity.y = 0.0f;
                    velocity.z = 0.0f;
                }
                else
                {
                    // Apply deceleration to X and Z only
                    velocity.x -= decelerationVector.x;
                    velocity.y -= decelerationVector.y;
                    velocity.z -= decelerationVector.z;
                }
            }
        }
    }

    // Apply movement to camera position based on velocity
    cameraPos += velocity * (float)deltaTime;
    if (cameraPos.y < -100)
    {
        cameraPos.y = 100.0f;
    }

    std::lock_guard<std::mutex> lock(world->pos_mtx);
    world->worldCurrPos = {(int)(cameraPos.x / CHUNK_SIZE), (int)(cameraPos.z / CHUNK_SIZE)};
}

void Camera::setForward(bool setter)
{
    movingForward = setter;
}

void Camera::setBackward(bool setter)
{
    movingBackward = setter;
}

void Camera::setLeftward(bool setter)
{
    movingLeftward = setter;
}

void Camera::setRightward(bool setter)
{
    movingRightward = setter;
}

void Camera::setUpward(bool setter)
{
    movingUpward = setter;
}

void Camera::setDownward(bool setter)
{
    movingDownward = setter;
}

void Camera::setSpeedMode(bool setter)
{
    speedMode = setter;
}

void Camera::setPhysics(bool setter)
{
    physics = setter;
}

void Camera::updateLookCoords(double xpos, double ypos)
{
    if (firstLook)
    {
        lastX = xpos;
        lastY = ypos;
        firstLook = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(direction);
}

glm::vec3 Camera::getPos()
{
    return cameraPos;
}