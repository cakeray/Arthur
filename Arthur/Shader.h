#pragma once
#ifndef SHADER_H
#define SHADER_H

#include<string.h>
#include<fstream>
#include<sstream>
#include<iostream>
using namespace std;

#include<GL/glew.h> //for openGL headers

class Shader
{
public:
    //the program ID
    GLuint Program;
    //constructor reads and builds the shader
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        //retrieve the vertex and fragment source code from the address path
        string vertexCode;
        string fragmentCode;
        ifstream vShaderFile;
        ifstream fShaderFile;

        //ensures ifstream objects can throw exceptions
        vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

        try
        {
            //open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            stringstream vShaderStream, fShaderStream;
            //read files buffer content into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            //close file handlers
            vShaderFile.close();
            fShaderFile.close();
            //convert stream into GLchar array
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (ifstream::failure e)
        {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
        }

        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar* fShaderCode = fragmentCode.c_str();

        //compile shaders
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];

        //vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        //print compile errors, if any
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << endl << infoLog << endl;
        }

        //fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        //print compile errors, if any
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << endl << infoLog << endl;
        }

        //shader program
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        //printing linking errors if any
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << endl << infoLog << endl;
        }

        //delete the shader resources as they are no longer required
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    //use the program
    void Use()
    {
        glUseProgram(this->Program);
    }

private:

};

#endif // !SHADER_H

