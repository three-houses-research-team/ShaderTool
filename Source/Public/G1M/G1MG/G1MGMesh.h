#pragma once

#ifndef G1M_G_MESH
#define G1M_G_MESH

struct G1MGMeshHeader
{
	char name[16];
	uint16_t meshType;
	uint16_t unk1;
	uint32_t externalID;
	uint32_t indexCount;
};

struct G1MGMesh
{
	G1MGMeshHeader header;
	std::vector<uint32_t> indices;

	G1MGMesh(std::ifstream& istream)
	{
		istream.read(reinterpret_cast<char*>(&header), sizeof(G1MGMeshHeader));
		
		if (header.indexCount > 0)
		{
			indices.resize(header.indexCount);
			istream.read(reinterpret_cast<char*>(indices.data()), (int)header.indexCount * sizeof(uint32_t));
		}
		else
			istream.seekg(4, std::ios_base::cur);
	}
};

struct G1MGMeshGroupHeader
{
	uint32_t LOD;
	uint32_t Group;
	uint32_t GroupEntryIndex;
	uint32_t submeshCount1; 
	uint32_t submeshCount2;
	uint32_t lodRangeStart;
	uint32_t lodRangeLength;
	uint32_t unk1;
	uint32_t unk2;
};

struct G1MGMeshGroup
{
	G1MGMeshGroupHeader header;
	std::vector<G1MGMesh> meshes;

	G1MGMeshGroup(std::ifstream& istream)
	{	
		istream.read(reinterpret_cast<char*>(&header), sizeof(G1MGMeshGroupHeader));
		for (auto i = 0; i < header.submeshCount1 + header.submeshCount2; i++)
		{
			meshes.push_back(G1MGMesh(istream));
		}
	}

	void Write(std::ofstream& ostream)
	{
		ostream.write(reinterpret_cast<const char*>(&header), sizeof(G1MGMeshGroupHeader));
		for (G1MGMesh& mesh : meshes)
		{
			ostream.write(reinterpret_cast<const char*>(&(mesh.header)), sizeof(G1MGMeshHeader));
			ostream.write(reinterpret_cast<const char*>(mesh.indices.data()), (int)mesh.indices.size() * sizeof(uint32_t));
		}
	}
};

#endif // !G1M_G_MESH
