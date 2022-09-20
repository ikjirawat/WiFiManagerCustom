#include <Arduino.h>
#include "./ManageFileSystem.h"

void ManageFileSystem::begin(boolean format_if_failed)
{
    if(!SPIFFS.begin(format_if_failed))
    {
        Serial.println("FileSystem Mount Failed");
        return;
    }
}
ListFileString ManageFileSystem::listDirectory(const char* dir_name, uint8_t level, fs::FS &fs)
{
    ListFileString list;
    Serial.printf("Listing directory: %s\r\n", dir_name);

    File root = fs.open(dir_name);
    if(!root){
        Serial.println("- fail to open");
        return list;
    }
    if(!root.isDirectory()){
        Serial.println("- not a directory");
        return list;
    }
    // else{
    //     Serial.print('\t');
    //     Serial.println(root.path());
    // }
    File file = root.openNextFile();
    if(file.isDirectory())
    {
        Serial.print('\t');
        Serial.println(file.path());
    }
    // while (file)
    // {
       
    //    if(file.isDirectory())
    //    {
    //     Serial.print("Dir: ");
    //     Serial.println(file.name());
    //     list.push_back(file.name());
    //     if(level)
    //     {
    //         listDirectory(file.name(), level -1, fs);
    //     }
    //    }else{
    //     Serial.print("File: ");
    //     Serial.print(file.name());
    //     Serial.print("\tSize: ");
    //     Serial.println(file.size());
    //    }
    //    file = root.openNextFile();
    // }
    while (true) 
    {
        File file =  root.openNextFile();
        if (!file) {
        Serial.println("=============================");
          break;
        }
        for (uint8_t i = 0; i < level; i++) {
          Serial.print('\t');
        }
        
        if (file.isDirectory()) {
          Serial.println("/");
          Serial.print(file.name());
          listDirectory(dir_name, level + 1);
        } else {
          // display zise for file, nothing for directory
          Serial.print(file.path());
          Serial.print("\t");
          list.push_back(file.path());
          Serial.println(file.size(), DEC);
        }
    file.close();
  }
  return list;
}

// String ManageFileSystem::readFile(const char* path, fs::FS &fs)
// {
//     Serial.printf("Reading file: %s\r\n", path);
//     char *buffer;
//     char *bb;
//     File file = fs.open(path);
//     if(!file || file.isDirectory())
//     {
//         Serial.println(F("- faild to open file for reading"));
//         return "null";
//     }
    
//     Serial.println(F("- read from file:"));
//     while(file.available())
//     {
//       buffer = new char[file.size()+2];
//       int l = file.readBytesUntil('\n', buffer, file.size()+2);
//       buffer[l] = 0;
//       strcpy(bb, buffer);
//     }
//     file.close();
//     return String(*bb);
// }

String ManageFileSystem::readBigFile(const char* path, fs::FS &fs)
{
  Serial.printf("Reading file: %s\r\n", path);
  String _read_buffer;
  File file = fs.open(path);
  if(!file || file.isDirectory())
  {
      Serial.println(F("- faild to open file for reading"));
      return "null";
  }
  
  Serial.println("- read from file size:"+String(file.size()));
  while(file.available())
  {
    char buff[65000];
    int l = file.readBytesUntil('\n', buff, sizeof(buff));
    buff[l] = 0;
    _read_buffer.concat(buff);

  }
  return _read_buffer;
}

ListFileString ManageFileSystem::readLargeFile(const char* path, fs::FS &fs)
{
  ListFileString list;
  Serial.printf("Reading file: %s\r\n", path);
  char* buffer;
  String _read_buffer;
  File file = fs.open(path);
  if(!file || file.isDirectory())
  {
      Serial.println(F("- faild to open file for reading"));
      return list;
  }
  
  Serial.println("- read from file size:"+String(file.size()));
  while(file.available())
  {
    list.push_back(file.readStringUntil('\n').c_str());
  }
  file.close();
  return list;
}

String ManageFileSystem::readFile(const char* path, fs::FS &fs)
{
    Serial.printf("Reading file: %s\r\n", path);
    char *buffer;
    File file = fs.open(path);
    if(!file || file.isDirectory())
    {
      Serial.println(F("- faild to open file for reading"));
      return "null";
    }
    Serial.println(F("- read from file:"));
    while(file.available())
    {
      buffer = new char[file.size() +2];
      strcpy(buffer, file.readString().c_str());
    }
    file.close();
    return String(buffer);
}

boolean ManageFileSystem::writeFile(const char* path, const char * message, fs::FS &fs)
{
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
      Serial.println("− failed to open file for writing");
      return false;
    }
    if(file.print(message)){
      Serial.println("− file written");
      file.close();
      return true;
    }else {
      Serial.println("− frite failed");
      return false;
    }
}

boolean ManageFileSystem::appendFile(const char* path, const char * message, fs::FS &fs)
{
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
      Serial.println("− failed to open file for appending");
      return false;
    }
     if(file.print(message)){
        Serial.println("− message appended");
        file.close();
        return true;
     } else {
        Serial.println("− append failed");
        return false;
     }
}

boolean ManageFileSystem::renameFile(const char* old_path, const char * new_path, fs::FS &fs)
{
   Serial.printf("Renaming file %s to %s\r\n", old_path, new_path);
   if (fs.rename(old_path, new_path)) {
      Serial.println("− file renamed");
      return true;
   } else {
      Serial.println("− rename failed");
      return false;
   }
}

boolean ManageFileSystem::deleteFile(const char* path, fs::FS &fs)
{
   Serial.printf("Deleting file: %s\r\n", path);
   if(fs.remove(path)){
      Serial.println("− file deleted");
      return true;
   } else {
      Serial.println("− delete failed");
      return false;
   }
}

