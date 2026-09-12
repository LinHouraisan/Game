#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "assimp_glm_helpers.h"
#include "animdata.h"

using namespace std;
namespace OpenGL {

	class Model
	{
	public:
		// model data 
		vector<Texture> textures_loaded;	
		vector<Mesh>    meshes;
		string directory;
		bool gammaCorrection;
		std::string modelName;



		// constructor, expects a filepath to a 3D model.
		Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
		{
			loadModel(path);
		}

		// draws the model, and thus all its meshes
		void Draw(Shader& shader)
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
				meshes[i].Draw(shader);
		}

		void DrawInstancedSSBO(Shader& shader, unsigned int instanceCount)
		{
			for (unsigned int i = 0; i < meshes.size(); i++) {
				meshes[i].DrawInstancedSSBO(shader, instanceCount);
			}
		}


		auto& GetBoneInfoMap() { return m_BoneInfoMap; }
		int& GetBoneCount() { return m_BoneCounter; }
		const std::string& GetModelName() const {
			return modelName;
		}

	private:

		std::map<string, BoneInfo> m_BoneInfoMap;
		int m_BoneCounter = 0;

		void loadModel(string const& path) {
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				// cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
				return;
			}

			// 提取模型名称（去掉路径）
			size_t lastSlash = path.find_last_of("/\\");
			size_t lastDot = path.find_last_of(".");
			if (lastSlash == std::string::npos) lastSlash = -1;
			if (lastDot == std::string::npos || lastDot < lastSlash) lastDot = path.length();

			modelName = path.substr(lastSlash + 1, lastDot - lastSlash - 1);

			directory = path.substr(0, path.find_last_of('/'));
			processNode(scene->mRootNode, scene);
		}

		void processNode(aiNode* node, const aiScene* scene) {

			// 处理当前节点的所有 mesh
			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.push_back(processMesh(mesh, scene));
			}

			// 递归处理子节点
			for (unsigned int i = 0; i < node->mNumChildren; i++) {
				processNode(node->mChildren[i], scene);
			}
		}

		void SetVertexBoneDataToDefault(Vertex& vertex)
		{
			for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
			{
				vertex.m_BoneIDs[i] = -1;
				vertex.m_Weights[i] = 0.0f;
			}
		}


		Mesh processMesh(aiMesh* mesh, const aiScene* scene)
		{
			vector<Vertex> vertices;
			vector<unsigned int> indices;
			vector<Texture> textures;

			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				SetVertexBoneDataToDefault(vertex);
				vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
				vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

				if (mesh->mTextureCoords[0])
				{
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}
				else
					vertex.TexCoords = glm::vec2(0.0f, 0.0f);

				if (mesh->mTangents) {
					vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh->mTangents[i]);
				}
				else {
					vertex.Tangent = glm::vec3(0.0f);
				}

				if (mesh->mBitangents) {
					vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh->mBitangents[i]);
				}
				else {
					vertex.Bitangent = glm::vec3(0.0f);
				}

				vertices.push_back(vertex);
			}
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

			ExtractBoneWeightForVertices(vertices, mesh, scene);


			for (auto& vertex : vertices)
			{
				float totalWeight = 0.0f;
				for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
					totalWeight += vertex.m_Weights[i];
				if (totalWeight <= 0.0f)  // 没有任何骨骼影响
				{
					vertex.m_BoneIDs[0] = 0;      // 默认赋值一个有效的骨骼索引（通常0号骨骼可代表静态骨骼或身份矩阵）
					vertex.m_Weights[0] = 1.0f;
				}
			}

			return Mesh(vertices, indices, textures);
		}

		void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
		{
			for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
			{
				if (vertex.m_BoneIDs[i] < 0)
				{
					vertex.m_Weights[i] = weight;
					vertex.m_BoneIDs[i] = boneID;
					break;
				}
			}
		}


		void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
		{
			auto& boneInfoMap = m_BoneInfoMap;
			int& boneCount = m_BoneCounter;

			for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
			{
				int boneID = -1;
				std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
				if (boneInfoMap.find(boneName) == boneInfoMap.end())
				{
					BoneInfo newBoneInfo;
					newBoneInfo.id = boneCount;
					newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
					boneInfoMap[boneName] = newBoneInfo;
					boneID = boneCount;
					boneCount++;
				}
				else
				{
					boneID = boneInfoMap[boneName].id;
				}
				assert(boneID != -1);
				auto weights = mesh->mBones[boneIndex]->mWeights;
				int numWeights = mesh->mBones[boneIndex]->mNumWeights;

				for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
				{
					int vertexId = weights[weightIndex].mVertexId;
					float weight = weights[weightIndex].mWeight;
					assert(vertexId < vertices.size());

					SetVertexBoneData(vertices[vertexId], boneID, weight);
				}
			}
		}


		unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
		{
			string filename = string(path);
			filename = directory + '/' + filename;

			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
			if (data)
			{
				GLenum format;
				if (nrComponents == 1)
					format = GL_RED;
				else if (nrComponents == 3)
					format = GL_RGB;
				else if (nrComponents == 4)
					format = GL_RGBA;
				else
				{
					std::cerr << "Unknown texture format for file: " << filename << std::endl;
					stbi_image_free(data);
					return 0;
				}

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else
			{
				// std::cout << "Texture failed to load at path: " << path << std::endl;
				stbi_image_free(data);
			}

			return textureID;
		}

		vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
		{
			vector<Texture> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				bool skip = false;
				for (unsigned int j = 0; j < textures_loaded.size(); j++)
				{
					if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
					{
						textures.push_back(textures_loaded[j]);
						skip = true; 
						break;
					}
				}
				if (!skip)
				{   
					Texture texture;
					texture.id = TextureFromFile(str.C_Str(), this->directory);
					texture.type = typeName;
					texture.path = str.C_Str();
					textures.push_back(texture);
					textures_loaded.push_back(texture);  
				}
			}
			return textures;
		}
	};

}

#endif
