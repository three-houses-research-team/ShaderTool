#pragma once

#ifndef G1M_G_MATERIAL
#define G1M_G_MATERIAL

struct G1MGMatHeader
{
	uint32_t unk0;
	uint32_t textureCount;
	uint32_t unk1;
	uint32_t unk2;
};

struct G1MGTexture
{
	uint16_t index; //texture index in g1t file
	uint16_t layer; //TEXCOORD layer
	uint16_t textureType;
	uint16_t otherType;
	uint16_t tileModex;
	uint16_t tileModey;
};

struct G1MGMaterial
{
	G1MGMatHeader matHeader;
	std::vector<char> textureData;

	G1MGMaterial(std::ifstream& istream)
	{
		istream.read(reinterpret_cast<char*>(&matHeader), sizeof(G1MGMatHeader));
		textureData.resize(matHeader.textureCount * sizeof(G1MGTexture));
		istream.read(reinterpret_cast<char*>(textureData.data()), (int)matHeader.textureCount * sizeof(G1MGTexture));
	}
};

#endif // !G1M_G_MATERIAL
