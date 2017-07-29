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
    GLenum texType, texInternalFormat,texFormat;
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

    GLuint loadHDR(GLchar* path, std::string name)
    {
        this->name = name;
        this->texType = GL_TEXTURE_2D;

        std::string tempPath = std::string(path);

        glGenTextures(1, &this->texID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->texID);

        if (stbi_is_hdr(tempPath.c_str()))
        {
            int width, height, numComponents;
            float* texData = stbi_loadf(tempPath.c_str(), &width, &height, &numComponents, 0);
            
            if (texData)
            {
                // Need a higher precision format for HDR to not lose informations, thus 32bits floating point
                if (numComponents == 3)
                {
                    this->texInternalFormat = GL_RGB32F;
                    this->texFormat = GL_RGB;
                }
                else if (numComponents == 4)
                {
                    this->texInternalFormat = GL_RGBA32F;
                    this->texFormat = GL_RGBA;
                }

                glTexImage2D(GL_TEXTURE_2D, 0, this->texInternalFormat, width, height, 0, this->texFormat, GL_FLOAT, texData);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glGenerateMipmap(GL_TEXTURE_2D);
            }

            else
            {
                std::cerr << "HDR TEXTURE - FAILED LOADING : " << path << std::endl;
            }

            stbi_image_free(texData);
        }

        else
        {
            std::cerr << "HDR TEXTURE - FILE IS NOT HDR : " << path << std::endl;
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        return this->texID;
    }

    GLuint getTextureID()
    {
        return this->texID;
    }
};


#endif // !_TEXTURE_H_

