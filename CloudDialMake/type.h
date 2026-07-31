#ifndef _TYPE_H_
#define _TYPE_H_


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef enum _ret_t {
	/**
	 * @const RET_OK
	 * 成功。
	 */
	RET_OK = 0,
	/**
	 * @const RET_OOM
	 * Out of memory。
	 */
	RET_OOM,
	/**
	 * @const RET_FAIL
	 * 失败。
	 */
	RET_FAIL,
	/**
	 * @const RET_NOT_IMPL
	 * 没有实现/不支持。
	 */
	RET_NOT_IMPL,
	/**
	 * @const RET_QUIT
	 * 退出。通常用于主循环。
	 */
	RET_QUIT,
	/**
	 * @const RET_FOUND
	 * 找到。
	 */
	RET_FOUND,
	/**
	 * @const RET_BUSY
	 * 对象忙。
	 */
	RET_BUSY,
	/**
	 * @const RET_REMOVE
	 * 移出。通常用于定时器。
	 */
	RET_REMOVE,
	/**
	 * @const RET_REPEAT
	 * 重复。通常用于定时器。
	 */
	RET_REPEAT,
	/**
	 * @const RET_NOT_FOUND
	 * 没找到。
	 */
	RET_NOT_FOUND,
	/**
	 * @const RET_DONE
	 * 操作完成。
	 */
	RET_DONE,
	/**
	 * @const RET_STOP
	 * 停止后续操作。
	 */
	RET_STOP,
	/**
	 * @const RET_SKIP
	 * 跳过当前项。
	 */
	RET_SKIP,
	/**
	 * @const RET_CONTINUE
	 * 继续后续操作。
	 */
	RET_CONTINUE,
	/**
	 * @const RET_OBJECT_CHANGED
	 * 对象属性变化。
	 */
	RET_OBJECT_CHANGED,
	/**
	 * @const RET_ITEMS_CHANGED
	 * 集合数目变化。
	 */
	RET_ITEMS_CHANGED,
	/**
	 * @const RET_BAD_PARAMS
	 * 无效参数。
	 */
	RET_BAD_PARAMS
} ret_t;

#define TKMEM_ALLOC(size) malloc(size)
#define TKMEM_ZALLOC(type) calloc(1,sizeof(type))
#define TKMEM_FREE(buff) free(buff);


#define tk_min(a, b) ((a) < (b) ? (a) : (b))
#define tk_abs(a) ((a) < (0) ? (-(a)) : (a))
#define tk_max(a, b) ((a) > (b) ? (a) : (b))


#define TK_ROUND_TO(size, round_size) ((((size) + round_size - 1) / round_size) * round_size)

#define return_value_if_fail(p, value)                  \
  if (!(p)) {                                           \
    printf("%s:%d " #p "\n", __FUNCTION__, __LINE__); \
    return (value);                                     \
  }

#endif
