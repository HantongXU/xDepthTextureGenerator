#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#define GLEW_STATIC
#include <glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture.h"

struct Vertex
{
	glm::vec3 m_pos;
	glm::vec2 m_tex;
	glm::vec3 m_normal;

	Vertex() {}

	Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal)
	{
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
	}
};

class Mesh
{
public:
	Mesh() {};
	~Mesh() {
		Clear();
	};
	bool LoadMesh(const std::string& Filename, bool isNormalize);
	void Render();

	std::vector<Vertex> m_vertices;

private:
	bool InitFromScene(const aiScene* pScene, const std::string& Filename, bool isNormalize);
	void InitMesh(unsigned int Index, const aiMesh* paiMesh, bool isNormalize);
	bool InitMaterials(const aiScene* pScene, const std::string& Filename);
	void Clear();

	struct MeshEntry {
		MeshEntry();
		~MeshEntry();

		bool Init(const std::vector<Vertex>& Vertices,
			const std::vector<unsigned int>& Indices);

		GLuint VB;
		GLuint IB;

		unsigned int NumIndices;
		unsigned int MaterialIndex;
	};

	std::vector<MeshEntry> m_Entries;
	std::vector<Texture*> m_Textures;
};



