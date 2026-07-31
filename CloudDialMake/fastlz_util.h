

#ifndef _FASTLZ_UTIL_H_
#define _FASTLZ_UTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif

//压缩文件
extern int fastlz_util_compress_file(FILE* fd_in, FILE* fd_out,uint32_t compress_block_size);

//压缩到buff
extern int fastlz_util_compress_buff(const void* src_buff, void* dst_buff,int src_buff_len,uint32_t compress_block_size);

//解压缩文件
extern  int fastlz_util_decompress_file(FILE* fd_in, FILE* fd_out,uint32_t compress_block_size);

//命令行工具
extern int fastlz_util_main(int argc, char** argv);

#ifdef __cplusplus
}
#endif



#endif

