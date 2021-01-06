#pragma once
#ifndef UTILS
#define UTILS

void UpdateSize(std::ofstream& ostream, size_t sizeOffset, size_t updatedSize)
{
	size_t originalPos = ostream.tellp();
	ostream.seekp(sizeOffset);
	ostream.write(reinterpret_cast<const char*>(&updatedSize), sizeof(size_t));
	ostream.seekp(originalPos);
}

struct GResourceHeader
{
	uint32_t magic;
	uint32_t version;
	uint32_t size;
};

#endif // !UTILS