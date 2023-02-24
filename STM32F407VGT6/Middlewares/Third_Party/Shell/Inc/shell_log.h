/**
  ******************************************************************************
  * @file    debug_log.h
  * @author  Iron
  * @date    2021-01-31
  * @version v1.0
  * @brief   debug log header file
  */

#ifndef __DEBUG_LOG_H
#define __DEBUG_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes ------------------------------------------------------------------*/
#include <stdint.h>

#if !defined( DEBUG_LOG_ENABLE ) && !defined( NDEBUG )
    #define DEBUG_LOG_ENABLE     1
#endif

#ifdef NDEBUG
    #define debug_assert(condition) ((void)0)
#else
    #define debug_assert(condition) ((condition) ? (void)0U : debug_assert_failed(__FILE__, __LINE__))
#endif

#if DEBUG_LOG_ENABLE

/* exported types ------------------------------------------------------------*/
typedef enum
{
    DBG_LOG_NONE,       /*!< No log output */
    DBG_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    DBG_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    DBG_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    DBG_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    DBG_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} dbg_log_level_t;

/* exported constants --------------------------------------------------------*/
/* exported macro ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef CONFIG_LOG_COLORS
    #define CONFIG_LOG_COLORS     1
#endif

#define _LINE_END_  "\r\n"
//#define _LINE_END_  "\r"

#if CONFIG_LOG_COLORS
    #define LOG_COLOR_BLACK   "30"
    #define LOG_COLOR_RED     "31"
    #define LOG_COLOR_GREEN   "32"
    #define LOG_COLOR_BROWN   "33"
    #define LOG_COLOR_BLUE    "34"
    #define LOG_COLOR_PURPLE  "35"
    #define LOG_COLOR_CYAN    "36"
    #define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
    #define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
    #define LOG_RESET_COLOR   "\033[0m"
    #define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
    #define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
    #define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
    #define LOG_COLOR_D
    #define LOG_COLOR_V       LOG_COLOR(LOG_COLOR_CYAN)
#else //CONFIG_LOG_COLORS
    #define LOG_COLOR_E
    #define LOG_COLOR_W
    #define LOG_COLOR_I
    #define LOG_COLOR_D
    #define LOG_COLOR_V
    #define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " [%4u] %s: " format LOG_RESET_COLOR _LINE_END_

#define DBG_LOG_LEVEL(level, tag, format, ...) \
    do {                                                                                                                                                                                                                                                                                                \
            if      (level == DBG_LOG_ERROR )     { debug_log(DBG_LOG_ERROR,      tag, LOG_FORMAT(E, format), debug_log_timestamp(), tag, ##__VA_ARGS__); } \
            else if (level == DBG_LOG_WARN )      { debug_log(DBG_LOG_WARN,       tag, LOG_FORMAT(W, format), debug_log_timestamp(), tag, ##__VA_ARGS__); } \
            else if (level == DBG_LOG_DEBUG )     { debug_log(DBG_LOG_DEBUG,      tag, LOG_FORMAT(D, format), debug_log_timestamp(), tag, ##__VA_ARGS__); } \
            else if (level == DBG_LOG_VERBOSE )   { debug_log(DBG_LOG_VERBOSE,    tag, LOG_FORMAT(V, format), debug_log_timestamp(), tag, ##__VA_ARGS__); } \
            else                                  { debug_log(DBG_LOG_INFO,       tag, LOG_FORMAT(I, format), debug_log_timestamp(), tag, ##__VA_ARGS__); } \
    } while(0)

#define DBG_LOG_LEVEL_LOCAL(level, tag, format, ...) \
    do {                                                                                                                                                                    \
        if ( LOG_LOCAL_LEVEL >= level ) DBG_LOG_LEVEL(level, tag, format, ##__VA_ARGS__);   \
    } while(0)

#define DBG_LOGE( tag, format, ... ) DBG_LOG_LEVEL_LOCAL(DBG_LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define DBG_LOGW( tag, format, ... ) DBG_LOG_LEVEL_LOCAL(DBG_LOG_WARN,    tag, format, ##__VA_ARGS__)
#define DBG_LOGI( tag, format, ... ) DBG_LOG_LEVEL_LOCAL(DBG_LOG_INFO,    tag, format, ##__VA_ARGS__)
#define DBG_LOGD( tag, format, ... ) DBG_LOG_LEVEL_LOCAL(DBG_LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define DBG_LOGV( tag, format, ... ) DBG_LOG_LEVEL_LOCAL(DBG_LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#define DBG_PUTS( str ) debug_log_puts(str)

#define DBG_LOG_TAG(tag) static const char * TAG = tag

/* exported functions ------------------------------------------------------- */
    uint32_t debug_log_timestamp(void);
    int debug_log_puts(const char *str);
    void debug_log(dbg_log_level_t level, const char *tag, const char *format, ...);
    void debug_print_hex(const uint8_t *data, int32_t len);
    void debug_assert_failed(const char *file, uint32_t line);

#else
    #define DBG_LOGE( tag, format, ... )
    #define DBG_LOGW( tag, format, ... )
    #define DBG_LOGI( tag, format, ... )
    #define DBG_LOGD( tag, format, ... )
    #define DBG_LOGV( tag, format, ... )
    
    #define DBG_PUTS( str )
    
    #define DBG_LOG_TAG(tag)

#endif // DEBUG_LOG_ENABLE
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_LOG_H */

/******************* (C)COPYRIGHT 2018 ***** END OF FILE *********************/
