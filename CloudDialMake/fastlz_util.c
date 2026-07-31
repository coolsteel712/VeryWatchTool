
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fastlz.h"
#include "fastlz_util.h"
#include "debug.h"

#include "error.h"

#define BLOCK_HEADER_SIZE              4

#define DCOMPRESS_BUFFER_SIZE          4096

/* The output buffer must be at least 5% larger than the input buffer and can not be smaller than 66 bytes */

/* compress level: 1 or 2 */
#define FASTLZ_COMPRESS_LEVEL          2

int fastlz_util_compress_buff(const void* src_buff, void* dst_buff,int src_buff_len,uint32_t compress_block_size)
{
    int block_size=0,ret=SUCCESS,cmprs_size=0,totle_cmprs_size=0;
    uint8_t *buffer=NULL,*cmprs_buffer=NULL;
    uint8_t buffer_hdr[BLOCK_HEADER_SIZE] = { 0 };
    if (src_buff==NULL || dst_buff==NULL)
    {
        LOG_INFO("err:%s%s",!src_buff?"src buff null ":"",!dst_buff?"dst buff null":"");
        return -1;
    }

    cmprs_buffer = (uint8_t *)malloc(compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));
    buffer = (uint8_t *)malloc(compress_block_size);

    if (cmprs_buffer==NULL || buffer==NULL)
    {
        LOG_INFO("err:%s%s",!cmprs_buffer?"malloc cmprs buffer failed ":"",!buffer?"malloc buffer failed":"");
        return -1;
    }

    uint8_t *dstptr = dst_buff;
    uint8_t *srcPtr = src_buff;

    for (size_t i = 0; i < src_buff_len; i += compress_block_size)
    {
        if ((src_buff_len - i) < compress_block_size)
        {
            block_size = src_buff_len - i;
        }
        else
        {
            block_size = compress_block_size;
        }


        memset(buffer, 0x00, compress_block_size);
        memset(cmprs_buffer, 0x00, compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));

        memcpy(buffer,srcPtr,block_size);
        srcPtr += block_size;

//        DEBUG_INFO("srcPtr:%p",srcPtr);

        /* The destination buffer must be at least size + 400 bytes large because incompressible data may increase in size. */
        cmprs_size = fastlz_compress_level(FASTLZ_COMPRESS_LEVEL, buffer, block_size, (char *)cmprs_buffer);
        if (cmprs_size < 0)
        {
            LOG_INFO("err:fastlz compress failed,compress offset:%d",i);
            ret = -1;
            break;
        }

        //如果通过fastlz快速压缩出来的数据大小大于块大小的话，需要在分段再压缩，确保不超过块大小 2023-6-15
        if (cmprs_size > compress_block_size)
        {
            DEBUG_INFO("warn:fastlz compress output size(%d) more than BLOCKSIZE(%d)",cmprs_size,compress_block_size);
            //分两段压
            for (int w = 0; w < 2; ++w) {
                memset(cmprs_buffer,0,compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));
                cmprs_size = fastlz_compress_level(FASTLZ_COMPRESS_LEVEL, buffer + (w * (block_size/2)), block_size/2, (char *)cmprs_buffer);
                DEBUG_INFO("[%d]two stage cmprs get coprs size:%d block size:%d",w,cmprs_size,block_size/2);

                buffer_hdr[3] = cmprs_size % (1 << 8);
                buffer_hdr[2] = (cmprs_size % (1 << 16)) / (1 << 8);
                buffer_hdr[1] = (cmprs_size % (1 << 24)) / (1 << 16);
                buffer_hdr[0] = cmprs_size / (1 << 24);

                DEBUG_INFO("cmprs_size:%d,buffer_hdr[3]:0x%X,buffer_hdr[2]:0x%X,buffer_hdr[1]:0x%X,buffer_hdr[0]:0x%X",cmprs_size,buffer_hdr[3],buffer_hdr[2],buffer_hdr[1],buffer_hdr[0]);
                memcpy(dstptr,buffer_hdr,BLOCK_HEADER_SIZE);
                dstptr += BLOCK_HEADER_SIZE;
                memcpy(dstptr,cmprs_buffer,cmprs_size);
                dstptr += cmprs_size;

                totle_cmprs_size += cmprs_size + BLOCK_HEADER_SIZE;
            }
            continue;
        }
        
        /* Store compress block size to the block header (4 byte). */
        buffer_hdr[3] = cmprs_size % (1 << 8);
        buffer_hdr[2] = (cmprs_size % (1 << 16)) / (1 << 8);
        buffer_hdr[1] = (cmprs_size % (1 << 24)) / (1 << 16);
        buffer_hdr[0] = cmprs_size / (1 << 24);
//        DEBUG_INFO("buffer hdr:%d %d %d %d",buffer_hdr[3],buffer_hdr[2],buffer_hdr[1],buffer_hdr[0]);
//        fwrite(buffer_hdr,1, BLOCK_HEADER_SIZE, fd_out);
//        fwrite(cmprs_buffer,1, cmprs_size, fd_out);

        memcpy(dstptr,buffer_hdr,BLOCK_HEADER_SIZE);
        dstptr += BLOCK_HEADER_SIZE;
        memcpy(dstptr,cmprs_buffer,cmprs_size);
        dstptr += cmprs_size;

        totle_cmprs_size += cmprs_size + BLOCK_HEADER_SIZE;
        LOG_ERROR("i:%d,comprs size:%d totle_cmprs_size:%d",i,cmprs_size,totle_cmprs_size);
        //DEBUG_INFO(">");
    }
    DEBUG_INFO("[fastlz]compressed %d KB into %d KB,compression ratio is %d %% !", src_buff_len/1024, totle_cmprs_size / 1024,
               100 - (totle_cmprs_size * 100) / src_buff_len);

    ret = totle_cmprs_size;
    LOG_ERROR("src buff len:%d dst buf len:%d",src_buff_len,totle_cmprs_size);

    free(buffer);
    free(cmprs_buffer);
    return ret;
}

int fastlz_util_compress_file(FILE *fd_in, FILE *fd_out,uint32_t compress_block_size)
{
	/* Start to compress file  */
	uint8_t *cmprs_buffer = NULL, *buffer = NULL;
	uint8_t buffer_hdr[BLOCK_HEADER_SIZE] = { 0 };
	int cmprs_size = 0, block_size = 0, totle_cmprs_size = 0;
	int file_size = 0;
	int ret = 0;
	fseek(fd_in, 0, SEEK_END);
	file_size = (int)ftell(fd_in);

	if (file_size == 0)
	{
		DEBUG_INFO("ERROR : input file size == 0  ");
		ret = -1;
		goto _exit;
	}
	fseek(fd_in, 0, SEEK_SET);

	cmprs_buffer = (uint8_t *)malloc(compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));
	buffer = (uint8_t *)malloc(compress_block_size);
	if (!cmprs_buffer || !buffer)
	{
		DEBUG_INFO("[fastlz] No memory for cmprs_buffer or buffer!");
		ret = -1;
		goto _exit;
	}

	LOG_ERROR("[fastlz]compress start : ");
	for (size_t i = 0; i < file_size; i += compress_block_size)
	{
		if ((file_size - i) < compress_block_size)
		{
			block_size = file_size - i;
		}
		else
		{
			block_size = compress_block_size;
		}

		memset(buffer, 0x00, compress_block_size);
		memset(cmprs_buffer, 0x00, compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));

		fread( buffer,1, block_size, fd_in);

		/* The destination buffer must be at least size + 400 bytes large because incompressible data may increase in size. */
		cmprs_size = fastlz_compress_level(FASTLZ_COMPRESS_LEVEL, buffer, block_size, (char *)cmprs_buffer);
		if (cmprs_size < 0)
		{
			ret = -1;
			goto _exit;
		}
        
        //如果通过fastlz快速压缩出来的数据大小大于块大小的话，需要在分段再压缩，确保不超过块大小 2023-6-15
        if (cmprs_size > compress_block_size)
        {
            DEBUG_INFO("warn:fastlz compress output size(%d) more than BLOCKSIZE(%d)",cmprs_size,compress_block_size);
            //分两段压
            for (int w = 0; w < 2; ++w) {
                memset(cmprs_buffer,0,compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));
                cmprs_size = fastlz_compress_level(FASTLZ_COMPRESS_LEVEL, buffer + (w * (block_size/2)), block_size/2, (char *)cmprs_buffer);
                DEBUG_INFO("[%d]two stage cmprs get coprs size:%d block size:%d",w,cmprs_size,block_size/2);

                buffer_hdr[3] = cmprs_size % (1 << 8);
                buffer_hdr[2] = (cmprs_size % (1 << 16)) / (1 << 8);
                buffer_hdr[1] = (cmprs_size % (1 << 24)) / (1 << 16);
                buffer_hdr[0] = cmprs_size / (1 << 24);

                fwrite(buffer_hdr,1, BLOCK_HEADER_SIZE, fd_out);
                fwrite(cmprs_buffer,1, cmprs_size, fd_out);

                totle_cmprs_size += cmprs_size + BLOCK_HEADER_SIZE;
            }
            continue;
        }

		/* Store compress block size to the block header (4 byte). */
		buffer_hdr[3] = cmprs_size % (1 << 8);
		buffer_hdr[2] = (cmprs_size % (1 << 16)) / (1 << 8);
		buffer_hdr[1] = (cmprs_size % (1 << 24)) / (1 << 16);
		buffer_hdr[0] = cmprs_size / (1 << 24);

		fwrite(buffer_hdr,1, BLOCK_HEADER_SIZE, fd_out);
		fwrite(cmprs_buffer,1, cmprs_size, fd_out);

		totle_cmprs_size += cmprs_size + BLOCK_HEADER_SIZE;
	}

	LOG_ERROR("[fastlz]compressed %d KB into %d KB , compression ratio is %d %% !", file_size/1024, totle_cmprs_size / 1024,
		100 - (totle_cmprs_size * 100) / file_size);
_exit:
	if (cmprs_buffer)
	{
		free(cmprs_buffer);
	}

	if (buffer)
	{
		free(buffer);
	}

	return ret;
}


 int fastlz_util_decompress_file(FILE *fd_in, FILE *fd_out,uint32_t compress_block_size)
{
	/* Start to decompress file  */
	uint8_t *dcmprs_buffer = NULL, *buffer = NULL;
	uint8_t buffer_hdr[BLOCK_HEADER_SIZE] = { 0 };
	size_t dcmprs_size = 0, block_size = 0, total_dcmprs_size = 0;
	int file_size = 0;
	int ret = 0;

	fseek(fd_in, 0, SEEK_END);
	file_size = ftell(fd_in);
	if (file_size == 0)
	{
		DEBUG_INFO("ERROR : input file size == 0  ");
		ret = -1;
		goto _dcmprs_exit;
	}

	fseek(fd_in, 0, SEEK_SET);

	if (file_size <= BLOCK_HEADER_SIZE)
	{
		DEBUG_INFO("[fastlz] decomprssion file size : %d error!", file_size);
		ret = -1;
		goto _dcmprs_exit;
	}

	dcmprs_buffer = (uint8_t *)malloc(DCOMPRESS_BUFFER_SIZE);
	buffer = (uint8_t *)malloc(DCOMPRESS_BUFFER_SIZE + FASTLZ_BUFFER_PADDING(compress_block_size));
	if (!dcmprs_buffer || !buffer)
	{
		DEBUG_INFO("[fastlz] No memory for dcmprs_buffer or buffer!");
		ret = -1;
		goto _dcmprs_exit;
	}

	DEBUG_INFO("[fastlz]decompress start ..");
	for (size_t i = 0; i < file_size; i += BLOCK_HEADER_SIZE + block_size)
	{
		/* Get the decompress block size from the block header. */
		fread(buffer_hdr,1, BLOCK_HEADER_SIZE, fd_in);
		block_size = buffer_hdr[0] * (1 << 24) + buffer_hdr[1] * (1 << 16) + buffer_hdr[2] * (1 << 8) + buffer_hdr[3];

		memset(buffer, 0x00, compress_block_size + FASTLZ_BUFFER_PADDING(compress_block_size));
		memset(dcmprs_buffer, 0x00, DCOMPRESS_BUFFER_SIZE);

		fread(buffer,1, block_size, fd_in);

		dcmprs_size = fastlz_decompress((const void *)buffer, block_size, dcmprs_buffer, DCOMPRESS_BUFFER_SIZE);
		fwrite(dcmprs_buffer,1, dcmprs_size, fd_out);

		total_dcmprs_size += dcmprs_size;
		LOG_ERROR(">");
	}
	DEBUG_INFO("[fastlz]decompressed %d bytes into %d bytes !", file_size, total_dcmprs_size);

_dcmprs_exit:
	if (dcmprs_buffer)
	{
		free(dcmprs_buffer);
	}

	if (buffer)
	{
		free(buffer);
	}

	return ret;
}
