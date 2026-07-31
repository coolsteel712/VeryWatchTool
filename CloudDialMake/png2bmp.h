#ifndef _PNG2BMP_H
#define _PNG2BMP_H


#ifdef __cplusplus
extern "C" {
#endif

// png2bmp
extern uint32_t protocol_makephoto_png2bmp(char *inname, char *ouname, uint8_t format);
extern uint32_t png_2_bmp(char *inname,char *outname,uint8_t format);
extern uint32_t png_2_16bit(char *input_filename,char *output_filename);

#ifdef __cplusplus
}
#endif




#endif
