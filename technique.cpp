#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <iostream>

#include "technique.h"

Technique::Technique(){
    m_shaderProg = 0;
}

Technique::~Technique(){
    for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++){
        glDeleteShader(*it);
    }

    if (m_shaderProg != 0){
        glDeleteProgram(m_shaderProg);
        m_shaderProg = 0;
    }
}

bool Technique::Init(){
    m_shaderProg = glCreateProgram();

    if (m_shaderProg == 0){
        fprintf(stderr, "Error creating shader program\n");
        return false;
    }	

	const char *pVS = shaderSourceFromFile("G:\\xht\\xDepthTextureGenerator1.0\\shader\\verShader.glsl");
	const char *pFS = shaderSourceFromFile("G:\\xht\\xDepthTextureGenerator1.0\\shader\\fragShader.glsl");

	if (!AddShader(GL_VERTEX_SHADER, pVS)) {
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, pFS)) {
		return false;
	}

	if (!Finalize()) {
		return false;
	}

	m_modelViewMatrix = GetUniformLocation("modelViewMatrix");
	m_projectionMatrix = GetUniformLocation("projectionMatrix");	
	m_samplerLocation = GetUniformLocation("sampler");

	if (m_modelViewMatrix == INVALID_UNIFORM_LOCATION ||
		m_projectionMatrix == INVALID_UNIFORM_LOCATION) 
	{
		std::cout << "invalid uniform location" << std::endl;
		return false;
	}

    return true;
}

bool Technique::AddShader(GLenum ShaderType, const char* pShaderText){
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0){
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        return false;
    }

    m_shaderObjList.push_back(ShaderObj);

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success){
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        return false;
    }

    glAttachShader(m_shaderProg, ShaderObj);

    return true;
}

// После добавления всех шейдеров в программу вызываем эту функцию
// для линковки и проверки программу на ошибки
bool Technique::Finalize(){
    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};

    glLinkProgram(m_shaderProg);

    glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
    if (Success == 0){
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        return false;
    }

    glValidateProgram(m_shaderProg);
    glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
    if (Success == 0){
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        return false;
    }

    // Удаляем промежуточные объекты шейдеров, которые были добавлены в программу
    for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++){
        glDeleteShader(*it);
    }

    m_shaderObjList.clear();

    return true;
}

void Technique::Enable(){
    glUseProgram(m_shaderProg);
}

GLint Technique::GetUniformLocation(const char* pUniformName){
    GLint Location = glGetUniformLocation(m_shaderProg, pUniformName);

    if ((unsigned int)Location == 0xFFFFFFFF){
                fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}

void Technique::SetModelView(const glm::mat4& mv)
{
	glUniformMatrix4fv(m_modelViewMatrix, 1, GL_FALSE, (const GLfloat*)&mv[0][0]);
}


void Technique::SetProjection(const glm::mat4& proj)
{
	glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, (const GLfloat*)&proj[0][0]);
}

void Technique::SetTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(m_samplerLocation, TextureUnit);
}

const char *Technique::shaderSourceFromFile(const char *fileName)
{
	FILE *in;

	in = fopen(fileName, "rb");
	fseek(in, 0, SEEK_END);
	unsigned size = ftell(in);
	fseek(in, 0, SEEK_SET);

	char *source = new char[size + 1];

	fread(source, sizeof(char), size, in);
	source[size] = '\0';
	fclose(in);

	return const_cast<const char *>(source);
}

















