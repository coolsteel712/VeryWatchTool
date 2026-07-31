
#include "bitmap_tool.h"
#include "stb_image.h"
#include "bitmap_font.h"
#include "include_help.h"
#include "error.h"
#include "debug.h"
#include "lz4.h"
#include "fastlz_util.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"
#include "protocol_func_table.h"

#define PIXEL_BUFF_MAX_SIZE        (1024*1024)
//#define WACTH_FACE_TEMPORARY
int compress_type = 0;//0:不压缩 1:lz4压缩 2:fastlz压缩
static uint32_t image_load_buff(const uint8_t *buff, uint32_t buff_size, bitmap_t *image) {
    int w = 0;
    int h = 0;
    int n = 0;
    uint32_t ret = RET_FAIL;


    uint8_t *data = stbi_load_from_memory(buff, buff_size, &w, &h, &n, 0);
    return_value_if_fail(data != NULL, RET_FAIL);
    //判断照片是否透明
    if (rgba_data_is_opaque(data, w, h, n)) {
        ret = bitmap_init_from_rgba(image, w, h, BITMAP_FMT_BGR565, data, n);
    } else {
        ret = bitmap_init_from_rgba(image, w, h, BITMAP_FMT_RGBA8888, data, n);
    }

    stbi_image_free((uint8_t *) (data));


    return ret;
}
/**
 * @brief lz4压缩缓存 嵌入式测试 2022-10-20
 * @param src 源数据
 * @param dst 压缩得到数据
 * @param src_size 源数据大小
 * @return int 压缩得到的数据大小,压缩失败则为0
 * */
//static uint32_t lz4_compression_buff(const char *src, char *dst, int src_size)
//{
//    return LZ4_compress_default(src,dst,src_size,LZ4_compressBound(src_size));
//}


/* 开发前提：
 * 由于使用stbi库加载 用第三方工具转换的位图(格式RGB565)会由崩溃的情况(使用第三方工具转换格式的位图头部信息损坏),
 * 所以遍历到文件后缀名是"_565.bmp"的图片使用下面函数加载和打包
 *
 * 函数功能：
 * 		加载rgb565位图文件, 制作RAW头部 + RGB565像素数据，以便打包表盘文件(.iwf)
 */

uint32_t bitmap_tool_load_rgb565_pic(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        LOG_INFO("err:open %s failed", file_name);
        return ERROR_NOT_FIND;
    }

    fseek(file, 0, SEEK_END);
    uint32_t file_size = (uint32_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buf = malloc(file_size);
    if (buf == NULL) {
        return ERROR_NO_MEM;
    }
    LOG_ERROR("bitmap_tool_load_rgb565_pic, filename:%s, filesize:%d", file_name, file_size);
    fread(buf, 1, file_size, file);

    fclose(file);

    struct bmp_fileheader *bfh = (struct bmp_fileheader *) buf;
    unsigned int rgb_data_offset = bfh->bfOffBits;
    int file_head_size = sizeof(struct bmp_fileheader);
    struct bmp_infoheader *bih = (struct bmp_infoheader *) (buf + file_head_size);
    unsigned int info_head_size = bih->biSize;

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';

    font_head->width = bih->biWidth;
    font_head->height = abs(bih->biHeight);
    font_head->format = format;
    font_head->alpha_format = FONT_FORMAT_NONE;
    font_head->alpha_data_start_offset = 0;


    LOG_ERROR(
            "image:%s, width=%d, height=%d, bih->biHeight = %d, format=%d, rgb_data_offset=%d, file_head_size=%d, info_head_size=%d, sizeof(font_head_t):%d, bih->biSizeImage:%d",
            file_name, font_head->width, font_head->height, bih->biHeight, font_head->format, rgb_data_offset,
            file_head_size, info_head_size, sizeof(font_head_t), bih->biSizeImage);

    uint8_t tempbuf[bih->biSizeImage];
    if (bih->biHeight > 0)//图像的高度，如果该值是一个正数，说明图像是倒向的，如果该值是一个负数，则说明图像是正向的。
    {
        uint8_t *out = NULL;
        for (int height = 0; height < bih->biHeight; height++) {
            out = buf + rgb_data_offset + bih->biSizeImage - ((height + 1) * bih->biWidth * 2);
            for (int width = 0; width < font_head->width * 2; width++) {
                tempbuf[height * font_head->width * 2 + width] = *out++;
            }
        }
    } else
        memcpy(tempbuf, buf + rgb_data_offset, bih->biSizeImage);

    uint16_t cRGB565;
//	memcpy(data_buff+sizeof(font_head_t), tempbuf, bih->biSizeImage);
    uint16_t *in = (uint16_t *) data_buff + sizeof(font_head_t);
    for (int i = 0; i < bih->biSizeImage - 1; i++) {
        cRGB565 = tempbuf[i++] << 8;
        cRGB565 |= tempbuf[i];

//		cARGB1555 = 0x0000;
//		cARGB1555 |= cRGB565 & 0x001f;      //Get ARGB1555 low 5 bits(B)
//		cARGB1555 |= (cRGB565 & 0x07c0)>>1; //Get ARGB1555 middle 5 bits(G)
//		cARGB1555 |= (cRGB565 & 0xf800)>>1; //Get ARGB1555 high 5 bits(R)
        *in++ = cRGB565;
    }

//	if (strcmp(file_name, "/storage/emulated/0/lx/watchFileTemp/image_565.bin") == 0)
//    {
//        FILE *fp = fopen("/sdcard/lx/so_outBmp565.bmp","w+");
//        fwrite(data_buff,sizeof(font_head_t)+bih->biSizeImage,1,fp);
//        fclose(fp);
//        DEBUG_INFO("write /sdcard/lx/so_outBmp565.bmp data,write size=%d,image:%s",rgb_data_offset+bih->biSizeImage,file_name);
//    }
    LOG_ERROR("bitmap_tool_load_rgb565_pic Success,file size:%d", sizeof(font_head_t) + bih->biSizeImage);
    return sizeof(font_head_t) + bih->biSizeImage;
}

uint32_t bitmap_tool_load_file(const char *file_name, bitmap_t *bitmap) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        LOG_INFO("err:open %s failed", file_name);
        return ERROR_NOT_FIND;
    }

    fseek(file, 0, SEEK_END);
    uint32_t file_size = (uint32_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *data_buff = malloc(file_size);
    if (data_buff == NULL) {
        return ERROR_NO_MEM;
    }
    fread(data_buff, 1, file_size, file);

    fclose(file);

    return image_load_buff(data_buff, file_size, bitmap);

}

//使用新的24bit组包逻辑且存在lz4压缩 需要功能表支持
static uint32_t get_565_pixel_buff_24bit(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format,
                                         bitmap_t *bitmap) {
    DEBUG_INFO("img %s %d x %d alpha = %d, compress type:%d", file_name, bitmap->w, bitmap->h,
               bitmap->format == BITMAP_FMT_RGBA8888, compress_type);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';

    font_head->width = bitmap->w;
    font_head->height = bitmap->h;
    font_head->format = format;

    if (bitmap->format != BITMAP_FMT_RGBA8888) {
        font_head->alpha_format = FONT_FORMAT_NONE;
        font_head->alpha_data_start_offset = 0;
    }

    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    rgba_t rgb;
    size_t pixel_buff_offset = 0;

    for (int h = 0; h < bitmap->h; h++) {
        for (int w = 0; w < bitmap->w; w++) {
            bitmap_get_pixel(bitmap, w, h, &rgb);

            pixel_buff[pixel_buff_offset++] = (((rgb.g << 3) & 0xe0) | rgb.b >> 3) & 0xff;
            pixel_buff[pixel_buff_offset++] = (rgb.r & 0xf8) | (rgb.g >> 5);
            if (bitmap->format == BITMAP_FMT_RGBA8888) {
                font_head->alpha_format = FONT_FORMAT_MONO4;
                pixel_buff[pixel_buff_offset++] = rgb.a;
            }
        }
    }

    if (compress_type == LZ4_TYPE) {
        int src_size = (int) pixel_buff_offset;
        uint8_t *src_buff = malloc(src_size);
        memcpy(src_buff, pixel_buff, src_size);
        memset(pixel_buff, 0, src_size);
        pixel_buff_offset = LZ4_compress_default((const char *) src_buff, (char *) pixel_buff, src_size,
                                                 LZ4_compressBound(src_size));
        LOG_ERROR("LZ4_compress_default,file_name:%s,format:%d,src_size:%d,dst_size:%d", file_name, format, src_size,
                  pixel_buff_offset);
        free(src_buff);
    }

    bitmap_destroy(bitmap);
    return pixel_buff_offset + sizeof(font_head_t);

}

//使用douiv6表盘框架压缩图片像素 需要功能表支持
static uint32_t get_565_pixel_buff_douiv6(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format,
                                          bitmap_t *bitmap) {
    bool is_swap_color = false;
    if (format == FONT_FORMAT_BGR565 || format == FONT_FORMAT_ABGR565) {
        is_swap_color = true;
    }

    DEBUG_INFO("img %s %d x %d alpha = %d,is_swap_color = %d", file_name, bitmap->w, bitmap->h,
               bitmap->format == BITMAP_FMT_RGBA8888, is_swap_color);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';

    font_head->width = bitmap->w;
    font_head->height = bitmap->h;
    font_head->format = format;

    if (bitmap->format != BITMAP_FMT_RGBA8888) {
        font_head->alpha_format = FONT_FORMAT_NONE;
        font_head->alpha_data_start_offset = 0;
    }

    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    rgba_t rgb;
    uint16_t rgb_value;

    size_t pixel_buff_offset = 0, file_size = (int) sizeof(font_head_t), src_pixel_data_size = 0, src_alpha_data_size = 0;

    for (int h = 0; h < bitmap->h; h++) {
        for (int w = 0; w < bitmap->w; w++) {
            bitmap_get_pixel(bitmap, w, h, &rgb);

            if (is_swap_color == false) {
                //rgb
                rgb_value = pixel_rbg565_encode(rgb);
            } else {
                //bgr
                rgb_value = pixel_bgr565_encode(rgb);
            }

            pixel_buff[pixel_buff_offset++] = (rgb_value >> 8) & 0xff;
            pixel_buff[pixel_buff_offset++] = rgb_value & 0xff;
        }
    }

    src_pixel_data_size = pixel_buff_offset;

    uint8_t *src_buff = malloc(src_pixel_data_size);
    memset(src_buff, 0, src_pixel_data_size);
    memcpy(src_buff, pixel_buff, src_pixel_data_size);
    memset(pixel_buff, 0, src_pixel_data_size);
    pixel_buff_offset = fastlz_util_compress_buff((const char *) src_buff, (char *) pixel_buff,
                                                  (int) src_pixel_data_size, 1024);
    file_size += pixel_buff_offset;
    LOG_ERROR("fastlz pixel data finish,after compress pixel size data:%d file size:%d", pixel_buff_offset, file_size);

    //追加透明通道
    if (bitmap->format == BITMAP_FMT_RGBA8888) {
        font_head->alpha_format = FONT_FORMAT_MONO4;
        font_head->alpha_data_start_offset = pixel_buff_offset;

        for (int h = 0; h < bitmap->h; h++) {
            for (int w = 0; w < (((bitmap->w + 1) / 2) * 2); w += 2)    // ÷2*2 是为了2对齐
            {
                bitmap_get_pixel(bitmap, w, h, &rgb);

                pixel_buff[pixel_buff_offset] = (rgb.a & 0xf0);

                bitmap_get_pixel(bitmap, w + 1, h, &rgb);

                pixel_buff[pixel_buff_offset] = (rgb.a >> 4) | pixel_buff[pixel_buff_offset];

                pixel_buff_offset++;
                src_alpha_data_size++;
            }
        }
        memset(src_buff, 0, src_pixel_data_size);
        memcpy(src_buff, &pixel_buff[font_head->alpha_data_start_offset], src_alpha_data_size);
        memset(&pixel_buff[font_head->alpha_data_start_offset], 0, src_alpha_data_size);
        pixel_buff_offset = fastlz_util_compress_buff((const char *) src_buff,
                                                      (char *) &pixel_buff[font_head->alpha_data_start_offset],
                                                      (int) src_alpha_data_size, 1024);
        file_size += pixel_buff_offset;

        LOG_ERROR("fastlz alpha data finish,after compress alpha size data %d file size:%d", pixel_buff_offset,
                  file_size);
    }

    bitmap_destroy(bitmap);
    free(src_buff);
    return file_size;
}


static uint32_t get_565_pixel_buff(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    uint32_t err_code;
    bitmap_t bitmap;
    bool is_swap_color = false;
    char *temp = strrchr(file_name, '_');
    if (temp != NULL) {
        if (strcmp(temp, "_565.bmp") == 0) {
            LOG_ERROR("file_name:%s, use func:bitmap_tool_load_rgb565_pic load file..", file_name);
            return bitmap_tool_load_rgb565_pic(file_name, data_buff, buff_size, format);
        } else
            err_code = bitmap_tool_load_file(file_name, &bitmap);

        //功能表支持的话且使用新定制的表盘包的话, 用24bit的组包逻辑, 不支持还是走原来的16+4 2022-12-9
        if (protocol_func_support_lvgl_watch_dial_frame() &&
            (!strcmp(temp, "_24bit.bmp") || !strcmp(temp, "_24bit.BMP") || !strcmp(temp, "_24bit.png") ||
             !strcmp(temp, "_24bit.PNG"))) {
            LOG_ERROR("use 24bit format mode to make up watch dail data ...");
            return get_565_pixel_buff_24bit(file_name, data_buff, buff_size, format, &bitmap);
        }

        //功能表支持新表盘框架douiv6
        if (compress_type == FASTLZ_TYPE &&
            (!strcmp(temp, "_lz.bmp") || !strcmp(temp, "_lz.BMP") || !strcmp(temp, "_lz.png") ||
             !strcmp(temp, "_lz.PNG"))) {
            LOG_ERROR("use douiv6 dial compress pixel data ...");
            return get_565_pixel_buff_douiv6(file_name, data_buff, buff_size, format, &bitmap);
        }
    } else
        err_code = bitmap_tool_load_file(file_name, &bitmap);
    if (err_code != SUCCESS) {
        LOG_INFO("err:load image file %s failed err code:%d", file_name, err_code);
        return 0;
    }

    if (format == FONT_FORMAT_BGR565 || format == FONT_FORMAT_ABGR565) {
        is_swap_color = true;
    }

    LOG_ERROR("img %s %d x %d alpha = %d,is_swap_color = %d", file_name, bitmap.w, bitmap.h,
              bitmap.format == BITMAP_FMT_RGBA8888, is_swap_color);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';

    font_head->width = bitmap.w;
    font_head->height = bitmap.h;
    font_head->format = format;

    if (bitmap.format != BITMAP_FMT_RGBA8888) {
        font_head->alpha_format = FONT_FORMAT_NONE;
        font_head->alpha_data_start_offset = 0;
    }


    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    rgba_t rgb;
    uint16_t rgb_value;

    size_t pixel_buff_offset = 0;

    for (int h = 0; h < bitmap.h; h++) {
        for (int w = 0; w < bitmap.w; w++) {

            bitmap_get_pixel(&bitmap, w, h, &rgb);

            if (is_swap_color == false) {
                //rgb
                rgb_value = pixel_rbg565_encode(rgb);
            } else {
                //bgr
                rgb_value = pixel_bgr565_encode(rgb);
            }

            pixel_buff[pixel_buff_offset++] = (rgb_value >> 8) & 0xff;
            pixel_buff[pixel_buff_offset++] = rgb_value & 0xff;
        }
    }

    //追加透明通道
    if (bitmap.format == BITMAP_FMT_RGBA8888) {

        font_head->alpha_format = FONT_FORMAT_MONO4;
        font_head->alpha_data_start_offset = (uint32_t) pixel_buff_offset;

        for (int h = 0; h < bitmap.h; h++) {
            for (int w = 0; w < (((bitmap.w + 1) / 2) * 2); w += 2)    // ÷2*2 是为了2对齐
            {

                bitmap_get_pixel(&bitmap, w, h, &rgb);

                pixel_buff[pixel_buff_offset] = (rgb.a & 0xf0);

                bitmap_get_pixel(&bitmap, w + 1, h, &rgb);


                pixel_buff[pixel_buff_offset] = (rgb.a >> 4) | pixel_buff[pixel_buff_offset];

                pixel_buff_offset++;

            }
        }
    }
    bitmap_destroy(&bitmap);
    return pixel_buff_offset + sizeof(font_head_t);
}

static uint32_t get_222_pixel_buff(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    uint32_t err_code;
    bitmap_t bitmap;

    bool is_swap_color = false;

    err_code = bitmap_tool_load_file(file_name, &bitmap);
    if (err_code != SUCCESS) {
        LOG_INFO("err:load image file %s failed err code:%d", file_name, err_code);
        return 0;
    }

    if (format == FONT_FORMAT_BGR222 || format == FONT_FORMAT_ABGR222) {
        is_swap_color = true;
    }

    LOG_ERROR("img %s %d x %d alpha = %d \r\n", file_name, bitmap.w, bitmap.h, bitmap.format == BITMAP_FMT_RGBA8888);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';

    font_head->width = bitmap.w;
    font_head->height = bitmap.h;
    font_head->format = format;

    if (bitmap.format != BITMAP_FMT_RGBA8888) {
        font_head->alpha_format = FONT_FORMAT_NONE;
        font_head->alpha_data_start_offset = 0;
    }

    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    rgba_t rgb;
    uint8_t rgb_value;

    size_t pixel_buff_offset = 0;

    for (int h = 0; h < bitmap.h; h++) {
        for (int w = 0; w < bitmap.w; w++) {
            bitmap_get_pixel(&bitmap, w, h, &rgb);

            if (is_swap_color == false) {
                //rgb
                rgb_value = pixel_rgb222_encode(rgb);
            } else {
                //bgr
                rgb_value = pixel_bgr222_encode(rgb);
            }

            pixel_buff[pixel_buff_offset++] = rgb_value;
        }
    }

    //追加透明通道
    //222格式的alpha转成1bit
    if (bitmap.format == BITMAP_FMT_RGBA8888) {
        font_head->alpha_format = FONT_FORMAT_MONO4;
        font_head->alpha_data_start_offset = (uint32_t) pixel_buff_offset;

        for (int h = 0; h < bitmap.h; h++) {
            for (int w = 0; w < (((bitmap.w + 7) / 8) * 8); w += 8) {
                uint8_t alpha = 0;

                for (int i = 0; i < 8; i++) {
                    bitmap_get_pixel(&bitmap, w + i, h, &rgb);

                    alpha |= (rgb.a & 0x80) >> i;
                }

                pixel_buff[pixel_buff_offset] = alpha;
                pixel_buff_offset++;
            }
        }
    }
    bitmap_destroy(&bitmap);
    return (uint32_t) (pixel_buff_offset + sizeof(font_head_t));
}

static uint32_t get_mono4_pixel_buff(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    uint32_t err_code;
    bitmap_t bitmap;
    int add64bit = 0;
    memset(data_buff, 0, buff_size);
    char *temp = strrchr(file_name, '_');
    if (temp != NULL) {
        if (strcmp(temp, "_24bit.bmp") == 0 && protocol_func_support_lvgl_watch_dial_frame()) {
            add64bit = 1;
//            DEBUG_INFO("pic buf add 64byte '0x00' data");
        }
    }
    err_code = bitmap_tool_load_file(file_name, &bitmap);
    if (err_code != SUCCESS) {
        return 0;
    }

    LOG_ERROR("img %s w : %d , h : %d ", file_name, bitmap.w, bitmap.h);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';

    font_head->width = bitmap.w;
    font_head->height = bitmap.h;
    font_head->format = format;
    font_head->alpha_format = FONT_FORMAT_NONE;
    font_head->alpha_data_start_offset = 0;


    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    rgba_t rgb;

    size_t pixel_buff_offset = 0;

    for (int h = 0; h < bitmap.h; h++) {
        for (int w = 0; w < ((bitmap.w / 2) * 2); w += 2)    // ÷2*2 是为了2对齐
        {

            bitmap_get_pixel(&bitmap, w, h, &rgb);

            pixel_buff[pixel_buff_offset] = (rgb.b & 0xf0);

            bitmap_get_pixel(&bitmap, w + 1, h, &rgb);


            pixel_buff[pixel_buff_offset] = (rgb.b >> 4) | pixel_buff[pixel_buff_offset];

            pixel_buff_offset++;


        }
    }
    if (add64bit && compress_type == LZ4_TYPE) {
        int src_size = (int) pixel_buff_offset;
        uint8_t *src_buff = malloc(src_size);
        memcpy(src_buff, pixel_buff, src_size);
        memset(pixel_buff, 0, src_size);
        pixel_buff_offset = LZ4_compress_default((const char *) src_buff, (char *) pixel_buff, src_size,
                                                 LZ4_compressBound(src_size));
        LOG_ERROR("LZ4_compress_default,file_name:%s,format:%d,src_size:%d,dst_size:%d", file_name, format, src_size,
                  pixel_buff_offset);
        free(src_buff);
    }
    bitmap_destroy(&bitmap);
    return (uint32_t) (pixel_buff_offset + sizeof(font_head_t));
}

static uint32_t get_jpg_pixel_buff(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    uint32_t err_code;
    bitmap_t bitmap;

    err_code = bitmap_tool_load_file(file_name, &bitmap);
    if (err_code != SUCCESS) {
        DEBUG_INFO("err:load file %s error(%d)", file_name, err_code);
        return 0;
    }

    DEBUG_INFO("img %s %d x %d", file_name, bitmap.w, bitmap.h);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'J';
    font_head->type[1] = 'P';
    font_head->type[2] = 'G';
    font_head->type[3] = '\0';
    font_head->width = bitmap.w;
    font_head->height = bitmap.h;
    font_head->format = FONT_FORMAT_NONE;
    font_head->alpha_format = FONT_FORMAT_NONE;
    font_head->alpha_data_start_offset = 0;

    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        LOG_INFO("err:fopen %s fail");
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    int file_size = (int) ftell(fp);

    if ((file_size + sizeof(font_head_t)) > PIXEL_BUFF_MAX_SIZE) {
        LOG_INFO("err:file %s size(%d) more than max buf size", file_name, file_size);
    } else
        DEBUG_INFO("file %s size:%d", file_name, file_size);

    fseek(fp, 0, SEEK_SET);
    fread(pixel_buff, 1, file_size, fp);
    fclose(fp);
    bitmap_destroy(&bitmap);
    return file_size + sizeof(font_head_t);
}

static uint32_t get_8888_pixel_buff(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    uint32_t err_code;
    bitmap_t bitmap;
    bool is_swap_color = false;

    err_code = bitmap_tool_load_file(file_name, &bitmap);
    if (err_code != SUCCESS) {
        DEBUG_INFO("err:load file %s error(%d)", file_name, err_code);
        return 0;
    }

    if (format == FONT_FORAMT_BGR888 || format == FONT_FORAMT_ABGR888) {
        is_swap_color = true;
    }

    DEBUG_INFO("img %s %d x %d alpha = %d,bitmap.format:%d,is_swap_color = %d", file_name, bitmap.w, bitmap.h,
               bitmap.format == BITMAP_FMT_RGBA8888, bitmap.format, is_swap_color);

    //打包头部信息
    font_head_t *font_head = (font_head_t *) data_buff;
    font_head->type[0] = 'R';
    font_head->type[1] = 'A';
    font_head->type[2] = 'W';
    font_head->type[3] = '\0';
    font_head->width = bitmap.w;
    font_head->height = bitmap.h;
    font_head->format = format;
    font_head->alpha_data_start_offset = 0;
    font_head->alpha_format = bitmap.format != BITMAP_FMT_RGBA8888 ? FONT_FORMAT_NONE : FONT_FORMAT_MONO8;

    uint8_t *pixel_buff = &data_buff[sizeof(font_head_t)];

    rgba_t rgb;
    //uint16_t rgb_value;
    int pixel_buff_offset = 0;

    for (int h = 0; h < bitmap.h; h++) {
        for (int w = 0; w < font_head->width; w++) {
            bitmap_get_pixel(&bitmap, w, h, &rgb);

            if (is_swap_color == false) {
                //argb
                pixel_buff[pixel_buff_offset++] = bitmap.format != BITMAP_FMT_RGBA8888 ? 0 : rgb.a;
                pixel_buff[pixel_buff_offset++] = rgb.r;
                pixel_buff[pixel_buff_offset++] = rgb.g;
                pixel_buff[pixel_buff_offset++] = rgb.b;
            } else {
                //bgra
                pixel_buff[pixel_buff_offset++] = rgb.b;
                pixel_buff[pixel_buff_offset++] = rgb.g;
                pixel_buff[pixel_buff_offset++] = rgb.r;
                pixel_buff[pixel_buff_offset++] = bitmap.format != BITMAP_FMT_RGBA8888 ? 0 : rgb.a;
            }
        }
    }

    bitmap_destroy(&bitmap);
    return pixel_buff_offset + sizeof(font_head_t);
}


uint32_t bitmap_tool_get_font_buff(const char *file_name, uint8_t *data_buff, uint32_t buff_size, uint8_t format) {
    switch (format) {
        case PICTURE_FORMAT_JPG :
            return get_jpg_pixel_buff(file_name, data_buff, buff_size, format);
        case FONT_FORMAT_RGB565 :
        case FONT_FORMAT_ARGB565 :
        case FONT_FORMAT_BGR565:
        case FONT_FORMAT_ABGR565:
            return get_565_pixel_buff(file_name, data_buff, buff_size, format);
            break;
        case FONT_FORMAT_MONO4 :
            return get_mono4_pixel_buff(file_name, data_buff, buff_size, format);
            break;
        case FONT_FORMAT_RGB222:
        case FONT_FORMAT_ARGB222:
        case FONT_FORMAT_BGR222:
        case FONT_FORMAT_ABGR222:
            return get_222_pixel_buff(file_name, data_buff, buff_size, format);
            break;
        case FONT_FORMAT_ARGB888:
        case FONT_FORAMT_BGR888:
            return get_8888_pixel_buff(file_name, data_buff, buff_size, format);
            break;
        default:
            break;
    }

    return ERROR_NOT_SUPPORTED;
}

//是否为png
bool isPNG(const char *filePath) {
    int width, height, channels;
    stbi_info(filePath, &width, &height, &channels);
    return channels == STBI_rgb_alpha;
}

//加载图片jpg
uint8_t *jpg_stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp) {
    return (uint8_t *) stbi_load(filename, x, y, comp, req_comp);
}

//写png
int jpg_stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes) {
    int rs = stbi_write_png(filename, w, h, comp, data, stride_in_bytes);
    stbi_image_free((uint8_t *) data);
    return rs;
}

