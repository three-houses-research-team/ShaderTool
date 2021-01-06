#pragma once
#ifndef G1M_G_SHAD
#define G1M_G_SHAD

struct G1MGShaderHeader
{
	uint32_t size;
	uint32_t paddedNameLength;
	uint32_t res;
	uint32_t datatype;
};

struct G1MGShader
{
	G1MGShaderHeader header;
	std::vector<char> data;

	G1MGShader(std::ifstream& istream)
	{
		istream.read(reinterpret_cast<char*>(&header), sizeof(G1MGShaderHeader));
		data.resize(header.size - 16);
		istream.read(reinterpret_cast<char*>(data.data()), data.size()*sizeof(char));
	}
};

struct G1MGShaderParamSet
{
	uint32_t count;
	std::vector<G1MGShader> shaders;	

	G1MGShaderParamSet(std::ifstream& istream)
	{
		istream.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
		
		for (auto i = 0; i < count; i++)
		{
			shaders.push_back(G1MGShader(istream));
		}
	}

	void Write(std::ofstream& ostream, uint32_t& totalShaderCount, uint32_t& totalStringPaddedSize)
	{
		ostream.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
		for (G1MGShader& shad : shaders)
		{
			totalShaderCount++;
			totalStringPaddedSize += shad.header.paddedNameLength;
			ostream.write(reinterpret_cast<const char*>(&(shad.header)), sizeof(G1MGShaderHeader));
			ostream.write(reinterpret_cast<const char*>(shad.data.data()), shad.data.size() * sizeof(char));
		}
		
	}
};

#endif // !G1M_G_SHAD
