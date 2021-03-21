#include "Main.h"

#include <iostream>

int main(void)
{
    // GLFW Init & Config
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    mWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Window", NULL, NULL);
    if (mWindow == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(mWindow);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferSizeCallback);
    glfwSetCursorPosCallback(mWindow, MouseCallback);
    glfwSetScrollCallback(mWindow, ScrollCallback);
    glfwSetKeyCallback(mWindow, KeyCallback);

    // Capture mouse
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // to enable the changing of the point size when rendering using GL_POINTS
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    mCamera.Position = glm::vec3(0, 0, -10);
    mHeight = 0;

    SetupShaders();
    SetupArraysAndBuffers();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(mWindow))
    {
        DrawErrors();

        ProcessInput(mWindow);
        RenderLoop();
    }

    OnExit();
    return 0;
}

/// <summary>
/// Setup before RenderLoop
/// </summary>

// setup shaders and materials
void SetupShaders()
{
    mShader = new Shader("shader/Vertex.vs", "shader/Fragment.frag", "shader/Geometry.gs");
    
    mNoiseShader = new Shader("shader/Noise.vs", "shader/Noise.frag");
}

// setup vao's and vbo's
void SetupArraysAndBuffers()
{
    // Framebuffer & 3D texture
    glGenTextures(1, &mFboTex);
    glBindTexture(GL_TEXTURE_3D, mFboTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, TERRAIN_WIDTH, TERRAIN_HEIGHT, TERRAIN_DEPTH, 0, GL_RED, GL_FLOAT, nullptr);

    glGenFramebuffers(1, &mFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

    GLenum c = GL_COLOR_ATTACHMENT0;
    glFramebufferTexture3D(GL_FRAMEBUFFER, c, GL_TEXTURE_3D, mFboTex, 0, 0);
    glDrawBuffers(1, &c);

    // VAO & VBO
    glGenVertexArrays(1, &mEmptyVao);

    glGenVertexArrays(1, &mTerrainVao);
    glGenBuffers(1, &mTerrainVbo);

    glBindVertexArray(mTerrainVao);
    glBindBuffer(GL_ARRAY_BUFFER, mTerrainVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVerticesTerrain), mVerticesTerrain, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
}

/// <summary>
/// Setup RenderLoop
/// </summary>

// Methods called every frame
void RenderLoop()
{
    // Per frame logic
    double currentFrame = glfwGetTime();
    mDeltaTime = currentFrame - mLastFrameTime;
    mLastFrameTime = currentFrame;

    DrawErrors();

    ProcessInput(mWindow);

    // Set light uniforms
    RenderScene();

    // Frame End Updates
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
}

// Prints Errors associated with OpenGL every Frame
void DrawErrors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << err << std::endl;
    }
}

void RenderScene()
{
    glViewport(0, 0, TERRAIN_WIDTH, TERRAIN_HEIGHT);
    mNoiseShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    glClear(GL_COLOR_BUFFER_BIT);
    mNoiseShader->setFloat("height", mHeight);
    for (int i = 0; i < TERRAIN_DEPTH; i++)
    {
        float layer = float(i) / float(TERRAIN_DEPTH - 1.0f);
        mNoiseShader->setFloat("layer", layer);
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, mFboTex, 0, i);
        glBindVertexArray(mTerrainVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(mCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 100.0f);
    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    mShader->setMat4("proj", projection);
    mShader->setMat4("view", view);
    mShader->setMat4("model", model);
    
    if (mShowWireFrame)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glBindVertexArray(mEmptyVao);
    glDrawArrays(GL_POINTS, 0, TERRAIN_WIDTH * TERRAIN_HEIGHT * TERRAIN_DEPTH);
}

/// <summary>
/// Callback functions
/// </summary>


// Process input
void ProcessInput(GLFWwindow* window)
{
    float delta = static_cast<float>(mDeltaTime);

    // close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }    

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        mCamera.ProcessKeyboard(Camera::Camera_Movement::FORWARD, delta);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        mCamera.ProcessKeyboard(Camera::Camera_Movement::BACKWARD, delta);
    }   
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        mCamera.ProcessKeyboard(Camera::Camera_Movement::LEFT, delta);
    }  
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        mCamera.ProcessKeyboard(Camera::Camera_Movement::RIGHT, delta);
    } 
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mHeight += delta;
    }   
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mHeight -= delta;
    }    
}

// GLFW Callback when mouse moves
void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (mFirstMouse)
    {
        mLastX = xpos;
        mLastY = ypos;
        mFirstMouse = false;
    }

    double xoffset = xpos - mLastX;
    double yoffset = mLastY - ypos; // Reverse: y-coordinates go from bottom to top

    mLastX = xpos;
    mLastY = ypos;

    mCamera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

//  GLFW Callback when window changes
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// GLFW Callback when mousewheel is used
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    mCamera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_P)
    {
        mShowWireFrame = !mShowWireFrame;
    }
}

/// <summary>
/// After RenderLoop has been finished
/// </summary>

void OnExit()
{
    glDeleteFramebuffers(1, &mFbo);
    glDeleteTextures(1, &mFboTex);
    glDeleteVertexArrays(1, &mTerrainVao);
    glDeleteBuffers(1, &mTerrainVbo);
    glDeleteBuffers(1, &mEmptyVbo);

    glfwTerminate();
}
