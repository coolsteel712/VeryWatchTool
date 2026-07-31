//
//  cpp2c.h
//  protocol_c
//
//  Created by hedongyang on 2022/9/27.
//

#ifndef cpp2c_hpp
#define cpp2c_hpp

#include "include_help.h"

#ifdef __cplusplus
extern "C" {
#endif
//制作(IWF)文件
extern uint32_t make_iwf_file(const char *file_path, const char *save_file_name, uint8_t fontFormat);
#ifdef __cplusplus
}
#endif

#endif /* cpp2c_h */
