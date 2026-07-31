
#ifndef _BITMAP_FONT_H_
#define _BITMAP_FONT_H_

#include <stdint.h>

#define FONT_FORMAT_ALPHA_MASK		(1 << 7)
#define FONT_FORMAT_SWAP_COLOR		(1 << 6)

#define FONT_FORMAT_NONE		0	//无效

#define FONT_FORMAT_RGB111		1
#define FONT_FORMAT_BGR111		(FONT_FORMAT_RGB111 | FONT_FORMAT_SWAP_COLOR)   //65

#define FONT_FORMAT_ARGB111		(FONT_FORMAT_RGB111 | FONT_FORMAT_ALPHA_MASK)   //129
#define FONT_FORMAT_ABGR111		(FONT_FORMAT_BGR111 | FONT_FORMAT_ALPHA_MASK)   //193

#define FONT_FORMAT_RGB222		2
#define FONT_FORMAT_BGR222		(FONT_FORMAT_RGB222 | FONT_FORMAT_SWAP_COLOR)   //66

#define FONT_FORMAT_ARGB222		(FONT_FORMAT_RGB222 | FONT_FORMAT_ALPHA_MASK)   //130
#define FONT_FORMAT_ABGR222		(FONT_FORMAT_BGR222 | FONT_FORMAT_ALPHA_MASK)   //194

#define FONT_FORMAT_RGB565		5
#define FONT_FORMAT_BGR565		(FONT_FORMAT_RGB565 | FONT_FORMAT_SWAP_COLOR)   //69

#define FONT_FORMAT_ARGB565		(FONT_FORMAT_RGB565 | FONT_FORMAT_ALPHA_MASK)   //133
#define FONT_FORMAT_ABGR565		(FONT_FORMAT_BGR565 | FONT_FORMAT_ALPHA_MASK)   //197

#define FONT_FORMAT_RGB888		8
#define FONT_FORAMT_BGR888		(FONT_FORMAT_RGB888 | FONT_FORMAT_SWAP_COLOR)   //72

#define FONT_FORMAT_ARGB888		(FONT_FORMAT_RGB888 | FONT_FORMAT_ALPHA_MASK)   //136
#define FONT_FORAMT_ABGR888		(FONT_FORAMT_BGR888 | FONT_FORMAT_ALPHA_MASK)   //200

#define FONT_FORMAT_ARGB6666    (9)
#define FONT_FORMAT_ABGR6666    (FONT_FORMAT_ARGB6666 | FONT_FORMAT_SWAP_COLOR) //73

#define FONT_FORMAT_RGBA5658    (10)

//------- realme项目 -------
#define FONT_FORMAT_RGB666      6
#define FONT_FORAMT_BGR666        (FONT_FORMAT_RGB666 | FONT_FORMAT_SWAP_COLOR) //70
#define FONT_FORMAT_RGBA6666    (FONT_FORMAT_RGB666 | FONT_FORMAT_ALPHA_MASK)   //134
#define FONT_FORAMT_BGRA6666    (FONT_FORAMT_BGR666 | FONT_FORMAT_ALPHA_MASK)   //198

//单色 1bit 
#define FONT_FORMAT_MONO1		100
//单色 2bit
#define FONT_FORMAT_MONO2		101
//单色 4bit
#define FONT_FORMAT_MONO4		102
//单色 8bit
#define FONT_FORMAT_MONO8		103

//自动模式,如果是8bit图片,采用4bit取模,或者采用rgb,或者rgba取模
#define FONT_FORMAT_AUTO		0xff

//内部使用
#define PICTURE_FORMAT_JPG      0xfe

//字体头部信息
typedef struct font_head
{
	uint8_t type[4];	//类型 RAW,BMP,PNG,JPG,等
	uint16_t width;
	uint16_t height;
	uint8_t format;	//位图格式 包含alpha 信息
	uint8_t alpha_format;	//alpha 通道格式
	uint32_t alpha_data_start_offset;	//alpha通道 数据开始位置


	/*
	像素内容,第一部分为 位图信息,
	位图的偏移可以通过widget*height*bpp得到
	第二部分为阿尔法通道信息
	**/


}font_head_t;


#endif
