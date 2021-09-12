#include "stdafx.h"

#include <iostream>
#include "texture.h"

Texture::Texture(GLenum TextureTarget, const std::string& FileName)
{
    m_textureTarget = TextureTarget;
    m_fileName      = FileName;
}

bool Texture::Load()
{
	m_image = cv::imread(m_fileName);
	cv::cvtColor(m_image, m_image, CV_BGR2RGBA); 
	//cv::Vec4b rgba = m_image.at<cv::Vec4b>(570, 786);
	//std::cout << "RGBA: " << rgba << std::endl;
    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glTexImage2D(m_textureTarget, 0, GL_RGB, m_image.cols, m_image.rows, -0.5, GL_RGBA, GL_UNSIGNED_BYTE, m_image.data);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
