#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL\glew.h>
#include <stb_image_aug.h>

#include <iostream>
#include <string>


class Texture
{
public:
    GLuint texID;
    std::string name;

    Texture()
    {

    }

    ~Texture()
    {
        glDeleteTextures(1, &this->texID);
    }
    
    GLuint loadTexture(GLchar* path, std::string name)
    {
        this->name = name;
        // Generate texture ID and load texture data 
        glGenTextures(1, &this->texID);
        int width, height;
        unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
        // Assign texture to ID
        glBindTexture(GL_TEXTURE_2D, this->texID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        SOIL_free_image_data(image);
        
        return this->texID;
    }

    GLuint getTextureID()
    {
        return this->texID;
    }
};


#endif // !_TEXTURE_H_

