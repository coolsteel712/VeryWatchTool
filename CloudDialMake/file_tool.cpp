#include "file_tool.h"
#include "debug.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace std;

void fileTool::getAllFiles(string pathString, vector<string> &files) {
#ifdef _WIN32
    string searchPath = pathString;
    if (searchPath.empty()) return;

    if (searchPath.back() != '\\' && searchPath.back() != '/') {
        searchPath += "\\*";
    } else {
        searchPath += "*";
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }
        // Exclude directory attribute to collect only files
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(findData.cFileName);
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
#else
    DIR *dir = opendir(pathString.c_str());
    if (dir == NULL) return;

    struct dirent *ptr;
    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }

        string fullPath = pathString + "/" + ptr->d_name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISREG(st.st_mode)) { // Check if regular file via stat
                files.push_back(ptr->d_name);
            }
        }
    }
    closedir(dir);
#endif
}

void fileTool::getAllFileAndDir(string pathString, vector<string> &files) {
#ifdef _WIN32
    string searchPath = pathString;
    if (searchPath.empty()) return;

    if (searchPath.back() != '\\' && searchPath.back() != '/') {
        searchPath += "\\*";
    } else {
        searchPath += "*";
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }
        files.push_back(findData.cFileName);
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
#else
    DIR *dir = opendir(pathString.c_str());
    if (dir == NULL) return;

    struct dirent *ptr;
    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        files.push_back(ptr->d_name);
    }
    closedir(dir);
#endif
}

void fileTool::wirteBufftoFile(std::string fileName, uint8_t *buff, uint32_t buff_size) {
    if (buff == NULL || buff_size == 0) {
        return;
    }
    fstream outFile;
    outFile.open(fileName, ios::out | ios::trunc | ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char *>(buff), buff_size);
        outFile.close();
    }
}

bool fileTool::separateFileName(const char *pcFile, char *pcFileName, char *pcFileExtension) {
    if (!pcFile || !pcFileName || !pcFileExtension) {
        return false;
    }

    char cFile[200] = {'\0'};
    strncpy(cFile, pcFile, sizeof(cFile) - 1);

    char *pcTemp = strrchr(cFile, '.');
    if (NULL == pcTemp) {
        return false;
    }

    strcpy(pcFileExtension, pcTemp + 1);
    *pcTemp = '\0';
    strcpy(pcFileName, cFile);

    return true;
}