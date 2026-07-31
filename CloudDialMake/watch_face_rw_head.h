//
// Created by Administrator on 2019-8-6.
//

#ifndef STUDIOBLETEST_WATCH_FACE_RW_HEAD_H
#define STUDIOBLETEST_WATCH_FACE_RW_HEAD_H

#include <stdint.h>

//文件类型
#define WATCH_FACE_RW_FILE_TYPE_UNKONW			0x0
#define WATCH_FACE_RW_FILE_TYPE_IMAGE			0x01
#define WATCH_FACE_RW_FILE_TYPE_FONT			0x03
#define WATCH_FACE_RW_FILE_TYPE_JSON			0x04


//文件名最大长度,不能随意修改
#define WATCH_FACE_RW_FILE_NAME_MAX_LENGTH		30

//最大表盘数
#define WATCH_FACE_RW_FILE_LIST_MAX_NUMBER		10


#define WATCH_FACE_RW_FILE_MAGIC		"iwf"		//ido watch face
#define WATCH_FACE_RW_FILE_MAGIC_LENGTH	 (sizeof(WATCH_FACE_RW_FILE_MAGIC) - 1)


//表盘主文件
#define WATCH_FACE_RW_FILE_MAIN_FILE_NAME		   "iwf.json"
//表盘主文件1,一般用于2种布局的情况,
#define WATCH_FACE_RW_FILE_MAIN1_FILE_NAME         "iwf1.json"
//字体文件
#define WATCH_FACE_RW_FILE_FONT_FILE_NAME		   "font.json"

typedef struct watch_face_file
{
    char name[WATCH_FACE_RW_FILE_NAME_MAX_LENGTH];
    uint32_t offset;
    uint32_t length;
}watch_face_file_t;

typedef struct watch_face_file_data
{
    watch_face_file_t file;
    uint8_t *data;
    uint16_t file_name_length;
}watch_face_file_data_t;

typedef struct watch_face_file_head
{
    char magic[4];	//标识
    uint16_t version;	//版本号
    uint16_t file_count;	//文件个数
}watch_face_file_head_t;

#endif //STUDIOBLETEST_WATCH_FACE_RW_HEAD_H
