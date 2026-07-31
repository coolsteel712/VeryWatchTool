//
//  cpp2c.cpp
//  protocol_c
//
//  Created by hedongyang on 2022/9/27.
//

#include "make_file.h"
#include "make_watch_face.h"
#include "file_compression.h"

//制作(IWF)文件
extern "C" uint32_t make_iwf_file(const char *file_path, const char *save_file_path, uint8_t fontFormat) {
    mkWatchFace mk;
    return mk.makeIwfFile(file_path, save_file_path, fontFormat);
}
