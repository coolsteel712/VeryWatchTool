/*
 * protocol_func_table.h
 *
 *  Created on: 2016年1月9日
 *      Author: Administrator
 */

#ifndef PROTOCOL_FUNC_TABLE_H_
#define PROTOCOL_FUNC_TABLE_H_

#include "include_help.h"

#ifdef __cplusplus
extern "C" {
#endif

//判断固件支持lvgl表盘框架
extern bool protocol_func_support_lvgl_watch_dial_frame(void);

//针对DOUIv6版本表盘框架的动画像素进行fastlz压缩,优化固件存储大小
extern bool protocol_func_support_douiv6_watch_dial_anima_compress_pixel_data(void);

//支持表盘打包解码JPG图片
extern bool protocol_func_support_make_watch_dial_decode_jpg(void);


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_FUNC_TABLE_H */
