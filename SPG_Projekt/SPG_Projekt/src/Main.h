#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "Utils/Camera.h"
#include "Utils/Shader.h"

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessInput(GLFWwindow* window);

void RenderLoop();
void DrawErrors();
void SetupShaders();
void SetupArraysAndBuffers();
void RenderLoop();
void RenderScene();
void OnExit();

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

const unsigned int TERRAIN_WIDTH =  64;
const unsigned int TERRAIN_HEIGHT = 64;
const unsigned int TERRAIN_DEPTH = 64;

Camera mCamera;
double mLastX = SCR_WIDTH / 2.0;
double mLastY = SCR_HEIGHT / 2.0;
bool mFirstMouse;

GLFWwindow* mWindow;
double mDeltaTime;
double mLastFrameTime;

// shaders
Shader* mNoiseShader;
Shader* mShader;

unsigned int mFbo;
unsigned int mFboTex;

unsigned int mTerrainVao;
unsigned int mTerrainVbo;

unsigned int mEmptyVao;
unsigned int mEmptyVbo;

float mHeight;

float mVerticesTerrain[6][2] = { {-1.0f, -1.0f}, {-1.0, 1.0}, {1.0, -1.0}, {1.0f, 1.0f}, {-1.0, 1.0}, {1.0, -1.0} };

float mShowWireFrame = false;