#include <iostream>
#include <iterator>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <string>
#include <map>
#include <Windows.h>
#include "..\Public\ShaderTool.h"

int main(int argc, char* argv[])
{
	//Check if we have at least 2 files to work with
	if (argc < 3)
	{
		std::cout << "Not enough arguments." << std::endl;
		system("pause");
	}

	//Check if the files are .g1m 
	std::filesystem::path firstFile(argv[1]);
	std::filesystem::path secondFile(argv[2]);
	if (firstFile.extension() != ".g1m" || secondFile.extension() != ".g1m")
	{
		std::cout << "Incorrect arguments, you need 2 .g1m files." << std::endl;
		system("pause");
	}

	//Open the first file
	std::ifstream firstG1M(argv[1], std::ios::in | std::ios::binary);
	if (!firstG1M.is_open())
	{
		std::cout << "Couldn't open the first G1M file" << std::endl;
		system("pause");
		return 1;
	}

	std::ifstream secondG1M(argv[2], std::ios::in | std::ios::binary);
	if (!secondG1M.is_open())
	{
		std::cout << "Couldn't open the second G1M file" << std::endl;
		system("pause");
		return 1;
	}

	std::vector<int> submeshListDest; 
	std::vector<int> submeshListSource;

	//Input submeshes
	std::cout << "Enter the submeshes indices of the first g1m \n";
	std::string destStr;
	getline(std::cin, destStr);
	std::cout << "Enter the submeshes indices of the second g1m \n";
	std::string sourceStr;
	getline(std::cin, sourceStr);
	std::stringstream iss(destStr);
	std::stringstream iss2(sourceStr);

	int number;
	while (iss >> number)
		submeshListDest.push_back(number);
	while (iss2 >> number)
		submeshListSource.push_back(number);

	G1M g1m = G1M(firstG1M, true);
	G1M otherG1M = G1M(secondG1M, false);
	if (!g1m.UpdateAttributes(otherG1M, submeshListDest, submeshListSource))
		return 0;

	std::string output = firstFile.filename().string() + "_patched";
	const char* test = output.c_str();

	std::ofstream ostream(output, std::ios::out | std::ios::binary);
	if (!ostream.is_open())
	{
		std::cout << "Couldn't create an output file" << std::endl;
		system("pause");
		return 1;
	}

	g1m.Write(ostream);

	return 0;
	
}

