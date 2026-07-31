/*
 * app_error.h
 *
 *  Created on: 2016年1月9日
 *      Author: Administrator
 */

//错误信息代码

#ifndef ERROR_H_
#define ERROR_H_

#define SUCCESS					0
#define ERROR_NO_MEM			4
#define ERROR_NOT_FIND			5
#define ERROR_NOT_SUPPORTED		6
#define ERROR_INVALID_PARAM		7
#define ERROR_INVALID_STATE		8
#define ERROR_INVALID_LENGTH 	9
#define ERROR_INVALID_FLAGS 	10
#define ERROR_INVALID_DATA		11
#define ERROR_DATA_SIZE			12
#define ERROR_TIMEOUT			13
#define ERROR_NULL				14
#define ERROR_FORBIDDEN			15
#define ERROR_BUSY				17
#define ERROR_LOW_BATT          18
#define ERROR_SERIAL            19
#define ERROR_TIME_OUT_RECONNECT 20
#define ERROR_APP_TEST  21
#define ERROR_MAX_TIME  22
#define ERROR_SPACE_ORGANIZATION      (24)    //空间够但需要整理
#define ERROR_SPACE_ORGANIZATION_ING  (25)    //空间整理中

#define TASK_TERMINATION (26) //任务终止
#define ERROR_INVAILD_FILE (27) //非法文件，例EPO文件过期
#define ERROR_CODE_REPEAT_REQUEST (31) //用来表示这个前一个文件传输未结束又发起了一个新文件传输的异常错误，然后这个错误码C库端拦截过滤掉，就是碰到这个错误不管，日志可以方便以后分析问题
/*
 * bool v3_support_data_tran_get_new_error_code;//支持文件传输获取新的通用错误码
 * 功能表支持的时候，使用下面的错误码，与固件使用同一套错误码
 * #define ERROR_CODE_SUCCESS                 (ERROR_CODE_BASE_NUM + 0) ///< Successful command
    #define ERROR_CODE_SVC_HANDLER_MISSING    (ERROR_CODE_BASE_NUM + 1) ///< SVC handler is missing
    #define ERROR_CODE_SOFTDEVICE_NOT_ENABLED (ERROR_CODE_BASE_NUM + 2) ///< SoftDevice has not been enabled
    #define ERROR_CODE_INTERNAL               (ERROR_CODE_BASE_NUM + 3) ///< Internal Error
    #define ERROR_CODE_NO_MEM                 (ERROR_CODE_BASE_NUM + 4) ///< No Memory for operation
    #define ERROR_CODE_NOT_FOUND              (ERROR_CODE_BASE_NUM + 5) ///< Not found
    #define ERROR_CODE_NOT_SUPPORTED          (ERROR_CODE_BASE_NUM + 6) ///< Not supported
    #define ERROR_CODE_INVALID_PARAM          (ERROR_CODE_BASE_NUM + 7) ///< Invalid Parameter
    #define ERROR_CODE_INVALID_STATE          (ERROR_CODE_BASE_NUM + 8) ///< Invalid state, operation disallowed in this state
    #define ERROR_CODE_INVALID_LENGTH         (ERROR_CODE_BASE_NUM + 9) ///< Invalid Length
    #define ERROR_CODE_INVALID_FLAGS          (ERROR_CODE_BASE_NUM + 10) ///< Invalid Flags
    #define ERROR_CODE_INVALID_DATA           (ERROR_CODE_BASE_NUM + 11) ///< Invalid Data
    #define ERROR_CODE_DATA_SIZE          	  (ERROR_CODE_BASE_NUM + 12) ///< Invalid Data size
    #define ERROR_CODE_TIMEOUT          	  (ERROR_CODE_BASE_NUM + 13) ///< Operation timed out
    #define ERROR_CODE_NULL           		  (ERROR_CODE_BASE_NUM + 14) ///< Null Pointer
    #define ERROR_CODE_FORBIDDEN         	  (ERROR_CODE_BASE_NUM + 15) ///< Forbidden Operation
    #define ERROR_CODE_INVALID_ADDR       	  (ERROR_CODE_BASE_NUM + 16) ///< Bad Memory Address
    #define ERROR_CODE_BUSY           		  (ERROR_CODE_BASE_NUM + 17) ///< Busy
    #define ERROR_CODE_CONN_COUNT         	  (ERROR_CODE_BASE_NUM + 18) ///< Maximum connection count exceeded.
    #define ERROR_CODE_RESOURCES          	  (ERROR_CODE_BASE_NUM + 19) ///< Not enough resources for operation
    #define ERROR_CODE_BT_OTA          	      (ERROR_CODE_BASE_NUM + 20) ///< Bt Bluetooth upgrade error
    #define ERROR_CODE_NO_SPACE          	  (ERROR_CODE_BASE_NUM + 21) ///< Not enough space for operation
    #define ERROR_CODE_LOW_BATTERY            (ERROR_CODE_BASE_NUM + 22) ///< Low Battery
    #define ERROR_CODE_INVALID_FILE_NAME      (ERROR_CODE_BASE_NUM + 23) ///< Invalid File Name/Format
    #define ERROR_SPACE_ORGANIZATION          (ERROR_CODE_BASE_NUM + 24) //空间够但需要整理
    #define ERROR_SPACE_ORGANIZATION_ING      (ERROR_CODE_BASE_NUM + 25) //空间整理中
 * */



#endif /* ERROR_H_ */
