#pragma once

#ifndef G1M_G_SUBM
#define G1M_G_SUBM

struct G1MGSubmesh
{
	uint32_t submeshType;
	uint32_t vertexBufferIndex;
	uint32_t bonePaletteIndex;
	uint32_t matPalID;
	uint32_t unk0;
	uint32_t shaderParamIndex;
	uint32_t materialIndex;
	uint32_t indexBufferIndex;
	uint32_t unk1;
	uint32_t indexBufferPrimType;
	uint32_t vertexBufferOffset;
	uint32_t vertexCount;
	uint32_t indexBufferOffset;
	uint32_t indexCount;
};

#endif // !G1M_G_SUBM