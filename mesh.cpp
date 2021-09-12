#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <cmath>

#include "mesh.h"
#include "utils.h"

#define EPS 1.0e-24

Mesh::MeshEntry::MeshEntry()
{
	VB = INVALID_OGL_VALUE;
	IB = INVALID_OGL_VALUE;
	NumIndices = 0;
	MaterialIndex = INVALID_MATERIAL;
}

Mesh::MeshEntry::~MeshEntry()
{
	if (VB != INVALID_OGL_VALUE)
	{
		glDeleteBuffers(1, &VB);
	}

	if (IB != INVALID_OGL_VALUE)
	{
		glDeleteBuffers(1, &IB);
	}
}

bool Mesh::MeshEntry::Init(const std::vector<Vertex>& Vertices,
	const std::vector<unsigned int>& Indices)
{
	NumIndices = Indices.size();

	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(),
		&Vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices,
		&Indices[0], GL_STATIC_DRAW);

	return true;
}

void Mesh::Clear()
{
}

bool Mesh::LoadMesh(const std::string& Filename, bool isNormalize)
{
	Clear();

	bool Ret = false;

	Assimp::Importer Importer;

	const aiScene* pScene = Importer.ReadFile(Filename.c_str(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs | aiProcess_ValidateDataStructure);

	if (pScene)
	{
		Ret = InitFromScene(pScene, Filename, isNormalize);
	}
	else
	{
		printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
	}

	return Ret;
}

bool Mesh::InitFromScene(const aiScene* pScene, const std::string& Filename, bool isNormalize)
{
	//std::cout << "pScene->mNumMeshes:  " << pScene->mNumMeshes << std::endl;
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);

	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh, isNormalize);
	}
	//return true;
	return InitMaterials(pScene, Filename);
}

void Mesh::InitMesh(unsigned int Index, const aiMesh* paiMesh, bool isNormalize)
{
	m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;

	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	float minX = 65535.0, maxX = -65535.0,
		minY = 65535.0, maxY = -65535.0,
		minZ = 65535.0, maxZ = -65535.0,
		centerX, centerY, centerZ;
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{

		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
		/*if (i == 0) {
			std::cout << "pTexCoord: " << pTexCoord->x << " " << pTexCoord->y << "\n";
		}*/
		if (pPos->x < minX)
			minX = pPos->x;
		if (pPos->y < minY)
			minY = pPos->y;
		if (pPos->z < minZ)
			minZ = pPos->z;

		if (pPos->x > maxX)
			maxX = pPos->x;
		if (pPos->y > maxY)
			maxY = pPos->y;
		if (pPos->z > maxZ)
			maxZ = pPos->z;

		Vertex v(glm::vec3(pPos->x, pPos->y, pPos->z),
			glm::vec2(pTexCoord->x, pTexCoord->y),
			glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		Vertices.push_back(v);
	}
	//std::cout << "minZ: " << minZ << std::endl;
	//std::cout << "maxZ: " << maxZ << std::endl;
	//std::cout << "centerX: " << (maxX + minX) / 2.0 << std::endl;
	//std::cout << "centerX: " << (maxY + minY) / 2.0 << std::endl;
	//std::cout << "centerX: " << (maxZ + minZ) / 2.0 << std::endl;
	//std::exit(0);
	if (isNormalize == true)
	{
		centerX = (minX + maxX) / 2.0;
		centerY = (minY + maxY) / 2.0;
		centerZ = (minZ + maxZ) / 2.0;

		for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
		{
			Vertices[i].m_pos.x -= centerX;
			Vertices[i].m_pos.y -= centerY;
			Vertices[i].m_pos.z -= centerZ;
		}
		float rangeX = maxX - minX, rangeY = maxY - minY, rangeZ = maxZ - minZ;
		float range = std::max(rangeX, std::max(rangeY, rangeZ)) / 2.0;
		std::cout << "range: " << range << std::endl;
		for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
		{
			Vertices[i].m_pos.x /= range;
			Vertices[i].m_pos.y /= range;
			Vertices[i].m_pos.z /= range;
		}
	}
	//std::cout << "vertex num: " << Vertices.size() << std::endl;
	
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		Vertices[i].m_pos.x /= 1;
		Vertices[i].m_pos.y /= 1;
		Vertices[i].m_pos.z /= 1;
		m_vertices.push_back(Vertices[i]);
	}
	/*std::cout << Vertices[3].m_pos.x << " " << Vertices[3].m_pos.y << " " << Vertices[3].m_pos.z << std::endl;
	std::cout << Vertices[4].m_pos.x << " " << Vertices[4].m_pos.y << " " << Vertices[4].m_pos.z << std::endl;
	std::cout << Vertices[5].m_pos.x << " " << Vertices[5].m_pos.y << " " << Vertices[5].m_pos.z << std::endl;*/
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		//std::cout << Face.mIndices[0] << std::endl;
		//std::cout << Face.mIndices[1] << std::endl;
		//std::cout << Face.mIndices[2] << std::endl;
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
	//std::cout << "Indices.size: " << Indices.size() << std::endl;
	m_Entries[Index].Init(Vertices, Indices);

#if 0
	{
		float fovy = 60.0f / 180.0f * 3.1415926f;
		int width = 640, height = 480;
		float fy = height / 2.0f / tan(fovy / 2.0f);
		float fx = fy;
		float cx = (width - 1.0f) / 2.0f,
			cy = (height - 1.0f) / 2.0f;

		std::ifstream fs;
		fs.open("D:\\xjm\\xDepthTextureGenerator1.0\\output\\camera_pose.txt", std::ifstream::binary);

		std::vector<glm::mat4> cameraPoseVec;
		for (int i = 0; i < 3000; ++i)
		{
			glm::mat4 pose;
			fs.read((char *)&pose[0][0], sizeof(glm::mat4));
			cameraPoseVec.push_back(pose);
		}

		std::ofstream outfs;
		outfs.open("D:\\xjm\\xDepthTextureGenerator1.0\\test3.ply");

		outfs << "ply";
		outfs << "\nformat " << "ascii" << " 1.0";
		outfs << "\nelement vertex " << Vertices.size();
		outfs << "\nproperty float x"
			"\nproperty float y"
			"\nproperty float z";
		outfs << "\nproperty uchar red"
			"\nproperty uchar green"
			"\nproperty uchar blue";
		outfs << "\nproperty float nx"
			"\nproperty float ny"
			"\nproperty float nz";
		outfs << "\nelement face " << Indices.size() / 3;
		outfs << "\nproperty list uchar int vertex_indices";
		outfs << "\nend_header\n";

		glm::vec4 v, v2;
		for (unsigned int i = 0; i < Vertices.size(); i++)
		{
			v.x = Vertices[i].m_pos.x;
			v.y = Vertices[i].m_pos.y;
			v.z = Vertices[i].m_pos.z;
			v.w = 1.0f;
			v2 = glm::inverse(cameraPoseVec[0]) * v;
			outfs << v2.x << " " << v2.y << " " << v2.z << " "
				<< (int)255 << " " << (int)0 << " " << (int)0 << " "
				<< 1 << " " << 0 << " " << 0
				<< std::endl;
		}

		for (unsigned int i = 0; i < Indices.size() / 3; i++)
		{
			outfs << 3 << " " << Indices[3 * i] << " " << Indices[3 * i + 1] << " " << Indices[3 * i + 2] << std::endl;
		}

		outfs.close();
		std::exit(0);
	}
#endif
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
	bool Ret = true;

	// Инициализируем материал
	for (unsigned int i = 0; i < pScene->mNumMaterials-1; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		m_Textures[i] = NULL;
		if (1)//pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			//std::cout << "i:" << i << "\n";
			aiString Path;


				char path[256];
				sprintf(path, "G:\\xht\\huawei\\2019-04-25_15.49.41\\obj-origin\\textured_material%04d_map_Kd.png", i);
				//std::cout << "path:"<< path << "\n";
				//std::string FullPath = std::string("G:\\xht\\ChairModel\\Model.jpg");
				m_Textures[i] = new Texture(GL_TEXTURE_2D, path);

				if (!m_Textures[i]->Load())
				{
					printf("Error loading texture '%s'\n", path);
					delete m_Textures[i];
					m_Textures[i] = NULL;
					Ret = false;
				}
		}
		// Загружаем белую текстуру если модель не имеет собственной
		if (!m_Textures[i])
		{
			//m_Textures[i] = new Texture(GL_TEXTURE_2D, "D:\\xjm\\xDepthTextureGenerator1.0\\chair_model\\Model.jpg");
			m_Textures[i] = new Texture(GL_TEXTURE_2D, "G:\\xht\\ChairModel\\Model.jpg");
			Ret = m_Textures[i]->Load();
			std::cout << Ret << std::endl;
		}
	}

	return Ret;
}

void Mesh::Render()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		
		glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VB);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);
		const unsigned int MaterialIndex = m_Entries[i].MaterialIndex-1;
		//std::cout << "MaterialIndex: " << MaterialIndex << "\n";
		if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex])
		{
			m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
		}

		glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
		//std::cout << "m_Entries[i].NumIndices: " << m_Entries[i].NumIndices << std::endl;
	}

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}
