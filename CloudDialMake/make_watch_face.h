
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <regex>  //正则表达式
#include <fstream>
#include "watch_face_rw_head.h"
#include "bitmap_font.h"

/*制作打包表盘*/

class mkWatchFace
{
private:

	std::vector<watch_face_file_data_t> fileVector;	//文件列表
	std::string filePath;
	uint32_t file_buff_offset;
	uint32_t file_head_size;

	//添加文件到缓存
	uint32_t addtoBuff(const char* string, uint8_t* buff, uint32_t size);

	//添加资源文件
	uint32_t addResItem(std::string resName, uint8_t format);

	//添加字体资源文件  字体资源命名有些特殊
	uint32_t addFontItem(std::string fontName, std::string fileName, uint8_t fontFormat);

	//添加所有资源
	uint32_t addAllResource(std::string filePath,uint8_t format);

	//添加子目录
	uint32_t addSubResource(std::string filePath, std::string subName, uint8_t format);

public:
    
	mkWatchFace();

	/*
	制作文件
	filePath 素材路径
	saveFileName 文件名
	format	取模图片的格式
	*/
	uint32_t makeFile(std::string filePath,std::string saveFileName,uint8_t format);

	/*
	制作(IWF)文件, 根据表盘包获取到IWF文件接口
	filePath      素材路径, 表盘包目录路径
	saveFilePath  输出保存的文件路径(.iwf)
	format	      取模图片的格式

	 返回值
	        失败 : 1
	 		成功 : IWF文件大小
	*/
	uint32_t makeIwfFile(std::string filePath, std::string saveFilePath, uint8_t format);

	/*
	制作压缩文件

	压缩文件会自动添加文件名.lz后缀
	filePath 素材路径
	saveFileName 文件名
	format	取模图片的格式
	block_size 压缩块大小 1024,4096
	*/
	uint32_t makeCompressionFile(std::string filePath, std::string saveFileName, uint8_t format,uint16_t block_size);


};

