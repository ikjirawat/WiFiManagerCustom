#ifndef MANAGEFILESYSTEM_H
#define MANAGEFILESYSTEM_H
#include <Arduino.h>
#include "SPIFFS.h"
#include "FS.h"
#include <vector>

typedef std::vector<String> ListFileString;

class ManageFileSystem
{
private:

public:
    void begin(boolean format_if_failed = true);
    ListFileString listDirectory(const char* dir_name, uint8_t level, fs::FS &fs = SPIFFS);
    String readFile(const char* path, fs::FS &fs = SPIFFS); //limit filesize can read 60081 can't 63207
    String readBigFile(const char* path, fs::FS &fs = SPIFFS);
    ListFileString readLargeFile(const char* path, fs::FS &fs = SPIFFS);
    boolean writeFile(const char* path, const char * message, fs::FS &fs = SPIFFS);
    boolean appendFile(const char* path, const char * message, fs::FS &fs = SPIFFS);
    boolean renameFile(const char* old_path, const char * new_path, fs::FS &fs = SPIFFS);
    boolean deleteFile(const char* path, fs::FS &fs = SPIFFS);
};

#endif