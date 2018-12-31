#ifndef STORAGE_H
#define STORAGE_H
#include <FS.h>
#include <stdio.h>
class STORAGE
{

  public:
    static File fsUploadFile;
    static void initSPIFFS();
    static bool handleFileRead(String path);
    static void handleFileUpload();

  private:
    static String getContentType(String filename);
    static String formatBytes(size_t bytes);
};

#endif