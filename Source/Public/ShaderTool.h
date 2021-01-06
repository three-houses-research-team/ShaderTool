#include "Utils.h"
#include "G1M/G1MG.h"
#define G1MG_MAGIC  0x47314D47

struct G1MHeader
{
	uint32_t magic;
	uint32_t version;
	uint32_t size;
	uint32_t firstChunkOffset;
	uint32_t reserved1;
	uint32_t chunkCount;
};

class G1M
{
	public:
		G1M(std::ifstream& istream, bool isDest)
		{
			//G1M Header
			istream.read(reinterpret_cast<char*>(&header), sizeof(G1MHeader));
			istream.seekg(header.firstChunkOffset);

			//Subsections
			GResourceHeader tempHeader;
			bool bHasPassedG1MG = false;
			for (auto i = 0; i < header.chunkCount; i++)
			{
				istream.read(reinterpret_cast<char*>(&tempHeader), sizeof(GResourceHeader));

				if (tempHeader.magic != G1MG_MAGIC)
				{
					if(!isDest)
						istream.seekg(tempHeader.size - 12, std::ios_base::cur); //Data not needed, skipping
					else
						if (!bHasPassedG1MG) //caching pre and post G1MG bytes to avoid reparsing
						{
							istream.seekg(-12, std::ios_base::cur);
							beforeG1MG.resize(beforeG1MG.size() + tempHeader.size);
							istream.read(reinterpret_cast<char*>(beforeG1MG.data()+ (beforeG1MG.size()- tempHeader.size) * sizeof(char)), tempHeader.size * sizeof(char));
						}
						else
						{
							istream.seekg(-12, std::ios_base::cur);
							afterG1MG.resize(afterG1MG.size() + tempHeader.size);
							istream.read(reinterpret_cast<char*>(afterG1MG.data() + (afterG1MG.size()- tempHeader.size) * sizeof(char)), tempHeader.size * sizeof(char));
						}
				}
				else
				{
					istream.seekg(-12, std::ios_base::cur);
					g1mg.push_back(G1MG(istream));
					bHasPassedG1MG = true;
				}
			}
		}
		void Write(std::ofstream& ostream)
		{
			//G1M Header
			ostream.write(reinterpret_cast<const char*>(&header), sizeof(G1MHeader));
			//Pre G1MG data
			ostream.write(reinterpret_cast<const char*>(beforeG1MG.data()), beforeG1MG.size() * sizeof(char));
			//G1MG data
			g1mg[0].Write(ostream);
			//Post G1MG data
			ostream.write(reinterpret_cast<const char*>(afterG1MG.data()), afterG1MG.size() * sizeof(char));
			//Update the G1M size
			size_t updatedSize = ostream.tellp();
			UpdateSize(ostream, 8, updatedSize);
			//Update the G1MF fields
			UpdateSize(ostream, 0x40, g1mg[0].totalShaderCount);
			UpdateSize(ostream, 0x44, g1mg[0].totalStringPaddedSize);
			UpdateSize(ostream, 0x48, g1mg[0].totalUsefulDataSize);
		}
		bool UpdateAttributes(const G1M& sourceG1M, std::vector<int>& submeshListDest, std::vector<int>& submeshListSource)
		{
			//Check if indices are ok
			for (auto& i : submeshListDest)
			{
				if (i >= g1mg[0].submeshes.size())
				{
					std::cout << "invalid submeshes index " << i << " in modified g1m" << std::endl;
					system("pause");
					return false;
				}
			}

			for (auto& i : submeshListSource)
			{
				if (i >= sourceG1M.g1mg[0].submeshes.size())
				{
					std::cout << "invalid submeshes index " << i << " in source g1m" << std::endl;
					system("pause");
					return false;
				}
			}


			//Update meshes
			std::vector<std::pair<int, int>> sourceMeshInfo;
			bool bHasFoundIndex;
			int j;
			for (auto& i : submeshListDest)
			{
				bHasFoundIndex = false;
				j = 0;
				while (!bHasFoundIndex && j < g1mg[0].meshGroups.size())
				{
					G1MGMeshGroup& meshgrp = g1mg[0].meshGroups[j];
					for (auto k = 0; k < meshgrp.meshes.size(); k++)
					{
						G1MGMesh& mesh = meshgrp.meshes[k];
						if (std::count(mesh.indices.begin(), mesh.indices.end(), i))
						{
							auto p = std::make_pair(j, k);
							sourceMeshInfo.push_back(p);
							bHasFoundIndex = true;
						}
					}
					j++;
				}
				if (!bHasFoundIndex)
				{
					std::cout << "Couldn't find submesh " << i << " in dest." << std::endl;
					system("pause");
				}
			}

			int index = 0;
			for (auto& i : submeshListSource)
			{
				bHasFoundIndex = false;
				j = 0;
				while (!bHasFoundIndex && j < sourceG1M.g1mg[0].meshGroups.size())
				{
					const G1MGMeshGroup& meshgrp = sourceG1M.g1mg[0].meshGroups[j];
					for (auto k = 0; k < meshgrp.meshes.size(); k++)
					{
						const G1MGMesh& mesh = meshgrp.meshes[k];
						if (std::count(mesh.indices.begin(), mesh.indices.end(), i))
						{
							memcpy(&(g1mg[0].meshGroups[sourceMeshInfo[index].first].meshes[sourceMeshInfo[index].second].header.name), &mesh.header.name, 16);
							bHasFoundIndex = true;
						}
					}
					j++;
				}
				index++;
				if (!bHasFoundIndex)
				{
					std::cout << "Couldn't find submesh " << i << " in source." << std::endl;
					system("pause");
				}
			}


			//Update materials and shaders
			std::vector<std::pair<int, int>> matShadSourceIndices;
			std::vector<std::pair<int, int>> matShadDestIndices;
			for (auto& i : submeshListDest)
			{
				auto p = std::make_pair(g1mg[0].submeshes[i].materialIndex, g1mg[0].submeshes[i].shaderParamIndex);
				matShadDestIndices.push_back(p);				
			}
			for (auto& i : submeshListSource)
			{
				auto p = std::make_pair(sourceG1M.g1mg[0].submeshes[i].materialIndex, sourceG1M.g1mg[0].submeshes[i].shaderParamIndex);
				matShadSourceIndices.push_back(p);
			}

			for (auto i = 0; i < matShadSourceIndices.size(); i++)
			{
				std::pair<int, int>& pSource = matShadSourceIndices[i];
				std::pair<int, int>& pDest = matShadDestIndices[i];

				g1mg[0].materials.at(pDest.first) = sourceG1M.g1mg[0].materials.at(pSource.first);
				g1mg[0].shaderParamSets.at(pDest.second) = sourceG1M.g1mg[0].shaderParamSets.at(pSource.second);
			}

			return true;
		}

	protected:
		G1MHeader header;
		std::vector<G1MG> g1mg;
		std::vector<char> beforeG1MG;
		std::vector<char> afterG1MG;
};