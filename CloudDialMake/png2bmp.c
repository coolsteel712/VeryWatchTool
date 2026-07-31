#include <stdio.h>
#include <png.h>  
#include <stdio.h>  
#include "zlib.h"
#include <stdlib.h>  
#include <string.h>
#include "debug.h"
#include "png2bmp.h"
#include "error.h"


#define required_bit_depth 4
#define PIC_FORMAT_RGB565 5
#define PIC_FORMAT_RGB888 8

#pragma pack(2) 

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

png_FILE_p fpin;  
FILE *fpout;   
//读：  
png_structp read_ptr;  
png_infop read_info_ptr, end_info_ptr;  
//写  
png_structp write_ptr;  
png_infop write_info_ptr,write_end_info_ptr;  
//  
png_bytep row_buf;  
png_uint_32 y;  
int num_pass, pass;  
png_uint_32 width, height;//宽度，高度  
int bit_depth, color_type;//位深，颜色类型  
int interlace_type, compression_type, filter_type;//扫描方式，压缩方式，滤波方式 

int isRGB;

png_textp text_ptr;  
int num_text;  
png_timep mod_time;
png_colorp palette;  
int num_palette;

enum ERROR_CODE{PASS,ERROR_PNGREAD,ERROR_BMPWRITE,ERROR_COLORTYPE,ERROR_PICTUREFORMAT,ERROR_MALLOC};

int read_png_info()
{
	//读文件有high level(高层）和low level两种，我们选择从底层具体信息中读取。  
    //这里我们读取所有可选。  
    png_read_info(read_ptr, read_info_ptr);      
    //（1）IHDR  
      //读取图像宽度(width)，高度(height)，位深(bit_depth)，颜色类型(color_type)，压缩方法(compression_type)  
    //滤波器方法(filter_type),隔行扫描方式(interlace_type)  
    if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,  
      &color_type, &interlace_type, &compression_type, &filter_type))  
    {  
    //我们采用默认扫描方式  
     png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,  
        color_type, PNG_INTERLACE_NONE, compression_type, filter_type);  
    }  
    //（2）cHRM  
    //读取白色度信息  白/红/绿/蓝 点的x,y坐标，这里采用整形，不采用浮点数  
    png_fixed_point white_x, white_y, red_x, red_y, green_x, green_y, blue_x,blue_y;  
  
    if (png_get_cHRM_fixed(read_ptr, read_info_ptr, &white_x, &white_y,  
     &red_x, &red_y, &green_x, &green_y, &blue_x, &blue_y))  
    {  
     png_set_cHRM_fixed(write_ptr, write_info_ptr, white_x, white_y, red_x,  
        red_y, green_x, green_y, blue_x, blue_y);  
    }  
        //（3）gAMA  
      png_fixed_point gamma;  
  
      if (png_get_gAMA_fixed(read_ptr, read_info_ptr, &gamma))  
         png_set_gAMA_fixed(write_ptr, write_info_ptr, gamma);  
        //（4）iCCP  
    png_charp name;  
    png_bytep profile;  
    png_uint_32 proflen;  
  
    if (png_get_iCCP(read_ptr, read_info_ptr, &name, &compression_type,  
              &profile, &proflen))  
    {  
     png_set_iCCP(write_ptr, write_info_ptr, name, compression_type,  
              profile, proflen);  
    }  
    //(5)sRGB  
      int intent;  
      if (png_get_sRGB(read_ptr, read_info_ptr, &intent))  
         png_set_sRGB(write_ptr, write_info_ptr, intent);  
    //(7)PLTE  
      //png_colorp palette;  
      //int num_palette;  
  
      if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette))  
         png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);  
    //(8)bKGD  
      png_color_16p background;  
  
      if (png_get_bKGD(read_ptr, read_info_ptr, &background))  
      {  
         png_set_bKGD(write_ptr, write_info_ptr, background);  
      }  
    //(9)hist  
   
      png_uint_16p hist;  
  
      if (png_get_hIST(read_ptr, read_info_ptr, &hist))  
         png_set_hIST(write_ptr, write_info_ptr, hist);  
       //(10)oFFs  
      png_int_32 offset_x, offset_y;  
      int unit_type;  
  
      if (png_get_oFFs(read_ptr, read_info_ptr, &offset_x, &offset_y,  
          &unit_type))  
      {  
         png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);  
      }  
    //(11)pCAL  
      png_charp purpose, units;  
      png_charpp params;  
      png_int_32 X0, X1;  
      int type, nparams;  
  
      if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,  
         &nparams, &units, &params))  
      {  
         png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,  
            nparams, units, params);  
      }  
       //(12)pHYs  
     
      png_uint_32 res_x, res_y;  
  
      if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y, &unit_type))  
         png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);  
   //(13)sBIT  
      png_color_8p sig_bit;  
  
      if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit))  
         png_set_sBIT(write_ptr, write_info_ptr, sig_bit);  
    //（14）sCAL  
      int unit;  
      png_charp scal_width, scal_height;

	/*  
      if (png_get_sCAL_s(read_ptr, read_info_ptr, &unit, &scal_width,  
          &scal_height))  
      {  
         png_set_sCAL_s(write_ptr, write_info_ptr, unit, scal_width,  
             scal_height);  
      }
	*///Dukelop:sth wrong with png_get/set_sCAL_s
        //(15)iTXt  
      //png_textp text_ptr;  
      //int num_text;  
  
      if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)  
      {  
         png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);  
      }  
    //(16)tIME,这里我们不支持RFC1123  
      //png_timep mod_time;  
  
      if (png_get_tIME(read_ptr, read_info_ptr, &mod_time))  
      {  
         png_set_tIME(write_ptr, write_info_ptr, mod_time);  
      }  
    //(17)tRNS  
      png_bytep trans_alpha;  
      int num_trans;  
      png_color_16p trans_color;  
  
      if (png_get_tRNS(read_ptr, read_info_ptr, &trans_alpha, &num_trans,  
         &trans_color))  
      {  
         int sample_max = (1 << bit_depth);  
         /* libpng doesn't reject a tRNS chunk with out-of-range samples */  
         if (!((color_type == PNG_COLOR_TYPE_GRAY &&  
             (int)trans_color->gray > sample_max) ||  
             (color_type == PNG_COLOR_TYPE_RGB &&  
             ((int)trans_color->red > sample_max ||  
             (int)trans_color->green > sample_max ||  
             (int)trans_color->blue > sample_max))))  
            png_set_tRNS(write_ptr, write_info_ptr, trans_alpha, num_trans,  
               trans_color);  
      }  

	LOG_ERROR("read file over");

    //写进新的png文件中
    //png_write_info(write_ptr, write_info_ptr);
	return PASS; 
}

//写入BMP 32bit数据
int write_bmp_data_32bit(uint8_t *data)
{
    png_bytep row_pointers[height];
    int row;
    for (row = 0; row <height; row++){
        row_pointers[row] = NULL;
    }
    for (row = 0; row <height; row++){
        row_pointers[row] = (png_bytep)png_malloc(read_ptr, png_get_rowbytes(read_ptr,read_info_ptr));
    }
    png_read_image(read_ptr, row_pointers);
    int i, j;
    int size = (width) * (height) * 4;
    LOG_ERROR("malloc(%d)", size);

//    unsigned int *Dst = (unsigned int*)malloc(size);//因为sizeof(unsigned long)=8
//    unsigned int *pDst=NULL;
    unsigned char* pSrc;
    unsigned long pixelR,pixelG,pixelB,pixelA;
    int fillwidth;
    int status=PASS;


    uint8_t *dstBMP = data + 54;
    unsigned int *temp = NULL;

    switch(color_type)
    {
        case 2:
        {
            LOG_ERROR("RGB_24");
            for(j=0; j<height; j++)
            {
                pSrc = row_pointers[j];
                temp = (unsigned int *)(dstBMP + (width * 4 * j));
                for(i=0; i<width; i++)
                {
                    pixelR = *pSrc++;
                    pixelG = *pSrc++;
                    pixelB = *pSrc++;
                    pixelA = 0xFF;
                    *temp++ = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
//                    DEBUG_INFO("%06x ", pDst[i]);
                }
//                fwrite(pDst,sizeof(unsigned int),width,fpout);
//                pDst += width;
            }
            break;
        }

        case 3:
        {
            LOG_ERROR("palette,color_type = 3");
            for(j=0; j<height; j++)
            {
                pSrc = row_pointers[j];
                temp = (unsigned int *)(dstBMP + (width * 4 * j));
                for(i=0; i<width; i++)
                {
                    pixelR = (palette+*pSrc++)->red;
                    pixelG = (palette+*pSrc++)->green;
                    pixelB = (palette+*pSrc++)->blue;
                    pixelA = 0xFF;

                    //dukelop:seems data in png with color_type(3) been copyed 3 times
                    *temp++ = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
                    *temp++ = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
                    *temp++ = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
                }
//                fwrite(pDst,sizeof(unsigned int),width,fpout);
//                pDst += width;
            }

            break;
        }

        case 6:
        {
            printf("RGB_32\n");
            for(j=0; j<height; j++)
            {
                pSrc = row_pointers[j];
                temp = (unsigned int *)(dstBMP + (width * 4 * j));
                for(i=0; i<width; i++)
                {
                    pixelR = *pSrc++;
                    pixelG = *pSrc++;
                    pixelB = *pSrc++;
                    pixelA = *pSrc++;
                    *temp++ = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
                    //printf("%08x ", pDst[i]);
                }
                //printf("\n");
//                fwrite(pDst,sizeof(unsigned int),width,fpout);
//                pDst += width;
            }

            break;
        }
        default:
        {
            DEBUG_INFO("color type:%d not defined",bit_depth);
            status=ERROR_COLORTYPE;
        }
            break;
    }

    for (row = 0; row <height; row++)
    {
        png_free(read_ptr,row_pointers[row]);
    }

    return status;
}

//写入BMP 16bit数据
void write_bmp_data_16bit(uint8_t *data)
{
    png_bytep row_pointers[height];
    int row;
    for (row = 0; row <height; row++){
        row_pointers[row] = NULL;
    }
    for (row = 0; row <height; row++){
        row_pointers[row] = (png_bytep)png_malloc(read_ptr, png_get_rowbytes(read_ptr,read_info_ptr));
    }
    png_read_image(read_ptr, row_pointers);
    int i, j;

    unsigned char* pSrc;
    unsigned short pixelR,pixelG,pixelB;

    uint8_t *dstBMP = data + 54;
    unsigned short *temp = NULL;


    LOG_ERROR("RGB_16, color_type = %d",color_type);
    for(j=0; j<height; j++)
    {
        pSrc = row_pointers[j];
        temp = (unsigned short *)(dstBMP + (width * 2 * j));
        for(i=0; i<width; i++)
        {
            pixelR = *pSrc++;
            pixelG = *pSrc++;
            pixelB = *pSrc++;
//            DEBUG_INFO("pixelR = 0x%04x, pixelG = 0x%04x, pixelB = 0x%04x",pixelR,pixelG,pixelB);
            pixelR = ((pixelR)+4)>>3;
            pixelG = ((pixelG)+4)>>3;
            pixelB = ((pixelB)+4)>>3;
            if(pixelR > 31) pixelR = 31;
            if(pixelG > 31) pixelG = 31;
            if(pixelB > 31) pixelB = 31;
//            //K = ((R << 11)|(G << 5)|(B<<0)); //内存中的保存形式 B分量在低地址
            //ARGB555格式, 透明度占一位，R,G,B各占5位
            *temp++ = ((0<<15)|(pixelR << 10)|(pixelG << 5)|(pixelB));

            // 修复32bit PNG图片转BMP图片模糊问题 2022-3-25
            if(color_type == 6)pSrc++;
        }
    }

    for (row = 0; row <height; row++)
    {
        png_free(read_ptr,row_pointers[row]);
    }
}

//制作bmp文件
int write_bmp(uint8_t format)
{
//    int status=PASS;
    struct bmp_fileheader bfh;
    struct bmp_infoheader bih;
    unsigned short depth;
    unsigned long headersize;
    unsigned long filesize;

    isRGB=1;

    LOG_ERROR("writing bmp header...");

    if (color_type==0||color_type==3||color_type==4)isRGB=0;
    else if (color_type==2||color_type==6)isRGB=1;


    memset(&bfh, 0, sizeof(struct bmp_fileheader));
    memset(&bih, 0, sizeof(struct bmp_infoheader));


    if( format == PIC_FORMAT_RGB888 ) {
        depth = 4;
        headersize = 14 + 40;
        filesize = headersize + width * height * depth;

        //写入比较关键的几个bmp头参数
        bfh.bfType = 0x4D42;
        bfh.bfSize = filesize;
        bfh.bfOffBits = headersize;

        bih.biSize = 40;
        bih.biWidth = width;
        bih.biHeight = -height;
        bih.biPlanes = 1;
        bih.biBitCount = (unsigned short) depth * 8;
        bih.biSizeImage = width * height * depth;

        LOG_ERROR("pic format= %d,headersize = %d,filesize =%d,biWidth =%d,biHeight=%d",format,headersize,filesize,bih.biWidth,bih.biHeight);

        uint8_t *bmp = (uint8_t *)malloc(bfh.bfSize);
        if(!bmp)
        {
            LOG_INFO("err:malloc for *bmp error");
            return ERROR_MALLOC;
        }

        memset(bmp, 0, bfh.bfSize);
        memcpy(bmp, &bfh, 14);
        memcpy(&bmp[14], &bih, 40);

        LOG_ERROR("writing bmp data...");

        int rel = write_bmp_data_32bit(bmp);
        if(rel != PASS)
        {
            LOG_INFO("err:write bmp data 32bit failed!");
            free(bmp);
            return ERROR_BMPWRITE;
        }
        fwrite(bmp,1,bfh.bfSize,fpout);

        free(bmp);
    }
    //16bitBMP图片制作, 包括包装头部,将数据位32bit压缩成16bit
    else{
        depth = 2;
        headersize = 14 + 40;
        filesize = headersize + width * height * depth;

        /*
         * BMP图片格式是RGB565的时候才需要调色板, 且需要将bih.biCompression = 0x3
         * ARGB555不需要调色板, bih.biCompression = 0
         * struct rgbquad pcolortable[4] = {0};
         * 所以RGB565文件大小 = 14+40+16（个字节的头部信息） + width * height * 2(字节)
         * AGRGB由于不需要调色板(掩码),所以比起RGB565文件大小少了头部信息内调色板的16个字节

        pcolortable[0].BYTErgbRed = 0x00;
        pcolortable[0].BYTErgbGreen=0xF8;
        pcolortable[0].BYTErgbBlue =0x00;
        pcolortable[0].BYTErgbReserved=0x00;

        pcolortable[1].BYTErgbRed = 0xE0;
        pcolortable[1].BYTErgbGreen=0x07;
        pcolortable[1].BYTErgbBlue =0x00;
        pcolortable[1].BYTErgbReserved=0x00;

        pcolortable[2].BYTErgbRed = 0x1F;
        pcolortable[2].BYTErgbGreen=0x00;
        pcolortable[2].BYTErgbBlue =0x00;
        pcolortable[2].BYTErgbReserved=0x00;

        pcolortable[3].BYTErgbRed = 0x00;
        pcolortable[3].BYTErgbGreen=0x00;
        pcolortable[3].BYTErgbBlue =0x00;
        pcolortable[3].BYTErgbReserved=0x00;*/

        bfh.bfType=0x4d42;
        bfh.bfSize=54 + height*width*2;
        bfh.bfOffBits=54;

        bih.biSize = 40;
        bih.biWidth=width;
        bih.biHeight=-height;
        bih.biPlanes=1;
        bih.biBitCount=16;
//        bih.biCompression=0x3;
        bih.biXPelsPerMeter=0;
        bih.biYpelsPerMeter=0;
        bih.biClrImportant =0;
        bih.biClrUsed = 0;
        bih.biSizeImage = height*width*2;

        uint8_t *bmp = (uint8_t *)malloc(bfh.bfSize);
        if(!bmp)
        {
            LOG_INFO("err:malloc for *bmp error");
            return ERROR_MALLOC;
        }

        memset(bmp, 0, bfh.bfSize);
        memcpy(bmp, &bfh, 14);
        memcpy(&bmp[14], &bih, 40);
//        memcpy(&bmp[54],pcolortable,4*sizeof(struct rgbquad));

        DEBUG_INFO("pic format= %d,headersize = %d,filesize =%d,biWidth =%d,biHeight=%d",format,headersize,filesize,bih.biWidth,bih.biHeight);

        LOG_ERROR("writing bmp data...");

        write_bmp_data_16bit(bmp);

        fwrite(bmp,1,bfh.bfSize,fpout);
        free(bmp);
    }

    return SUCCESS;
}

uint32_t png_2_16bit(char *input_filename,char *output_filename)
{
    if (input_filename==NULL||output_filename==NULL) {
        LOG_INFO("err:input param null");
        return ERROR_NULL;
    }
    
    FILE *fp = fopen(input_filename, "rb");
    if (!fp) {
        LOG_INFO("err:fopen %s failed",input_filename);
        return ERROR_NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return ERROR_INVALID_PARAM;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);

    // Ensure we are working with RGB or RGBA
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && png_get_bit_depth(png, info) < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_bit_depth(png, info) == 16) {
        png_set_strip_16(png);
    }
    if (color_type == PNG_COLOR_TYPE_RGB_ALPHA || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_strip_alpha(png);
    }

    png_read_update_info(png, info);  // Update info structure

    // Prepare for output
    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        LOG_INFO("err:fopen %s failed",output_filename);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return ERROR_NULL;
    }

    // Create a new PNG file for output
    png_structp png_out = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_out = png_create_info_struct(png_out);
    
    if (setjmp(png_jmpbuf(png_out))) {
        png_destroy_write_struct(&png_out, &info_out);
        fclose(outfile);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return ERROR_INVALID_PARAM;
    }

    png_init_io(png_out, outfile);
    
    // Set output PNG properties
    png_set_IHDR(png_out, info_out, width, height, 16, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png_out, info_out);

    png_bytep row = (png_bytep)malloc(png_get_rowbytes(png, info));

    for (int y = 0; y < height; y++) {
        png_read_row(png, row, NULL);
        for (int x = 0; x < width; x++) {
            int r = row[x * 3];        // Red channel
            int g = row[x * 3 + 1];    // Green channel
            int b = row[x * 3 + 2];    // Blue channel

            // Convert to 16-bit RGB565 format
            uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            fwrite(&rgb565, sizeof(uint16_t), 1, outfile);
        }
        free(row);
    }

    // Finish writing
    png_write_end(png_out, NULL);

    // Clean up
    fclose(outfile);
    png_destroy_write_struct(&png_out, &info_out);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return SUCCESS;
}

uint32_t png_2_bmp(char *inname,char *outname,uint8_t format)
{
    LOG_ERROR("png_2_bmp,inname=%s,outname=%s,format=%d",inname,outname,format);
    if (format != PIC_FORMAT_RGB565 && format != PIC_FORMAT_RGB888)
    {
        LOG_INFO("err:picture format conversion error,picture format in invalid");
        return ERROR_PICTUREFORMAT;
    }
    //读
    row_buf = NULL;
    //打开读文件
    if ((fpin = fopen(inname, "rb")) == NULL)
    {
        fprintf(stderr,"Could not find input file %s\n", inname);
        return ERROR_PNGREAD;
    }
    //打开写文件
    if ((fpout = fopen(outname, "wb+")) == NULL)
    {
        printf("Could not open output file %s\n", outname);
        fclose(fpin);
        return ERROR_BMPWRITE;
    }
    //检查一下这个PNG文件是不是真正的PNG文件
    unsigned char header[8] = {0};
    DEBUG_INFO("this picture format is PNG_000_1");
    fread(header,1,8,fpin);
    if((header[0] == 0x89 &&header[1] == 'P' &&header[2] == 'N' &&header[3] == 'G' &&header[4] == 0x0D &&header[5] == 0x0A &&header[6] == 0x1A &&header[7] == 0x0A) == 0)
    {
        fclose(fpin);
        DEBUG_INFO("err: this picture format is not PNG ");
        return ERROR_PNGREAD;
    }
    DEBUG_INFO("this picture format is PNG_000_2");
    fseek(fpin, 0, SEEK_SET);
    //我们这里不处理未知的块unknown chunk
    //初始化1
    read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    read_info_ptr = png_create_info_struct(read_ptr);
    end_info_ptr = png_create_info_struct(read_ptr);
    write_info_ptr = png_create_info_struct(write_ptr);
    write_end_info_ptr = png_create_info_struct(write_ptr);
    //初始化2
    png_init_io(read_ptr, fpin);
    png_init_io(write_ptr, fpout);

	int status=PASS;

	if((status=read_png_info())!=PASS)	return status;
    if(write_bmp(format)!=PASS) return status;
//	if((status=write_bmp_header())!=PASS)return status;
//	if((status=write_bmp_data())!=PASS)	return status;


   fclose(fpin);
   fclose(fpout);


	LOG_ERROR("file write over");

//    DEBUG_INFO("pic_info:pic_width:%d, pic_height:%d,",width,height);
//    DEBUG_INFO("bit_depth:%d, color_type:%d",bit_depth,color_type);
//    DEBUG_INFO("interlace_type:%d, compression_type:%d, filter_type:%d",interlace_type, compression_type, filter_type);
//    DEBUG_INFO("palette num:%d",num_palette);
//     get_rgb565("/sdcard/lx/rgb888.bmp","/sdcard/lx/rgb565.bmp");
    return SUCCESS;

//   return status;
}

uint32_t protocol_makephoto_png2bmp(char *inname, char *outame, uint8_t format)
{
    return png_2_bmp(inname,outame,format);
}




