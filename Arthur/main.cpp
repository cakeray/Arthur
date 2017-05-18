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
GLuint loadTexture(GLchar* path);


// ================================

// WINDOW PROPERTIES
GLuint screenWidth = 1280, screenHeight = 720;

// ================================

// IMGUI GLOBAL VARIABLES AND DECLARATIONS

int guiWidth = 350;

// Transform variables
GLfloat scaleFactor = 2.0f;
glm::vec3 modelScale(2.0f);
GLfloat rotationAngle;
bool rotX = false;
bool rotY = true;
bool rotZ = false;

// Skybox
std::string skyboxPath;
Skybox cubemap;
GLuint cubemapTexture;


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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Arthur", nullptr, nullptr); // Windowed
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
    glViewport(0, 0, screenWidth, screenHeight);

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);


    // List of shaders
    Shader gridShader("shaders/gridTexture.vert", "shaders/gridTexture.frag");
    Shader modelShader("shaders/model_loading.vert", "shaders/model_loading.frag");
    Shader modelReflection("shaders/model_reflection.vert", "shaders/model_reflection.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

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
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    //Setting skybox defaul path
    skyboxPath = "images/lake";
    cubemapTexture = cubemap.configureSkybox(skyboxPath);

    // Load default model
    ourModel.loadModel("models/shaderBall_small.obj");

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
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // GUI
        guiSetup();

        modelShader.Use();
        // Transformation matrices
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(modelShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(modelShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(modelShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        // Draw the loaded model
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::rotate(model, rotationAngle, glm::vec3(rotX, rotY, rotZ));
        model = glm::scale(model, modelScale);	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(modelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
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
    ImGui::SetWindowSize(ImVec2(guiWidth, screenHeight - 20));
    ImGui::SetWindowPos(ImVec2(screenWidth - guiWidth - offset, 0 + offset));

    // Scene Setup
    if (ImGui::CollapsingHeader("Model Setup", 0))
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

    }
    // Textures
    if (ImGui::CollapsingHeader("Textures", 0))
    {

    }
    // Lighting 
    if (ImGui::CollapsingHeader("Lighting", 0))
    {

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
    if (xpos < screenWidth - guiWidth)
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