#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "Utils/Camera.h"
#include "Utils/Shader.h"
#include "Utils/TextureLoader.h"
#include "Utils/HelperObjects.h"
#include "Utils/Particles/ParticleSystem.h"
#include "Utils/Collision_Detection/KDTree.h"
#include "Utils/Collision_Detection/Triangle.h"

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessInput(GLFWwindow* window);
void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);

void RenderLoop();
void DrawErrors();
void SetupShaders();
void SetupTextures();
void SetupArraysAndBuffers();
void RenderLoop();
void RenderScene();
void OnExit();

void UpdateParticleSystem();

glm::vec3 CreateRay(const glm::mat4& projection, const glm::mat4& view);

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
Shader* mParallaxShader;
Shader* mTriangleShader;

// Textures
unsigned int diffuseMap;
unsigned int normalMap;
unsigned int heightMap;

unsigned int mFbo;
unsigned int mFboTex;

unsigned int mTerrainVao;
unsigned int mTerrainVbo;

unsigned int mEmptyVao;
unsigned int mEmptyVbo;

float mHeight;
int mNormalSteps = 10;
int mRefinementSteps = 5;

float mVerticesTerrain[6][2] = { {-1.0f, -1.0f}, {-1.0, 1.0}, {1.0, -1.0}, {1.0f, 1.0f}, {-1.0, 1.0}, {1.0, -1.0} };

float mShowWireFrame = false;

float heightScale = 0.1;
// lighting info
glm::vec3 lightPos(-5.0f, 1.0f, 0.3f);
glm::vec3 startPos(-5.0f, 0.0f, 0.0f);

ParticleSystem mParticleSystem;

//kd tree vars
int triangleAmount = 40;
int maxVal = 10;
int minVal = -maxVal;
std::vector<Triangle> triangles;
KDTree tree;
Triangle* lastResult;

unsigned int triangleVAO;
unsigned int triangleVBO;
unsigned int pointVAO;
unsigned int pointVBO;
float point[3] = { 0.0f, 0.0f,  0.0f };
unsigned int wireCubeVAO;
unsigned int wireCubeVBO;

//mouse values
double mouseX, mouseY;
double clickX = -10, clickY = -10;
