// ARTHUR
// Real Time OpenGL Renderer
// By Rushil Kekre
// C++, OpenGL, GLSL, ImGUI

// ================================

// HEADER FILES

// Standard C++ Headers
#include <string>
#include <random>

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
#include "Texture.h"

// GLM Mathemtics Header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Image loading Header
#include <stb_image_aug.h>

// ImGUI Headers
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

// ================================

// FUNCTION PROTOTYPES

// Setup functions for various purposes
// guiSetup() to initialize ImGUI and UI menu options
// gBufferInit(), ssaoInit() and pbrInit() to initialize framebuffers for the gBuffer, SSAO and PBR pipelines respectively
void guiSetup();
void gBufferInit();
void ssaoInit();
void pbrInit();
void skyboxInit();
void fixScreenSize(GLFWwindow* window);

// Callback functions for user interaction
// key_callback() for keyboard input
// scroll_callback(), mouse_button_callback() and mouse_callback() to handle mouse inputs
// Do_Movement() to collect user input to be processed
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Shape functions to render Spheres, Quads and Cubes
void RenderCube();
void RenderQuad();
void RenderSphere();

// Computation function to calculate linear interpolation
GLfloat lerp(GLfloat a, GLfloat b, GLfloat f);

// ================================

// WINDOW PROPERTIES
// GLFW window creation and screen size specification
GLFWwindow* window;
GLuint SCREEN_WIDTH = 1920, SCREEN_HEIGHT = 1080;

// ================================

// IMGUI GLOBAL VARIABLES AND DECLARATIONS
// All variables that are used with ImGUI to control object behavior

// Size of the menu
int guiWidth = 400;

// Transform variables to handle rotation and scaling
GLfloat scaleFactor = 2.0f;
glm::vec3 modelScale(2.0f);
GLfloat rotationAngle = 0.2;
bool rotX = false;
bool rotY = true;
bool rotZ = false;

// Shading variables to control material properties for forward rendering
//ImVec4 objectColor = ImColor(175, 175, 175);
ImVec4 ambientMaterial = ImColor(0, 0, 0);
ImVec4 diffuseMaterial = ImColor(255, 255, 255);
ImVec4 specularMaterial = ImColor(127, 127, 127);
float shineAmount = 0.25;

// Lighting
ImVec4 lightColor = ImColor(255, 255, 255);

// Rendering flags to select rendering methods and operations to be performed
bool deferredRendering = false;
bool forwardRendering = false;
bool deferredActive = true;
bool ssaoActive = false;
bool pbrActive = true;


// SSAO variables
int num_samples = 64;
int kernelSize = 64;
float ssaoRadius = 0.5;
float ssaoBias = 0.025;
int power = 2;

// PBR variables
float metallic = 0.5;
float roughness = 0.5;


// ================================

// MAIN GLOBAL VARIABLES

// Camera and mouse control variables
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
bool mouseClickActive;

// Time
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// gBuffer
GLuint gBuffer;
GLuint gPosition, gNormal, gAlbedo;

// SSAO
GLuint ssaoFBO, ssaoBlurFBO; 
GLuint ssaoColorBuffer, ssaoColorBufferBlur;
std::vector<glm::vec3> ssaoKernel;
GLuint noiseTexture;

// PBR
GLuint hdrTexture;
GLuint envCubemap;
GLuint captureFBO;
GLuint captureRBO;
GLuint irradianceMap;
GLuint prefilterMap;
GLuint brdfLUTTexture;

// Model
Model ourModel;
GLboolean blinn = false;

// Skybox
GLuint skyboxVAO, skyboxVBO;
std::string skyboxPath;
Skybox cubemap;
GLuint cubemapTexture;

// Light
glm::vec3 lightPos(0.0f, 1.0f, 2.5f);
GLfloat lightIntensity = 0.5f;
glm::vec3 lightDirection(1.0f, 3.0f, 3.0f);
int lightMode = 1;
int attenuationMode = 1;

// Textures
// PBR texture variables
Texture objectAlbedo;
Texture objectMetallic;
Texture objectRoughness;
Texture objectNormal;
Texture objectAO;
// Environment map variable
Texture envHDR;

// Shaders
Shader gridShader;
Shader lightSource;
Shader modelShader;
Shader modelReflection;
Shader skyboxShader;
Shader floorShader;
Shader modelGeometryPass;
Shader modelLightingPass;
Shader ssaoShader;
Shader ssaoBlurShader;
Shader pbrShader;
Shader rectToCubemap;
Shader irradianceShader;
Shader prefilterShader;
Shader brdfShader;
Shader backgroundShader;



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
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Arthur", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);

    // Setting callback functions for keyboard/mouse input
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    // List of shaders
    gridShader.loadShader("shaders/gridTexture.vert", "shaders/gridTexture.frag");
    lightSource.loadShader("shaders/lightSource.vert", "shaders/lightSource.frag");
    modelShader.loadShader("shaders/model_loading.vert", "shaders/model_loading.frag");
    modelReflection.loadShader("shaders/model_reflection.vert", "shaders/model_reflection.frag");
    skyboxShader.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
    floorShader.loadShader("shaders/floorShader.vert", "shaders/floorShader.frag");
    modelGeometryPass.loadShader("shaders/model_geometry.vert", "shaders/model_geometry.frag");
    modelLightingPass.loadShader("shaders/model_lighting.vert", "shaders/model_lighting.frag");
    ssaoShader.loadShader("shaders/model_lighting.vert", "shaders/ssaoShader.frag");
    ssaoBlurShader.loadShader("shaders/model_lighting.vert", "shaders/ssaoBlur.frag");
    pbrShader.loadShader("shaders/pbrShader.vert", "shaders/pbrShader.frag");
    rectToCubemap.loadShader("shaders/rectToCubemap.vert", "shaders/rectToCubemap.frag");
    irradianceShader.loadShader("shaders/rectToCubemap.vert", "shaders/pbrIrradiance.frag");
    prefilterShader.loadShader("shaders/rectToCubemap.vert", "shaders/prefilter.frag");
    brdfShader.loadShader("shaders/brdf.vert", "shaders/brdf.frag");
    backgroundShader.loadShader("shaders/background.vert", "shaders/background.frag");

    // configure skybox
    skyboxInit();

    //Setting skybox default path
    skyboxPath = "images/san-francisco";
    cubemapTexture = cubemap.configureSkybox(skyboxPath);

    // Setting default model
    ourModel.loadModel("models/shaderball_small.obj");
    
    // configure g-buffer framebuffer
    gBufferInit();

    // configure framebuffer for SSAO
    ssaoInit();
    
    // lighting info
    glm::vec3 ssaoLightPos = glm::vec3(0.0, 12.0, 0.0);
    glm::vec3 ssaoLightColor = glm::vec3(1.0, 1.0, 1.0);

    // shader configuration
    modelLightingPass.Use();
    modelLightingPass.setInt("gPosition", 0);
    modelLightingPass.setInt("gNormal", 1);
    modelLightingPass.setInt("gAlbedo", 2);
    modelLightingPass.setInt("ssao", 3);
    ssaoShader.Use();
    ssaoShader.setInt("gPosition", 0);
    ssaoShader.setInt("gNormal", 1);
    ssaoShader.setInt("texNoise", 2);
    ssaoBlurShader.Use();
    ssaoBlurShader.setInt("ssaoInput", 0);

    // PBR Shader configuration
    pbrShader.Use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);
    pbrShader.setInt("albedoMap", 3);
    pbrShader.setInt("normalMap", 4);
    pbrShader.setInt("metallicMap", 5);
    pbrShader.setInt("roughnessMap", 6);
    pbrShader.setInt("aoMap", 7);

    // PBR texture loading
    objectAlbedo.loadTexture("images/rustediron/albedo.png", "albedo");
    objectNormal.loadTexture("images/rustediron/normal.png", "normal");
    objectMetallic.loadTexture("images/rustediron/metallic.png", "metallic");
    objectRoughness.loadTexture("images/rustediron/roughness.png", "roughness");
    objectAO.loadTexture("images/rustediron/ao.png", "ao");

    // HDR Background
    backgroundShader.Use();
    backgroundShader.setInt("environmentMap", 0);

    // Load HDR texture
    hdrTexture = envHDR.loadHDR("images/loft/Newport_Loft_Ref_Flip.hdr", "loft");
    
    // PBR setup
    pbrInit();
    
    glm::vec3 lightPositions[] = {
        glm::vec3(-5.0f,  5.0f, 5.0f),
        glm::vec3(5.0f,  5.0f, 5.0f),
        glm::vec3(0.0f, 5.0f, -5.0f),
        glm::vec3(0.0f, -10.0f, 0.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // Initializing static shader uniforms before rendering
    glm::mat4 projection;
    projection = glm::perspective(camera.Zoom, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    pbrShader.Use();
    pbrShader.setMat4("projection", projection);
    backgroundShader.Use();
    backgroundShader.setMat4("projection", projection);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    fixScreenSize(window);

    // ================================

    // GAME LOOP
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
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model;
        glm::mat4 view;
        view = camera.GetViewMatrix();
        model = glm::translate(model, glm::vec3(-0.5f, -1.0f, 0.0f)); // Translating to center of screen
        model = glm::rotate(model, rotationAngle, glm::vec3(rotX, rotY, rotZ));
        model = glm::scale(model, modelScale);	// Scaling with ImGui

        // GUI
        guiSetup();

        if (pbrActive)
        {
            pbrShader.Use();
            pbrShader.setMat4("view", view);
            pbrShader.setVec3("camPos", camera.Position);

            // bind pre-computed IBL data
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

            // PBR textures            
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, objectAlbedo.getTextureID());
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, objectNormal.getTextureID());
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, objectMetallic.getTextureID());
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, objectRoughness.getTextureID());
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, objectAO.getTextureID());

            pbrShader.setMat4("model", model);
            RenderSphere();
            //ourModel.Draw(pbrShader);
            
            for (GLuint i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
            {
                glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
                newPos = lightPositions[i];
                pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
                pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

                lightSource.Use();
                lightSource.setMat4("projection", projection);
                lightSource.setMat4("view", view);
                model = glm::mat4();
                model = glm::translate(model, lightPositions[i]);
                model = glm::scale(model, glm::vec3(0.5f)); // Make it a smaller cube
                lightSource.setMat4("model", model);
                RenderSphere();
            }


            // render skybox (render as last to prevent overdraw)
            glDepthFunc(GL_LEQUAL);
            backgroundShader.Use();
            backgroundShader.setMat4("view", view);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            RenderCube();
        }

        if (deferredRendering) 
        {
            // Deferred Rendering
            // 1. Geometry Pass: render scene's geometry/color data into gbuffer
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            modelGeometryPass.Use();
            modelGeometryPass.setMat4("projection", projection);
            modelGeometryPass.setMat4("view", view);
            modelGeometryPass.setBool("invertedNormals", 0);
            // model
            modelGeometryPass.setMat4("model", model);
            ourModel.Draw(modelGeometryPass);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            if (ssaoActive)
            {
                // 2. generate SSAO texture
                glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
                glClear(GL_COLOR_BUFFER_BIT);
                ssaoShader.Use();
                // Send kernel + rotation 
                ssaoShader.setInt("kernelSize", kernelSize);
                ssaoShader.setFloat("radius", ssaoRadius);
                ssaoShader.setFloat("bias", ssaoBias);
                ssaoShader.setInt("power", power);
                for (GLuint i = 0; i < 64; ++i)
                    ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
                ssaoShader.setMat4("projection", projection);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gPosition);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gNormal);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, noiseTexture);
                RenderQuad();
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // 3. blur SSAO texture to remove noise
                glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
                glClear(GL_COLOR_BUFFER_BIT);
                ssaoBlurShader.Use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
                RenderQuad();
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
            

            // 2. Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            modelLightingPass.Use();
            // send light relevant uniforms
            glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(ssaoLightPos, 1.0));
            modelLightingPass.setVec3("light.Position", lightPosView);
            modelLightingPass.setVec3("light.Color", ssaoLightColor);
            modelLightingPass.setVec3("viewPos", camera.Position);
            // Update attenuation parameters
            const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
            const float linear = 0.09;
            const float quadratic = 0.032;
            modelLightingPass.setFloat("light.Linear", linear);
            modelLightingPass.setFloat("light.Quadratic", quadratic);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedo);
            glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
            glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
            RenderQuad();

            modelLightingPass.setBool("deferredActive", deferredActive);
            modelLightingPass.setBool("ssaoActive", ssaoActive);
        }
        else if(forwardRendering)
        {
            modelShader.Use();
            // Draw the loaded model
            modelShader.setVec3("cameraPos", camera.Position);
            modelShader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);
            if (lightMode == 1)
            {
                modelShader.setVec3("lightPos", lightPos);
                modelShader.setFloat("light.constant", 1.0);
                modelShader.setFloat("light.linear", 0.09);
                modelShader.setFloat("light.quadratic", 0.032);
            }
            if (lightMode == 2)
                modelShader.setVec3("light.direction", -lightDirection);
            modelShader.setInt("lightMode", lightMode);
            modelShader.setVec3("viewPos", camera.Position);
            modelShader.setVec3("material.ambient", ambientMaterial.x, ambientMaterial.y, ambientMaterial.z);
            modelShader.setVec3("material.diffuse", diffuseMaterial.x, diffuseMaterial.y, diffuseMaterial.z);
            modelShader.setVec3("material.specular", specularMaterial.x, specularMaterial.y, specularMaterial.z);
            modelShader.setFloat("material.shininess", shineAmount);

            // Transformation matrices
            modelShader.setMat4("projection", projection);
            modelShader.setMat4("view", view);
            modelShader.setMat4("model", model);
            
            ourModel.Draw(modelShader);

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

        }
        
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
                ourModel.loadModel("models/shaderBall_small2.obj");
            }
            if (ImGui::Button("Stanford Dragon"))
            {
                ourModel.~Model();
                ourModel.loadModel("models/dragon_small2.obj");
            }
            if (ImGui::Button("Stanford Bunny"))
            {
                ourModel.~Model();
                ourModel.loadModel("models/bunny_small2.obj");
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
    }
    /*
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
    // Rendering properties
    if (ImGui::CollapsingHeader("Rendering", 1))
    {
        if (ImGui::Button("Deferred Rendering"))
        {
            deferredRendering = true;
            forwardRendering = false;
            pbrActive = false;
        }
        if (ImGui::Button("Forward Rendering"))
        {
            deferredRendering = false;
            forwardRendering = true;
            pbrActive = false;
        }
        if (ImGui::TreeNode("SSAO"))
        {
            if (ImGui::Button("Enable"))
            {
                ssaoActive = true;
            }
            if (ImGui::Button("Disable"))
            {
                ssaoActive = false;
            }

            ImGui::SliderInt("Kernel Size", &kernelSize, 1, 128);
            ImGui::SliderFloat("Radius", &ssaoRadius, 0.0, 1.0);
            ImGui::SliderFloat("Bias", &ssaoBias, 0.0, 1.0);
            ImGui::SliderInt("Power", &power, 1, 10);

            ImGui::TreePop();
        }
    }
    //PBR
    if (ImGui::CollapsingHeader("PBR", 0))
    {
        if (ImGui::Button("Enable"))
        {
            pbrActive = true;
            deferredRendering = false;
            forwardRendering = false;
        }
        
        if (ImGui::TreeNode("Controls"))
        {
            ImGui::SliderFloat("Metallic", &metallic, 0.0, 1.0);
            ImGui::SliderFloat("Roughness", &roughness, 0.0, 1.0);

            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Materials"))
        {
            if (ImGui::Button("Rusted Iron"))
            {
                objectAlbedo.loadTexture("images/rustediron/albedo.png", "albedo");
                objectNormal.loadTexture("images/rustediron/normal.png", "normal");
                objectMetallic.loadTexture("images/rustediron/metallic.png", "metallic");
                objectRoughness.loadTexture("images/rustediron/roughness.png", "roughness");
                objectAO.loadTexture("images/rustediron/ao.png", "ao");
            }
            if (ImGui::Button("Gold"))
            {
                objectAlbedo.loadTexture("images/gold/albedo_boosted.png", "albedo");
                objectNormal.loadTexture("images/gold/normal.png", "normal");
                objectMetallic.loadTexture("images/gold/metallic.png", "metallic");
                objectRoughness.loadTexture("images/gold/roughness.png", "roughness");
                objectAO.loadTexture("images/gold/ao.png", "ao");
            }
            if (ImGui::Button("Concrete"))
            {
                objectAlbedo.loadTexture("images/concrete/albedo.png", "albedo");
                objectNormal.loadTexture("images/concrete/normal.png", "normal");
                objectMetallic.loadTexture("images/concrete/metallic.png", "metallic");
                objectRoughness.loadTexture("images/concrete/roughness.png", "roughness");
                objectAO.loadTexture("images/concrete/ao.png", "ao");
            }
            if (ImGui::Button("Plastic"))
            {
                objectAlbedo.loadTexture("images/plastic/albedo.png", "albedo");
                objectNormal.loadTexture("images/plastic/normal.png", "normal");
                objectMetallic.loadTexture("images/plastic/metallic.png", "metallic");
                objectRoughness.loadTexture("images/plastic/roughness.png", "roughness");
                objectAO.loadTexture("images/plastic/ao.png", "ao");
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Environments"))
        {
            if (ImGui::Button("Newport Loft"))
            {
                hdrTexture = envHDR.loadHDR("images/loft/Newport_Loft_Ref_Flip.hdr", "loft");
                pbrInit();
            }
            if (ImGui::Button("Industrial Hall"))
            {
                hdrTexture = envHDR.loadHDR("images/industrial-hall/industrial_Ref_Flip.hdr", "industrial");
                pbrInit();
            }
            if (ImGui::Button("Winter Forest"))
            {
                hdrTexture = envHDR.loadHDR("images/winter-forest/WinterForest_Ref_Flip.hdr", "forest");
                pbrInit();
            }
            if (ImGui::Button("City Night"))
            {
                hdrTexture = envHDR.loadHDR("images/city-night/CityNight_Ref_Flip.hdr", "city");
                pbrInit();
            }

            fixScreenSize(window);

            ImGui::TreePop();
        }

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
    */
}

void gBufferInit()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void ssaoInit()
{
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0;

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // generate noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


void pbrInit()
{
    // pbr: setup framebuffer
    // ----------------------
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    rectToCubemap.Use();
    rectToCubemap.setInt("equirectangularMap", 0);
    rectToCubemap.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (GLuint i = 0; i < 6; ++i)
    {
        rectToCubemap.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
    // --------------------------------------------------------------------------------
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
    // -----------------------------------------------------------------------------
    irradianceShader.Use();
    irradianceShader.setInt("environmentMap", 0);
    irradianceShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (GLuint i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
    // --------------------------------------------------------------------------------
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilterShader.Use();
    prefilterShader.setInt("environmentMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    GLuint maxMipLevels = 5;
    for (GLuint mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        GLuint mipWidth = 128 * std::pow(0.5, mip);
        GLuint mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (GLuint i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.Use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void skyboxInit()
{
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
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
}

void fixScreenSize(GLFWwindow* window)
{
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
}


GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}



// renders (and builds at first invocation) a sphere
// -------------------------------------------------
GLuint sphereVAO = 0;
GLuint indexCount;
void RenderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        GLuint vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<GLuint> indices;

        const GLuint X_SEGMENTS = 64;
        const GLuint Y_SEGMENTS = 64;
        const float PI = 3.14159265359;
        for (GLuint y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (GLuint x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
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

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                                                                  // front face
                                                                  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                                                                  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                                                                  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                                                                  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                                                                  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                                                                  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                                                                                                                        // left face
                                                                                                                        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                                                                        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                                                                                                                        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                                                                                                                        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                                                                                                                        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                                                                                                                        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                                                                                                                              // right face
                                                                                                                                                                              1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                                                                                                                                                                              1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                                                                                                                                                                              1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                                                                                                                                                                              1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                                                                                                                                                                              1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                                                                                                                                                                              1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                                                                                                                                                                                                                                   // bottom face
                                                                                                                                                                                                                                   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                                                                                                                                                                                                                                   1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                                                                                                                                                                                                                                   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                                                                                                                                                                                                                                   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                                                                                                                                                                                                                                   -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                                                                                                                                                                                                                                                                                         // top face
                                                                                                                                                                                                                                                                                         -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                                                                                                                                                                                                                                                                                         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                                                                         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                                                                                                                                                                                                                                                                                         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                                                                         -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                                                                                                                                                                                                                                                                                         -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
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


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
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