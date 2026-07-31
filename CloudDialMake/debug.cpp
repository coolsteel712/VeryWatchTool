//
// Refactored for Cross-Platform Compatibility (Windows & POSIX)
//

#include "debug.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/time.h>
#endif

static bool m_is_write_file = false;
static bool m_is_print_consloe = true;
static bool m_debug_log_init = false;

int cur_debug_level = DEBUG_LEVEL_WARN; // 0:debug 1:debug 2:release
static int log_save_day = 2;             // Default retention: 2 days

void printf_log_consloe(char *log) {
    printf("%s\n", log);
}

void debug_log_get_time(struct tm *get_time) {
    if (!get_time) return;
    time_t now_time;
    time(&now_time);
#ifdef _WIN32
    localtime_s(get_time, &now_time);
#else
    struct tm *now = localtime(&now_time);
    if (now) {
        memcpy(get_time, now, sizeof(struct tm));
    }
#endif
    get_time->tm_year += 1900;
    get_time->tm_mon += 1;
}

static long debug_log_get_mil(void) {
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    return (long)st.wMilliseconds;
#else
    struct timeval val = {0};
    gettimeofday(&val, NULL);
    return (val.tv_usec / 1000);
#endif
}

static char log_buff[4096];

void debug_log(DEBUG_LEVEL level, const char *tag, const char *func_name, int line, const char *fmt, ...) {
    bool write_close = false;
    if (!m_debug_log_init) {
        return;
    }

    if (cur_debug_level > level) {
        write_close = true;
    }

    struct tm now_time;
    debug_log_get_time(&now_time);

    memset(log_buff, 0, sizeof(log_buff));

    if (tag != NULL) {
        const char *get_last_p = strrchr(tag, '/');
        if (!get_last_p) {
            get_last_p = strrchr(tag, '\\'); // Account for Windows path backslashes
        }
        if (get_last_p != NULL) {
            get_last_p += 1;
        } else {
            get_last_p = tag;
        }

        snprintf(log_buff, sizeof(log_buff), "[%02d-%02d %d:%02d:%02d.%03ld] [%s] [%s] [line %d] ",
                 now_time.tm_mon, now_time.tm_mday, now_time.tm_hour, now_time.tm_min, 
                 now_time.tm_sec, debug_log_get_mil(), get_last_p, func_name, line);
    } else {
        snprintf(log_buff, sizeof(log_buff), "[%02d-%02d %d:%02d:%02d.%03ld] [%s] [line %d] ",
                 now_time.tm_mon, now_time.tm_mday, now_time.tm_hour, now_time.tm_min, 
                 now_time.tm_sec, debug_log_get_mil(), func_name ? func_name : "", line);
    }

    va_list arg;
    va_start(arg, fmt);
    size_t current_len = strlen(log_buff);
    if (current_len < sizeof(log_buff) - 1) {
        vsnprintf(log_buff + current_len, sizeof(log_buff) - current_len - 1, fmt, arg);
    }
    va_end(arg);

    if (m_is_print_consloe) {
        printf_log_consloe(log_buff);
    }

    if (m_is_write_file && !write_close) {
        // File output handling if needed
    }
}

void debug_log_get_old_one_day_time(struct tm *get_time) {
    if (!get_time) return;
    time_t now_time;
    time(&now_time);
    now_time -= 24 * 60 * 60;

#ifdef _WIN32
    localtime_s(get_time, &now_time);
#else
    struct tm *now = localtime(&now_time);
    if (now) {
        memcpy(get_time, now, sizeof(struct tm));
    }
#endif
    get_time->tm_year += 1900;
    get_time->tm_mon += 1;
}

static bool get_file_time_2_delete(time_t start_time, char *name) {
    char date[20] = {0};
    int year = 0, month = 0, day = 0;

    if (name == NULL) {
        return true;
    }

    char *suffix = strrchr(name, '.');
    if (!suffix) {
        return true;
    }

    size_t name_len = strlen(name);
    size_t suffix_len = strlen(suffix);

    // Safeguard against underflow and buffer overflows
    if (suffix_len >= name_len) {
        return true;
    }

    size_t prefix_len = name_len - suffix_len;
    if (prefix_len >= sizeof(date)) {
        prefix_len = sizeof(date) - 1;
    }

    memcpy(date, name, prefix_len);
    date[prefix_len] = '\0';

    DEBUG_INFO("file date:%s", date);

    int result = sscanf(date, "%d-%d-%d", &year, &month, &day);
    if (result < 3) {
        return true;
    }

    struct tm timetmp;
    memset(&timetmp, 0, sizeof(struct tm));
    debug_log_get_time(&timetmp);

    // Re-adjust struct tm fields to standard C zero-indexed offsets
    timetmp.tm_year -= 1900;
    timetmp.tm_mon -= 1;
    time_t end_time = mktime(&timetmp);

    memset(&timetmp, 0, sizeof(struct tm));
    timetmp.tm_year = year - 1900; // Correct offset for mktime
    timetmp.tm_mon = month - 1;    // Correct 0-indexed month
    timetmp.tm_mday = day;
    timetmp.tm_hour = 0;
    timetmp.tm_min = 0;
    timetmp.tm_sec = 0;
    timetmp.tm_isdst = -1;

    time_t cur_time = mktime(&timetmp);

    if (cur_time != (time_t)-1 && cur_time >= start_time && cur_time <= end_time) {
        return true;
    }

    return false;
}

static void clear_old_log(char *filepath, int save_day) {
    if (!filepath || strlen(filepath) == 0) return;

    struct tm ntime;
    memset(&ntime, 0, sizeof(struct tm));
    debug_log_get_time(&ntime);

    ntime.tm_year -= 1900;
    ntime.tm_mon -= 1;
    ntime.tm_hour = 0;
    ntime.tm_min = 0;
    ntime.tm_sec = 0;
    ntime.tm_isdst = -1;

    time_t today_time = mktime(&ntime);
    time_t save_end_time = today_time - ((save_day - 1) * 24 * 60 * 60);

#ifdef _WIN32
    // Windows API Directory Search Logic
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*.*", filepath);

    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        LOG_INFO("err: FindFirstFileA failed");
        return;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        if (get_file_time_2_delete(save_end_time, find_data.cFileName)) {
            DEBUG_INFO("save %s", find_data.cFileName);
        } else {
            char pathname[MAX_PATH];
            snprintf(pathname, sizeof(pathname), "%s\\%s", filepath, find_data.cFileName);
            int ret = remove(pathname);
            if (ret != 0) {
                LOG_INFO("删除 %s 失败", find_data.cFileName);
            } else {
                LOG_INFO("删除 %s 成功", find_data.cFileName);
            }
        }
    } while (FindNextFileA(hFind, &find_data) != 0);

    FindClose(hFind);

#else
    // POSIX Directory Search Logic (Linux/macOS)
    DIR *dir = opendir(filepath);
    if (dir == NULL) {
        LOG_INFO("err:opendir error");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if ((strncmp(entry->d_name, ".", 1) == 0) || (strncmp(entry->d_name, "..", 2) == 0) ||
            get_file_time_2_delete(save_end_time, entry->d_name)) {
            DEBUG_INFO("save %s", entry->d_name);
        } else {
            char pathname[strlen(filepath) + strlen(entry->d_name) + 2];
            snprintf(pathname, sizeof(pathname), "%s/%s", filepath, entry->d_name);
            int ret = remove(pathname);
            if (ret != 0) {
                LOG_INFO("删除%s失败", entry->d_name);
            } else {
                LOG_INFO("删除%s成功", entry->d_name);
            }
        }
    }
    closedir(dir);
#endif
}

void debug_log_init(bool is_print_console, bool is_write_file, const char *file_path) {
    if (!file_path) return;

    struct tm now_time;
    memset(&now_time, 0, sizeof(struct tm));
    debug_log_get_time(&now_time);

    m_is_write_file = is_write_file;
    m_is_print_consloe = is_print_console;
    m_debug_log_init = true;

    char path[MAX_PATH] = {0};
    strncpy(path, file_path, sizeof(path) - 1);
    clear_old_log(path, log_save_day);
}

void set_log_save_day(int save_day) {
    log_save_day = save_day < 2 ? 2 : save_day;
}