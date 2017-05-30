// ARTHUR
// Real Time OpenGL Renderer
// By Rushil Kekre
// C++, OpenGL, GLSL, ImGUI

// ================================

// HEADER FILES

// Standard C++ Headers
#include <string>

// GLEW Header
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW Header
#include <GLFW/glfw3.h>

// Custom headers
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Skybox.h"

// GLM Mathemtics Header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Image loading Header
#include <SOIL.h>

// ImGUI Headers
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

// ================================

// FUNCTION PROTOTYPES

// guiSetup() is self explanatory
// key_callback() to check for keyboard input and act accordingly
// scroll_callback(), mouse_button_callback() and mouse_callback() are used to listen for mouse events
// Do_Movement() to move around the screen if certain keys are pressed
// loadTexture() is self explanatory
void guiSetup();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
void RenderQuad();
GLuint loadTexture(GLchar* path);


// ================================

// WINDOW PROPERTIES
GLuint SCREEN_WIDTH = 1280, SCREEN_HEIGHT = 720;

// ================================

// IMGUI GLOBAL VARIABLES AND DECLARATIONS

int guiWidth = 400;

// Transform variables
GLfloat scaleFactor = 2.0f;
glm::vec3 modelScale(2.0f);
GLfloat rotationAngle = 0.2;
bool rotX = false;
bool rotY = true;
bool rotZ = false;

// Skybox
GLuint skyboxVAO, skyboxVBO;
std::string skyboxPath;
//Skybox cubemap(skyboxVAO,skyboxVBO);
Skybox cubemap;
GLuint cubemapTexture;

// Shading
//ImVec4 objectColor = ImColor(175, 175, 175);
ImVec4 ambientMaterial = ImColor(0, 0, 0);
ImVec4 diffuseMaterial = ImColor(255, 255, 255);
ImVec4 specularMaterial = ImColor(127, 127, 127);
float shineAmount = 0.25;

// Lighting
ImVec4 lightColor = ImColor(255, 255, 255);


// ================================

// MAIN GLOBAL VARIABLES

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
bool mouseClickActive;

// Time
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Model
Model ourModel;
GLboolean blinn = false;

// Light
glm::vec3 lightPos(0.0f, 1.0f, 2.5f);
GLfloat lightIntensity = 0.5f;
glm::vec3 lightDirection(1.0f, 3.0f, 3.0f);
int lightMode = 1;
int attenuationMode = 1;



// ================================

// MAIN FUNCTION
int main()
{
    // Initializing GLFW, specifying version of OpenGL in Core profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Window creation
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Arthur", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);

    // Setting callback functions for keyboard/mouse input
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Specifying input mode
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // Set ImGUI Binding
    ImGui_ImplGlfwGL3_Init(window, true);

    // Define the viewport dimensions
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);


    // List of shaders
    Shader gridShader("shaders/gridTexture.vert", "shaders/gridTexture.frag");
    Shader modelShader("shaders/model_loading.vert", "shaders/model_loading.frag");
    Shader modelReflection("shaders/model_reflection.vert", "shaders/model_reflection.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");
    Shader floorShader("shaders/floorShader.vert", "shaders/floorShader.frag");
    Shader modelGeometryPass("shaders/model_geometry.vert", "shaders/model_geometry.frag");
    Shader modelLightingPass("shaders/model_lighting.vert", "shaders/model_lighting.frag");

    GLfloat planeVertices[] = {
        // Positions          // Normals         // Texture Coords
        1.0f, -0.5f,  1.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
        -1.0f, -0.5f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f, -0.5f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,

        1.0f, -0.5f,  1.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
        -1.0f, -0.5f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,
        1.0f, -0.5f, -1.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f
    };
    // Setup plane VAO
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glBindVertexArray(0);
    GLuint floorTexture = loadTexture("images/checkerboard.jpg");


    GLfloat skyboxVertices[] = {
        // Positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    // Setup skybox VAO
    //GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    //Setting skybox default path
    skyboxPath = "images/san-francisco";
    cubemapTexture = cubemap.configureSkybox(skyboxPath);

    // Setting samplers
    modelLightingPass.Use();
    glUniform1i(glGetUniformLocation(modelLightingPass.Program,"gPosition"),0);
    glUniform1i(glGetUniformLocation(modelLightingPass.Program, "gNormal"),1);
    glUniform1i(glGetUniformLocation(modelLightingPass.Program, "gAlbedoSpec"),2);

    // Load default model
    ourModel.loadModel("models/shaderBall_small.obj");
    
    // Model positon
    //glm::vec3 objectPosition(0.0, 0.0, 0.0);
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-0.5, -1.0, 0.0));
    // - Colors
    const GLuint NR_LIGHTS = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(13);
    lightPositions.push_back(glm::vec3(2.0, 2.0, 2.0));
    lightPositions.push_back(glm::vec3(-2.0, 2.0, 2.0));
    lightPositions.push_back(glm::vec3(0.0, 2.0, -2.0));
    lightColors.push_back(glm::vec3(1.0, 1.0, 1.0));
    lightColors.push_back(glm::vec3(1.0, 1.0, 1.0));
    lightColors.push_back(glm::vec3(1.0, 1.0, 1.0));
    

    // Set up G-Buffer
    // 3 textures:
    // 1. Positions (RGB)
    // 2. Color (RGB) + Specular (A)
    // 3. Normals (RGB) 
    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    GLuint gPosition, gNormal, gAlbedoSpec;
    // - Position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // - Normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // - Color + Specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // - Create and attach depth buffer (renderbuffer)
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // - Finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);


    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();
        ImGui_ImplGlfwGL3_NewFrame();

        // Clear the colorbuffer
        /*glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

        // GUI
        guiSetup();

        
        // Deferred Rendering
        // 1. Geometry Pass: render scene's geometry/color data into gbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model;
        modelGeometryPass.Use();
        glUniformMatrix4fv(glGetUniformLocation(modelGeometryPass.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(modelGeometryPass.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = glm::mat4();
        model = glm::translate(model, objectPositions[0]);
        glUniformMatrix4fv(glGetUniformLocation(modelGeometryPass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        ourModel.Draw(modelGeometryPass);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        modelLightingPass.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        // Also send light relevant uniforms
        //glUniform3fv(glGetUniformLocation(modelLightingPass.Program, ("lights[0].Position").c_str()), 1, &lightPosition);
        for (GLuint i = 0; i < lightPositions.size(); i++)
        {
            glUniform3fv(glGetUniformLocation(modelLightingPass.Program, ("lights[" + std::to_string(i) + "].Position").c_str()), 1, &lightPositions[i][0]);
            glUniform3fv(glGetUniformLocation(modelLightingPass.Program, ("lights[" + std::to_string(i) + "].Color").c_str()), 1, &lightColors[i][0]);
            // Update attenuation parameters and calculate radius
            const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
            const GLfloat linear = 0.7;
            const GLfloat quadratic = 1.8;
            glUniform1f(glGetUniformLocation(modelLightingPass.Program, ("lights[" + std::to_string(i) + "].Linear").c_str()), linear);
            glUniform1f(glGetUniformLocation(modelLightingPass.Program, ("lights[" + std::to_string(i) + "].Quadratic").c_str()), quadratic);
        }
        glUniform3fv(glGetUniformLocation(modelLightingPass.Program, "viewPos"), 1, &camera.Position[0]);
        // Finally render quad
        RenderQuad();

        

        //modelShader.Use();
        //// Draw the loaded model
        //glUniform3f(glGetUniformLocation(modelShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        ////glUniform3f(glGetUniformLocation(modelShader.Program, "objectColor"), objectColor.x, objectColor.y, objectColor.z);
        //glUniform3f(glGetUniformLocation(modelShader.Program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
        //if (lightMode == 1)
        //{
        //    glUniform3f(glGetUniformLocation(modelShader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        //    glUniform1f(glGetUniformLocation(modelShader.Program, "light.constant"), 1.0f);
        //    glUniform1f(glGetUniformLocation(modelShader.Program, "light.linear"), 0.09);
        //    glUniform1f(glGetUniformLocation(modelShader.Program, "light.quadratic"), 0.032);
        //}
        //if(lightMode == 2)
        //    glUniform3f(glGetUniformLocation(modelShader.Program, "light.direction"), -lightDirection.x, -lightDirection.y, -lightDirection.z);
        ////glUniform1f(glGetUniformLocation(modelShader.Program, "lightIntensity"), lightIntensity * 10.0f);
        //glUniform1i(glGetUniformLocation(modelShader.Program, "lightMode"), lightMode);
        //glUniform3f(glGetUniformLocation(modelShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        //glUniform3f(glGetUniformLocation(modelShader.Program, "material.ambient"), ambientMaterial.x, ambientMaterial.y, ambientMaterial.z);
        //glUniform3f(glGetUniformLocation(modelShader.Program, "material.diffuse"), diffuseMaterial.x, diffuseMaterial.y, diffuseMaterial.z);
        //glUniform3f(glGetUniformLocation(modelShader.Program, "material.specular"), specularMaterial.x, specularMaterial.y, specularMaterial.z);
        //glUniform1f(glGetUniformLocation(modelShader.Program, "material.shininess"), shineAmount);

        //// Transformation matrices
        //glm::mat4 model;
        //model = glm::translate(model, glm::vec3(-0.5f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        //model = glm::rotate(model, rotationAngle, glm::vec3(rotX, rotY, rotZ));
        //model = glm::scale(model, modelScale);	// It's a bit too big for our scene, so scale it down
        //glUniformMatrix4fv(glGetUniformLocation(modelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //glm::mat4 projection = glm::perspective(camera.Zoom, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 view = camera.GetViewMatrix();
        //glUniformMatrix4fv(glGetUniformLocation(modelShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        //glUniformMatrix4fv(glGetUniformLocation(modelShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        //ourModel.Draw(modelShader);


        // Floor plane
        /*
        //Draw the grid/floor plane
        floorShader.Use();
        glUniform1i(glGetUniformLocation(floorShader.Program, "blinn"), blinn);
        // Floor
        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        */

        
        // Draw skybox as last
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.Use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(modelShader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        

        // Rendering
        ImGui::Render();

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}


void guiSetup()
{
    // GUI positioning
    int offset = 10;
    ImGui::SetWindowSize(ImVec2(guiWidth, SCREEN_WIDTH - 20));
    ImGui::SetWindowPos(ImVec2(SCREEN_WIDTH - guiWidth - offset, 0 + offset));

    // Scene Setup
    if (ImGui::CollapsingHeader("Scene Setup", 0))
    {
        // Rotation and Scale settings
        if (ImGui::TreeNode("Transformation"))
        {
            // Setting Scale
            ImGui::SliderFloat("Scale", &scaleFactor, 0.0f, 5.0f);
            modelScale = glm::vec3(scaleFactor);

            //Setting Rotation
            ImGui::SliderFloat("Rotation", &rotationAngle, 0.0f, 6.0f);
            ImGui::Checkbox("X Axis", &rotX);
            ImGui::Checkbox("Y Axis", &rotY);
            ImGui::Checkbox("Z Axis", &rotZ);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Model"))
        {
            // Model options
            if (ImGui::Button("Shader Ball"))
            {
                ourModel.~Model();
                ourModel.loadModel("models/shaderBall_small.obj");
            }
            if (ImGui::Button("Stanford Dragon"))
            {
                ourModel.~Model();
                ourModel.loadModel("models/dragon_small.obj");
            }
            if (ImGui::Button("Stanford Bunny"))
            {
                ourModel.~Model();
                ourModel.loadModel("models/bunny_small.obj");
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Skybox"))
        {
            // Skybox opions
            if (ImGui::Button("Lake"))
            {
                skyboxPath = "images/lake";
                cubemapTexture = cubemap.configureSkybox(skyboxPath);
            }
            if (ImGui::Button("San Francisco"))
            {
                skyboxPath = "images/san-francisco";
                cubemapTexture = cubemap.configureSkybox(skyboxPath);
            }
            if (ImGui::Button("Rome"))
            {
                skyboxPath = "images/rome";
                cubemapTexture = cubemap.configureSkybox(skyboxPath);
            }
            if (ImGui::Button("Niagara"))
            {
                skyboxPath = "images/niagara";
                cubemapTexture = cubemap.configureSkybox(skyboxPath);
            }
            if (ImGui::Button("Stockholm"))
            {
                skyboxPath = "images/stockholm";
                cubemapTexture = cubemap.configureSkybox(skyboxPath);
            }

            ImGui::TreePop();
        }
        /*static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        if (ImGui::Button("Test Window")) show_test_window ^= 1;
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);*/
    }
    // Shading Properties
    if (ImGui::CollapsingHeader("Shading properties", 0))
    {
        if (ImGui::TreeNode("Shader"))
        {
            //ImGui::ColorEdit3("Object Color", (float*)&objectColor);
            ImGui::ColorEdit3("Ambient", (float*)&ambientMaterial);
            ImGui::ColorEdit3("Diffuse", (float*)&diffuseMaterial);
            ImGui::ColorEdit3("Specular", (float*)&specularMaterial);
            ImGui::SliderFloat("Shininess", &shineAmount, 0.0f, 32.0f);
            
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Preset Materials"))
        {
            if (ImGui::Button("Jade"))
            {
                ambientMaterial = ImColor(34, 57, 40);
                diffuseMaterial = ImColor(138, 227, 161);
                specularMaterial = ImColor(81,81,81);
                shineAmount = 0.1;
            }
            if (ImGui::Button("Obsidian"))
            {
                ambientMaterial = ImColor(14, 13, 17);
                diffuseMaterial = ImColor(47, 43, 57);
                specularMaterial = ImColor(85, 84, 88);
                shineAmount = 0.3;
            }
            if (ImGui::Button("Bronze"))
            {
                ambientMaterial = ImColor(54, 33, 14);
                diffuseMaterial = ImColor(182, 109, 46);
                specularMaterial = ImColor(100, 69, 43);
                shineAmount = 0.2;
            }

            ImGui::TreePop();
        }

    }
    // Lighting 
    if (ImGui::CollapsingHeader("Lighting", 0))
    {
        ImGui::ColorEdit3("Light Color", (float*)&lightColor);
        
        ImGui::RadioButton("Point Light", &lightMode, 1);
        ImGui::RadioButton("Directional Light", &lightMode, 2);
        if (lightMode == 1)
        {
            ImGui::SliderFloat3("Light Position", (float*)&lightPos, 0.0f, 10.0f);
        }
        if (lightMode == 2)
        {
            ImGui::Text("Values will be negated before passing to shader");
            ImGui::SliderFloat3("Light Direction", (float*)&lightDirection, 0.0f, 10.0f);
        }

    }
    // Camera properties
    if (ImGui::CollapsingHeader("Camera", 0))
    {

    }
    // About
    if (ImGui::CollapsingHeader("About", 0))
    {
        ImGui::Text("Arthur");
        ImGui::Text("Real Time OpenGL Rendering");
        ImGui::Text("using C++, OpenGL, GLSL, ImGui, Visual Studio 15");
        ImGui::Text("Developed by Rushil Kekre");
        ImGui::Text("2017");

        if (ImGui::TreeNode("Credits"))
        {
            ImGui::Text("Joey DeVries from learnopengl.com");
            ImGui::Text("Dear ImGui library by Omar Cornut");
            ImGui::Text("Skyboxes from Emil Persson");
            ImGui::Text("Stanford Dragon and Bunny from the Stanford Computer Graphics Laboratory");

            ImGui::TreePop();
        }
    }

}


// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


GLuint loadTexture(GLchar* path)
{
    // Generate texture ID and load texture data 
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;
}


#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;
}

// Checking to see if mouse button is pressed
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        mouseClickActive = true;
    else
        mouseClickActive = false;

}

// Mouse positions to control camera
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (xpos < SCREEN_WIDTH - guiWidth)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        if (mouseClickActive)
            camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// Scrolling to control Zoom 
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

#pragma endregion