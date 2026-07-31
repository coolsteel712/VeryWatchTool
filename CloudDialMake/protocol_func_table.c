/*
 * protocol_func_table.c
 *
 *  Created on: 2016年1月8日
 *      Author: Administrator
 */

//功能表解析

#define DEBUG_STR "[PROTOCOL_FUNC_TABLE]"

#include "debug.h"
#include "protocol_func_table.h"


//判断固件支持lvgl表盘框架
bool protocol_func_support_lvgl_watch_dial_frame(void) {
//    return func_table.v3_function_table->table29.support_lvgl_dial_frame;
    return true;
}

//针对DOUIv6版本表盘框架的动画像素进行fastlz压缩,优化固件存储大小
bool protocol_func_support_douiv6_watch_dial_anima_compress_pixel_data(void) {
//    return func_table.v3_function_table->table35.support_douiv6_watch_dial_anima_compress_pixel_data;
    return false;
};
//支持表盘打包解码JPG图片
bool protocol_func_support_make_watch_dial_decode_jpg(void) {
//    return func_table.v3_function_table->table47.support_make_watch_dial_deocde_jpg;
    return true;
};
