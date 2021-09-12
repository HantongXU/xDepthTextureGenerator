#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <opencv2\opencv.hpp>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    bool Load();

    void Bind(GLenum TextureUnit);

private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    cv::Mat m_image;
};


#endif	/* TEXTURE_H */

