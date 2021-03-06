#pragma once

#ifndef SKYBOX_H
#define SKYBOX_H

//for openGL headers
#include<GL/glew.h> 

#include<string>
#include<vector>

// Image loading Libs
#include <SOIL.h>

// GLFW Header
#include <GLFW/glfw3.h>

class Skybox
{
public:
    Skybox()
    {

    }
    ~Skybox()
    {

    }

    GLuint configureSkybox(std::string path)
    {
        std::string right = "/right.jpg";
        std::string left = "/left.jpg";
        std::string top = "/top.jpg";
        std::string bottom = "/bottom.jpg";
        std::string back = "/back.jpg";
        std::string front = "/front.jpg";
        std::string finalRight = path + right;
        std::string finalLeft = path + left;
        std::string finalTop = path + top;
        std::string finalBottom = path + bottom;
        std::string finalBack = path + back;
        std::string finalFront = path + front;

        //cout << finalRight << endl;

        // Cubemap (Skybox)
        vector<const GLchar*> faces;
        faces.push_back(finalRight.c_str());
        faces.push_back(finalLeft.c_str());
        faces.push_back(finalTop.c_str());
        faces.push_back(finalBottom.c_str());
        faces.push_back(finalBack.c_str());
        faces.push_back(finalFront.c_str());
        GLuint tex = loadCubemap(faces);

        //cout << tex << endl;

        return tex;
    }

private:

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
        GLuint skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glBindVertexArray(0);
    }

    // Loads a cubemap texture from 6 individual texture faces
    // Order should be:
    // +X (right)
    // -X (left)
    // +Y (top)
    // -Y (bottom)
    // +Z (front)
    // -Z (back)
    GLuint loadCubemap(vector<const GLchar*> faces)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);

        int width, height;
        unsigned char* image;

        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        for (GLuint i = 0; i < faces.size(); i++)
        {
            image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            SOIL_free_image_data(image);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        //cout << textureID << endl;

        return textureID;
    }


};

#endif
