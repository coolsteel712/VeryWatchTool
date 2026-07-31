#pragma once

#include <string>
#include <vector>
#include <stdint.h>

class fileTool
{
public:
	//获取路径下的所有文件
	static void getAllFiles(std::string pathString, std::vector<std::string>& files);

	static void getAllFileAndDir(std::string pathString, std::vector<std::string>& files);

	//把buff写入到文件
	static void wirteBufftoFile(std::string fileName, uint8_t* buff, uint32_t buff_size);

	//把文件名和后缀分离
	static bool separateFileName(const char* pcFile, char* pcFileName, char* pcFileExtension);

};

