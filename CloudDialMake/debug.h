/*
 * debug.h
 *
 *  Created on: 2016年1月9日
 *      Author: Administrator
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdbool.h>

typedef enum
{
    DEBUG_LEVEL_ERROR,
    DEBUG_LEVEL_WARN,
    DEBUG_LEVEL_INFO,
}DEBUG_LEVEL;



#ifdef __cplusplus
extern "C" {
#endif

extern int cur_debug_level;
extern void debug_log_init(bool is_print_console,bool is_write_file,const char *file_path);
extern void debug_log(DEBUG_LEVEL level,const char *tag, const char *func_name,int line, const char *fmt, ...);
extern void set_log_save_day(int save_day);

#ifdef __cplusplus
}
#endif

//缩减c库流传输指令log的打印 1使能
//#define TRAN_INFO_LOG_CUT_ENABLE 0

#ifndef DEBUG_STR
#define DEBUG_STR   __FILE__
#endif

#define DEBUG_INFO(...)   debug_log(DEBUG_LEVEL_WARN,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define LOG_INFO(...)     debug_log(DEBUG_LEVEL_INFO,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define LOG_ERROR(...)    debug_log(DEBUG_LEVEL_ERROR,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define LOG_WARN(...)     debug_log(DEBUG_LEVEL_WARN,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
//断言
#define APP_ERROR_CHECK(ERR_CODE)

#endif /* DEBUG_H_ */
