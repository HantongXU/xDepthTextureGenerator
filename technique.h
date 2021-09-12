#ifndef TEXHNIQUE_H
#define TEXHNIQUE_H

#define GLEW_STATIC
#include <glew.h>
#include <glm/glm.hpp>
#include <list>

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

class Technique
{
    public:
        Technique();
        virtual ~Technique();
        virtual bool Init();
        void Enable();

		void SetModelView(const glm::mat4& mv);
		void SetProjection(const glm::mat4& proj);
		void SetTextureUnit(unsigned int TextureUnit);

		const char *shaderSourceFromFile(const char *fileName);

    protected:
        bool AddShader(GLenum ShaderType, const char* pShaderText);
        bool Finalize();
        GLint GetUniformLocation(const char* pUniformName);

    private:
        GLuint m_shaderProg;
        typedef std::list<GLuint> ShaderObjList;
        ShaderObjList m_shaderObjList;	

		GLuint m_modelViewMatrix;
		GLuint m_projectionMatrix;
		GLuint m_samplerLocation;
};

#endif /* TEXHNIQUE_H */
