#include "Main.h"

#include <iostream>
#include <random>

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
    mWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SPG", NULL, NULL);
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
    glfwSetMouseButtonCallback(mWindow, MouseButtonCallBack);

    // Capture mouse
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //create random triangles in range [minValue, maxValue]
    std::default_random_engine e1(1234);
    std::uniform_int_distribution<int> uniform_dist(minVal, maxVal);

    for (int i = 0; i < triangleAmount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(uniform_dist(e1), uniform_dist(e1), uniform_dist(e1)));
        model = glm::rotate(model, (float)uniform_dist(e1), glm::vec3(1, 0, 0));
        //model = glm::rotate(model, (float)90, glm::vec3(1, 0, 0));
        model = glm::rotate(model, (float)uniform_dist(e1), glm::vec3(0, 1, 0));
        model = glm::rotate(model, (float)uniform_dist(e1), glm::vec3(0, 0, 1));
        triangles.push_back(Triangle(model));
    }

    tree = KDTree(triangles, minVal, maxVal);

    // to enable the changing of the point size when rendering using GL_POINTS
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    // Setup Members
    mCamera.Position = glm::vec3(0, 0, -3);
    mHeight = 0;

    mParticleSystem.SetGeneratorProperties(
        glm::vec3(0.0f, 0.0f, 0.0f), // Where the particles are generated
        glm::vec3(-20, 30, -5), // Minimal velocity
        glm::vec3(20, 50, 5), // Maximal velocity
        glm::vec3(0, -5, 0), // Gravity force applied to particles
        glm::vec3(1.0f, 1.0f, 1.0f), // Color
        4.0f, // Minimum lifetime in seconds
        5.0f, // Maximum lifetime in seconds
        0.5f, // Rendered size
        0.02f, // Spawn every 0.05 seconds
        30); // And spawn 30 particles

    SetupShaders();
    SetupTextures();
    SetupArraysAndBuffers();
    DepthMapConfig();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwGetCursorPos(mWindow, &mouseX, &mouseY);
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

    mParallaxShader = new Shader("shader/Parallax.vs", "shader/Parallax.frag");

    mTriangleShader = new Shader("shader/SimpleVertexShader.vs", "shader/SimpleColorFragmentShader.frag");

    mDepthShader = new Shader("shader/Depth.vs", "shader/Depth.frag");

    mSoftShadowShader = new Shader("shader/SoftShadowShader.vs", "shader/SoftShadowShader.frag");
}

void SetupTextures()
{
    
    mParticleSystem.InitializeParticleSystem();
    mParticleSystem.mTexture = loadTexture("resource/bricks.jpg");
    
    diffuseMap = loadTexture("resource/bricks.jpg");
    normalMap = loadTexture("resource/bricks_normal.jpg");
    heightMap = loadTexture("resource/bricks_disp.jpg");

    mParallaxShader->use();
    mParallaxShader->setInt("diffuseMap", 0);
    mParallaxShader->setInt("normalMap", 1);
    mParallaxShader->setInt("depthMap", 2);

    mSoftShadowShader->use();
    mSoftShadowShader->setInt("diffuseTexture", 0);
    mSoftShadowShader->setInt("shadowMap", 1);
}

// setup vao's and vbo's
void SetupArraysAndBuffers()
{
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);
    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle::mesh), Triangle::mesh, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glBindVertexArray(0);

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

    UpdateParticleSystem();
    // Set light uniforms
    RenderScene();

    //std::cout << mParticleSystem.GetNumParticles() << std::endl;

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

void UpdateParticleSystem()
{
    mParticleSystem.UpdateParticles(mDeltaTime);
}

void DepthMapConfig() {
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderScene()
{
    glm::mat4 projection = glm::perspective(glm::radians(mCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = mCamera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    // render scene from shadows perspective for shadow map
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 0.1f, far_plane = 100.0f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPosShadows, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    // render scene from light's point of view
    mDepthShader->use();
    mDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    mDepthShader->setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
    mDepthShader->setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
    mDepthShader->setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
    mDepthShader->setMat4("model", model);
    renderPlane();
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // render objects normally with generated shadow map
    mSoftShadowShader->use();
    mSoftShadowShader->setMat4("projection", projection);
    mSoftShadowShader->setMat4("view", view);
    // set light uniforms
    mSoftShadowShader->setVec3("viewPos", mCamera.Position);
    mSoftShadowShader->setVec3("lightPos", lightPosShadows);
    mSoftShadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    mSoftShadowShader->setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
    mSoftShadowShader->setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
    mSoftShadowShader->setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
    mSoftShadowShader->setMat4("model", model);
    renderPlane();

    // render terrain
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

    /*glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

    mShader->use();
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    mShader->setMat4("proj", projection);
    mShader->setMat4("view", view);
    mShader->setMat4("model", model);
    
    if (mShowWireFrame)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glBindVertexArray(mEmptyVao);
    glDrawArrays(GL_POINTS, 0, TERRAIN_WIDTH * TERRAIN_HEIGHT * TERRAIN_DEPTH);

    //Parallax Render
    //configure view/projection matrices    
    mParallaxShader->use();
    mParallaxShader->setMat4("projection", projection);
    mParallaxShader->setMat4("view", view);
    //render parallax-mapped quad
    //glm::mat4 model = glm::mat4(1.0f);
    model = glm::mat4(1.0f);
    model = glm::translate(model, startPos);
    //model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0))); // rotate the quad to show parallax mapping from multiple directions
    mParallaxShader->setMat4("model", model);
    mParallaxShader->setVec3("viewPos", mCamera.Position);
    mParallaxShader->setVec3("lightPos", lightPos);
    mParallaxShader->setFloat("heightScale", heightScale); // adjust with Q and E keys
    mParallaxShader->setInt("normalSteps", mNormalSteps); // adjust with , and . keys
    mParallaxShader->setInt("refinementSteps", mRefinementSteps); // adjust with + and - keys
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    renderQuad();

   /* model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos + glm::vec3(0.0, 0.0, -1.0));
    model = glm::scale(model, glm::vec3(0.2f));
    mParallaxShader->setMat4("model", model);
    renderQuad();*/

    //render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    mParallaxShader->setMat4("model", model);
    renderQuad();

    //triangles
    mTriangleShader->use();
    mTriangleShader->setMat4("projection", projection);
    mTriangleShader->setMat4("view", view);
    glBindVertexArray(triangleVAO);
    for (auto triangle : triangles) {
        mTriangleShader->setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));
        mTriangleShader->setMat4("model", triangle.getModelMat());
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // set matrices for particle system and render particles
    mParticleSystem.SetMatrices(projection, view, mCamera.Front, mCamera.Up);
    mParticleSystem.RenderParticles();

    // we test if we need to create a new raycast and if so search the tree for a hit
    if (clickX > 0 && clickY > 0 && clickX < SCR_WIDTH && clickY < SCR_HEIGHT) {
        //std::cout << clickX << " " << clickY << std::endl;
        glm::vec3 ray = CreateRay(projection, view);
        //std::cout << ray.x << " " << ray.y << " " << ray.z << std::endl;

        float camPos[3] = { mCamera.Position.x, mCamera.Position.y, mCamera.Position.z };
        //float camPos[3] = { cameraPos.x, cameraPos.y, cameraPos.z };
        float rayDir[3] = { ray.x, ray.y, ray.z };

        Triangle* result = tree.searchHit(camPos, rayDir, 100);
        if (result != nullptr) {
            lastResult = result;
        }

        clickX = -10;
        clickY = -10;
        std::cout << tree.lastPoint.x << " " << tree.lastPoint.y << " " << tree.lastPoint.z << std::endl;
    }

    if (tree.lastPoint.x != 0 && tree.lastPoint.y != 0 && tree.lastPoint.z != 0) {
        mParticleSystem.SetGeneratorPosition(tree.lastPoint);
    }
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        ++mNormalSteps;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        mNormalSteps = std::max(--mNormalSteps, 1);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        ++mRefinementSteps;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        mRefinementSteps = std::max(--mRefinementSteps, 1);
    }
    // Particle System
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        mParticleSystem.mNextGenerationTime += delta;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        mParticleSystem.mNextGenerationTime = std::max(mParticleSystem.mNextGenerationTime - delta, 0.01f);
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

void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        clickX = mouseX;
        clickY = mouseY;
    }
}

glm::vec3 CreateRay(const glm::mat4& projection, const glm::mat4& view) {
    float x = (2.0f * clickX) / SCR_WIDTH - 1.0f;
    float y = 1.0f - (2.0f * clickY) / SCR_HEIGHT;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    glm::vec4 result = (glm::inverse(view) * ray_eye);
    glm::vec3 ray_wor = glm::vec3(result.x, result.y, result.z);
    // don't forget to normalise the vector at some point
    ray_wor = glm::normalize(ray_wor);
    return ray_wor;
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
