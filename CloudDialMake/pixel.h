/**
 * File:   pixel.h
 * Author: AWTK Develop Team
 * Brief:  pixel format definitions
 *
 * Copyright (c) 2018 - 2019  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-10-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_PIXEL_H
#define TK_PIXEL_H

#include <stdint.h>



 /**
  * @class rgba_t
  * 颜色的四个通道。
  *
  */
typedef struct _rgba_t {
	/**
	 * @property {uint8_t} r
	 * @annotation ["readable","writable"]
	 * 红色。
	 */
	uint8_t r;
	/**
	 * @property {uint8_t} g
	 * @annotation ["readable","writable"]
	 * 绿色。
	 */
	uint8_t g;
	/**
	 * @property {uint8_t} b
	 * @annotation ["readable","writable"]
	 * 蓝色。
	 */
	uint8_t b;
	/**
	 * @property {uint8_t} a
	 * @annotation ["readable","writable"]
	 * alpha。
	 */
	uint8_t a;
} rgba_t;

/**
 * class color_t
 * 颜色。
 * @annotation ["scriptable"]
 *
 */
typedef union _color_t {
	/**
	 * @property {rgba_t} rgba
	 * @annotation ["readable", "writable"]
	 * 颜色的RGBA值。
	 */
	rgba_t rgba;
	/**
	 * @property {uint32_t} color
	 * @annotation ["readable", "writable", "scriptable"]
	 * 颜色的数值。
	 */
	uint32_t color;
} color_t;


static inline uint8_t pixel_mono4_encode(rgba_t rgba)
{
    return 0;
}


#pragma pack(push, 1)
typedef struct _pixel_rgb565_t {
  uint16_t r : 5;
  uint16_t g : 6;
  uint16_t b : 5;
} pixel_rgb565_t;

#define pixel_rgb565_BPP 2
#define pixel_rgb565_a(p) 0xff
#define pixel_rgb565_format BITMAP_FMT_RGB565
#define pixel_rgb565_to_rgba(p) \
  { p.r << 3, p.g << 2, p.b << 3, 0xff }
#define pixel_rgb565_from_rgb(r, g, b) \
  { r >> 3, g >> 2, b >> 3 }
static inline void pixel_rgb565_blend_rgba_dark(void* p, uint8_t a) {
  uint16_t v = *(uint16_t*)p;

  *(uint16_t*)p = (((((v & 0xf800) >> 8) * a) >> 11) << 11) |
                  (((((v & 0x7e0) >> 3) * a) >> 10) << 5) | ((((v & 0x1f) << 3) * a) >> 11);
}

static inline void pixel_rgb565_blend_rgba_premulti(void* p, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint16_t v = *(uint16_t*)p;

  *(uint16_t*)p = (((((v & 0xf800) >> 8) * a + (rgba.b << 8)) >> 11) << 11) |
                  (((((v & 0x7e0) >> 3) * a + (rgba.g << 8)) >> 10) << 5) |
                  ((((v & 0x1f) << 3) * a + (rgba.r << 8)) >> 11);
}

static inline uint16_t pixel_rbg565_encode(rgba_t rgba)
{
	return ((uint16_t)(rgba.r & 0xF8) << 8 | (uint16_t)(rgba.g & 0xFC) << 3 | (uint16_t)(rgba.b & 0xF8) >> 3);
}


typedef struct _pixel_bgr565_t {
  uint16_t b : 5;
  uint16_t g : 6;
  uint16_t r : 5;
} pixel_bgr565_t;

#define pixel_bgr565_BPP 2
#define pixel_bgr565_a(p) 0xff
#define pixel_bgr565_format BITMAP_FMT_BGR565
#define pixel_bgr565_to_rgba(p) \
  { p.r << 3, p.g << 2, p.b << 3, 0xff }
#define pixel_bgr565_from_rgb(r, g, b) \
  { b >> 3, g >> 2, r >> 3 }

static inline void pixel_bgr565_blend_rgba_dark(void* p, uint8_t a) {
  uint16_t v = *(uint16_t*)p;

  *(uint16_t*)p = (((((v & 0xf800) >> 8) * a) >> 11) << 11) |
                  (((((v & 0x7e0) >> 3) * a) >> 10) << 5) | ((((v & 0x1f) << 3) * a) >> 11);
}

static inline void pixel_bgr565_blend_rgba(void* p, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t minus_a = 0xff - a;
  uint16_t v = *(uint16_t*)p;

  *(uint16_t*)p = (((((v & 0xf800) >> 8) * minus_a + (rgba.r * a)) >> 11) << 11) |
                  (((((v & 0x7e0) >> 3) * minus_a + (rgba.g * a)) >> 10) << 5) |
                  ((((v & 0x1f) << 3) * minus_a + (rgba.b * a)) >> 11);
}

static inline uint16_t pixel_bgr565_encode(rgba_t rgba)
{
	return ((uint16_t)(rgba.b & 0xF8) << 8 | (uint16_t)(rgba.g & 0xFC) << 3 | (uint16_t)(rgba.r & 0xF8) >> 3);
}

static inline void pixel_bgr565_blend_rgba_premulti(void* p, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint16_t v = *(uint16_t*)p;

  *(uint16_t*)p = (((((v & 0xf800) >> 8) * a + (rgba.r << 8)) >> 11) << 11) |
                  (((((v & 0x7e0) >> 3) * a + (rgba.g << 8)) >> 10) << 5) |
                  ((((v & 0x1f) << 3) * a + (rgba.b << 8)) >> 11);
}

typedef struct _pixel_rgb888_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} pixel_rgb888_t;

#define pixel_rgb888_BPP 3
#define pixel_rgb888_a(p) 0xff
#define pixel_rgb888_format BITMAP_FMT_RGB888
#define pixel_rgb888_to_rgba(p) \
  { p.r, p.g, p.b, 0xff }
#define pixel_rgb888_from_rgb(r, g, b) \
  { r, g, b }

static inline void pixel_rgb888_blend_rgba_dark(void* pixel, uint8_t a) {
  uint8_t* p = (uint8_t*)pixel;

  p[0] = (p[0] * a) >> 8;
  p[1] = (p[1] * a) >> 8;
  p[2] = (p[2] * a) >> 8;
}

static inline void pixel_rgb888_blend_rgba_premulti(void* pixel, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t* p = (uint8_t*)pixel;

  p[0] = ((p[0] * a) >> 8) + rgba.r;
  p[1] = ((p[1] * a) >> 8) + rgba.g;
  p[2] = ((p[2] * a) >> 8) + rgba.b;
}

typedef struct _pixel_bgr888_t {
  uint8_t b;
  uint8_t g;
  uint8_t r;
} pixel_bgr888_t;

#define pixel_bgr888_BPP 3
#define pixel_bgr888_a(p) 0xff
#define pixel_bgr888_format BITMAP_FMT_BGR888
#define pixel_bgr888_to_rgba(p) \
  { p.r, p.g, p.b, 0xff }
#define pixel_bgr888_from_rgb(r, g, b) \
  { b, g, r }
static inline void pixel_bgr888_blend_rgba_dark(void* pixel, uint8_t a) {
  uint8_t* p = (uint8_t*)pixel;

  p[0] = (p[0] * a) >> 8;
  p[1] = (p[1] * a) >> 8;
  p[2] = (p[2] * a) >> 8;
}

static inline void pixel_bgr888_blend_rgba_premulti(void* pixel, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t* p = (uint8_t*)pixel;

  p[0] = ((p[0] * a) >> 8) + rgba.b;
  p[1] = ((p[1] * a) >> 8) + rgba.g;
  p[2] = ((p[2] * a) >> 8) + rgba.r;
}

typedef struct _pixel_rgba8888_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} pixel_rgba8888_t;

#define pixel_rgba8888_BPP 4
#define pixel_rgba8888_a(p) p.a
#define pixel_rgba8888_format BITMAP_FMT_RGBA8888
#define pixel_rgba8888_to_rgba(p) \
  { p.r, p.g, p.b, p.a }
#define pixel_rgba8888_from_rgba(r, g, b, a) \
  { r, g, b, a }
#define pixel_rgba8888_from_rgb(r, g, b) \
  { r, g, b, 0xff }
static inline void pixel_rgba8888_blend_rgba_dark(void* pixel, uint8_t a) {
  uint8_t* p = (uint8_t*)pixel;

  p[0] = (p[0] * a) >> 8;
  p[1] = (p[1] * a) >> 8;
  p[2] = (p[2] * a) >> 8;
}

static inline void pixel_rgba8888_blend_rgba_premulti(void* pixel, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t* p = (uint8_t*)pixel;

  p[0] = ((p[0] * a) >> 8) + rgba.r;
  p[1] = ((p[1] * a) >> 8) + rgba.g;
  p[2] = ((p[2] * a) >> 8) + rgba.b;
}

typedef struct _pixel_abgr8888_t {
  uint8_t a;
  uint8_t b;
  uint8_t g;
  uint8_t r;
} pixel_abgr8888_t;

#define pixel_abgr8888_BPP 4
#define pixel_abgr8888_a(p) p.a
#define pixel_abgr8888_format BITMAP_FMT_ABGR8888
#define pixel_abgr8888_to_rgba(p) \
  { p.r, p.g, p.b, p.a }
#define pixel_abgr8888_from_rgb(r, g, b) \
  { 0xff, b, g, r }
#define pixel_abgr8888_from_rgba(r, g, b, a) \
  { a, b, g, r }
static inline void pixel_abgr8888_blend_rgba_dark(void* pixel, uint8_t a) {
  uint8_t* p = (uint8_t*)pixel;

  p[1] = (p[1] * a) >> 8;
  p[2] = (p[2] * a) >> 8;
  p[3] = (p[3] * a) >> 8;
}

static inline void pixel_abgr8888_blend_rgba_premulti(void* pixel, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t* p = (uint8_t*)pixel;

  p[1] = ((p[1] * a) >> 8) + rgba.b;
  p[2] = ((p[2] * a) >> 8) + rgba.g;
  p[3] = ((p[3] * a) >> 8) + rgba.r;
}

typedef struct _pixel_bgra8888_t {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
} pixel_bgra8888_t;

#define pixel_bgra8888_BPP 4
#define pixel_bgra8888_a(p) p.a
#define pixel_bgra8888_format BITMAP_FMT_BGRA8888
#define pixel_bgra8888_to_rgba(p) \
  { p.r, p.g, p.b, p.a }
#define pixel_bgra8888_from_rgba(r, g, b, a) \
  { b, g, r, a }
#define pixel_bgra8888_from_rgb(r, g, b) \
  { b, g, r, 0xff }
static inline void pixel_bgra8888_blend_rgba_dark(void* pixel, uint8_t a) {
  uint8_t* p = (uint8_t*)pixel;

  p[0] = (p[0] * a) >> 8;
  p[1] = (p[1] * a) >> 8;
  p[2] = (p[2] * a) >> 8;
}

static inline void pixel_bgra8888_blend_rgba_premulti(void* pixel, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t* p = (uint8_t*)pixel;

  p[0] = ((p[0] * a) >> 8) + rgba.b;
  p[1] = ((p[1] * a) >> 8) + rgba.g;
  p[2] = ((p[2] * a) >> 8) + rgba.r;
}

typedef struct _pixel_argb8888_t {
  uint8_t a;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} pixel_argb8888_t;

#define pixel_argb8888_BPP 4
#define pixel_argb8888_a(p) p.a
#define pixel_argb8888_format BITMAP_FMT_ARGB8888
#define pixel_argb8888_to_rgba(p) \
  { p.r, p.g, p.b, p.a }
#define pixel_argb8888_from_rgba(r, g, b, a) \
  { a, r, g, b }
#define pixel_argb8888_from_rgb(r, g, b) \
  { 0xff, r, g, b }
static inline void pixel_argb8888_blend_rgba_dark(void* pixel, uint8_t a) {
  uint8_t* p = (uint8_t*)pixel;

  p[1] = (p[1] * a) >> 8;
  p[2] = (p[2] * a) >> 8;
  p[3] = (p[3] * a) >> 8;
}

static inline void pixel_argb8888_blend_rgba_premulti(void* pixel, rgba_t rgba) {
  uint8_t a = rgba.a;
  uint8_t* p = (uint8_t*)pixel;

  p[1] = ((p[1] * a) >> 8) + rgba.r;
  p[2] = ((p[2] * a) >> 8) + rgba.g;
  p[3] = ((p[3] * a) >> 8) + rgba.b;
}

static inline uint8_t pixel_rgb222_encode(rgba_t rgba)
{
	return ((uint8_t)(rgba.r & 0xC0) >> 2 | (uint8_t)(rgba.g & 0xC0) >> 4 | (uint8_t)(rgba.b & 0xC0) >> 6);
}

static inline uint8_t pixel_bgr222_encode(rgba_t rgba)
{
	return ((uint8_t)(rgba.b & 0xC0) >> 2 | (uint8_t)(rgba.g & 0xC0) >> 4 | (uint8_t)(rgba.r & 0xC0) >> 6);
}
#pragma pack(pop)


#ifdef WITH_BITMAP_BGRA
#define rgba_to_image8888(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#else
#define rgba_to_image8888(r, g, b, a) (((a) << 24) | ((b) << 16) | ((g) << 8) | (r))
#endif /*WITH_BITMAP_BGRA*/

#define rgb_to_bgr565(r, g, b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define rgb_to_image8888(r, g, b) rgba_to_image8888(r, g, b, (uint8_t)0xff)


#define color_to_pixel(c) pixel_from_rgb(c.rgba.r, c.rgba.g, c.rgba.b)

#endif /*TK_PIXEL_H*/
