

#ifndef _IMAGE_LOAD_H_
#define _IMAGE_LOAD_H_

#include "bitmap.h"

typedef enum
{
    NO_TYPE = 0,
    LZ4_TYPE,
    FASTLZ_TYPE
}COMPRESS_TYPE;
extern int compress_type;

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

struct bmp_fileheader
{
    unsigned short    bfType;        //若不对齐，这个会占4Byte
    unsigned int    bfSize;
    unsigned short    bfReverved1;
    unsigned short    bfReverved2;
    unsigned int    bfOffBits;
};

struct bmp_infoheader
{
    unsigned int    biSize;
    unsigned int    biWidth;
    unsigned int    biHeight;
    unsigned short    biPlanes;
    unsigned short    biBitCount;
    unsigned int	biCompression;
    unsigned int    biSizeImage;
    unsigned int    biXPelsPerMeter;
    unsigned int    biYpelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
};

struct rgbquad{
    unsigned char BYTErgbRed;     //   红色的亮度(值范围为0-255)
    unsigned char BYTErgbGreen;   //   绿色的亮度(值范围为0-255)
    unsigned char BYTErgbBlue;    //   蓝色的亮度(值范围为0-255)
    unsigned char BYTErgbReserved;//   保留，必须为0
};

struct rgbhead{
    struct bmp_fileheader bfh;
    struct bmp_infoheader bih;
    struct rgbquad rq[4];
};

#pragma pack()

//加载文件,得到bitmap
extern uint32_t bitmap_tool_load_file(const char *file_name, bitmap_t *bitmap);

//加载图片,得到图片的像素缓存,返回font_buff的大小
extern uint32_t bitmap_tool_get_font_buff(const char* file_name, uint8_t* data_buff, uint32_t buff_size, uint8_t format);

//是否为png
bool isPNG(const char* filePath);

//加载图片jpg
extern uint8_t *jpg_stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);

//写png
extern int jpg_stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);

#ifdef __cplusplus
}
#endif

#endif
