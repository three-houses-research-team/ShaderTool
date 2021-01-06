#pragma once

#ifndef G1MG_
#define G1MG_

#include "G1MG/G1MGMaterial.h"
#include "G1MG/G1MGShaderParams.h"
#include "G1MG/G1MGSubMesh.h"
#include "G1MG/G1MGMesh.h"

#define MATERIALS_MAGIC  0x00010002
#define SHADER_MAGIC  0x00010003
#define SUBMESH_MAGIC  0x00010008
#define MESH_MAGIC  0x00010009


struct G1MGSubSectionHeader
{
	uint32_t magic;
	uint32_t size;
	uint32_t count;
};

struct G1MGHeader
{
	uint32_t platform;
	uint32_t reserved;
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
	uint32_t sectionCount;
};

struct G1MG
{
	GResourceHeader header;
	G1MGHeader g1mgHeader;
	uint32_t totalShaderCount = 0;
	uint32_t totalStringPaddedSize = 0;
	uint32_t totalUsefulDataSize = 0;
	std::map<int, std::vector<char>> otherSections;
	std::vector<G1MGMaterial> materials;
	std::vector<G1MGShaderParamSet> shaderParamSets;
	std::vector<G1MGSubmesh> submeshes;
	std::vector<G1MGMeshGroup> meshGroups;

	G1MG(std::ifstream& istream)
	{

		//Read headers
		istream.read(reinterpret_cast<char*>(&header), sizeof(GResourceHeader));
		istream.read(reinterpret_cast<char*>(&g1mgHeader), sizeof(G1MGHeader));

		G1MGSubSectionHeader tempHeader;
		for (auto i = 0; i < g1mgHeader.sectionCount; i++)
		{
			istream.read(reinterpret_cast<char*>(&tempHeader), sizeof(G1MGSubSectionHeader));

			switch (tempHeader.magic)
			{
			case MATERIALS_MAGIC:
				for (auto j = 0; j < tempHeader.count; j++)
				{
					materials.push_back(G1MGMaterial(istream));
				}
				break;
			case SHADER_MAGIC:
				for (auto j = 0; j < tempHeader.count; j++)
				{
					shaderParamSets.push_back(G1MGShaderParamSet(istream));
				}
				break;
			case SUBMESH_MAGIC:
				{
					G1MGSubmesh subM;
					for (auto j = 0; j < tempHeader.count; j++)
					{
						istream.read(reinterpret_cast<char*>(&subM), sizeof(G1MGSubmesh));
						submeshes.push_back(subM);
					}
				}	
				break;
			case MESH_MAGIC:
				for (auto j = 0; j < tempHeader.count; j++)
				{
					meshGroups.push_back(G1MGMeshGroup(istream));
				}
				break;
			default:
				istream.seekg(-12, std::ios_base::cur);
				otherSections[i + 1] = std::vector<char>(tempHeader.size);
				istream.read(reinterpret_cast<char*>(otherSections[i + 1].data()), tempHeader.size * sizeof(char));
				break;
			}
		}
	}
	void Write(std::ofstream& ostream)
	{
		std::streamoff sizeCheckpoint = ostream.tellp();
		//header
		ostream.write(reinterpret_cast<const char*>(&header), sizeof(GResourceHeader));
		ostream.write(reinterpret_cast<const char*>(&g1mgHeader), sizeof(G1MGHeader));
		ostream.write(reinterpret_cast<const char*>(otherSections[1].data()), otherSections[1].size() * sizeof(char));
		WriteMaterial(ostream);
		WriteShaderParam(ostream);
		ostream.write(reinterpret_cast<const char*>(otherSections[4].data()), otherSections[4].size() * sizeof(char));
		ostream.write(reinterpret_cast<const char*>(otherSections[5].data()), otherSections[5].size() * sizeof(char));
		ostream.write(reinterpret_cast<const char*>(otherSections[6].data()), otherSections[6].size() * sizeof(char));
		ostream.write(reinterpret_cast<const char*>(otherSections[7].data()), otherSections[7].size() * sizeof(char));
		WriteSubmesh(ostream);
		WriteMesh(ostream);
		size_t updatedSize = ostream.tellp() - sizeCheckpoint;
		UpdateSize(ostream, sizeCheckpoint + 8, updatedSize);
	}

	void WriteMaterial(std::ofstream& ostream)
	{
		G1MGSubSectionHeader temp;
		temp.magic = 0x00010002;
		temp.size = 0;
		temp.count = materials.size();
		std::streamoff sizeCheckpoint = ostream.tellp();
		ostream.write(reinterpret_cast<const char*>(&temp), sizeof(G1MGSubSectionHeader));
		for (G1MGMaterial& mat : materials)
		{
			ostream.write(reinterpret_cast<const char*>(&(mat.matHeader)), sizeof(G1MGMatHeader));
			ostream.write(reinterpret_cast<const char*>(mat.textureData.data()), mat.textureData.size() * sizeof(char));
		}
		size_t updatedSize = ostream.tellp() - sizeCheckpoint;
		UpdateSize(ostream, sizeCheckpoint + 4, updatedSize);

	}

	void WriteShaderParam(std::ofstream& ostream)
	{
		G1MGSubSectionHeader temp;
		temp.magic = 0x00010003;
		temp.size = 0;
		temp.count = shaderParamSets.size();
		std::streamoff sizeCheckpoint = ostream.tellp();
		ostream.write(reinterpret_cast<const char*>(&temp), sizeof(G1MGSubSectionHeader));
		for (G1MGShaderParamSet& shadPrmSet : shaderParamSets)
		{
			shadPrmSet.Write(ostream, totalShaderCount, totalStringPaddedSize);
		}
		size_t updatedSize = ostream.tellp() - sizeCheckpoint;
		totalUsefulDataSize = updatedSize - shaderParamSets.size() * 4 - 12;
		UpdateSize(ostream, sizeCheckpoint + 4, updatedSize);
	}

	void WriteSubmesh(std::ofstream& ostream)
	{
		G1MGSubSectionHeader temp;
		temp.magic = 0x00010008;
		temp.size = 12 + sizeof(G1MGSubmesh)*submeshes.size();
		temp.count = submeshes.size();
		ostream.write(reinterpret_cast<const char*>(&temp), sizeof(G1MGSubSectionHeader));
		for (G1MGSubmesh& sm : submeshes)
		{
			ostream.write(reinterpret_cast<const char*>(&sm), sizeof(G1MGSubmesh));
		}
	}

	void WriteMesh(std::ofstream& ostream)
	{
		G1MGSubSectionHeader temp;
		temp.magic = 0x00010009;
		temp.size = 0;
		temp.count = meshGroups.size();
		std::streamoff sizeCheckpoint = ostream.tellp();
		ostream.write(reinterpret_cast<const char*>(&temp), sizeof(G1MGSubSectionHeader));
		for (G1MGMeshGroup& mg : meshGroups)
		{
			mg.Write(ostream);
		}
		size_t updatedSize = ostream.tellp() - sizeCheckpoint;
		UpdateSize(ostream, sizeCheckpoint + 4, updatedSize);
	}
};

#endif //!G1MG_