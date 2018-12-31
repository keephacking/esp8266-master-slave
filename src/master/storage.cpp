#include "storage.h"
#include "common.h"
#include <FS.h>

void STORAGE::initSPIFFS()
{
    SPIFFS.begin(); // Start the SPI Flash File System (SPIFFS)
    Serial.println("SPIFFS started. Contents:");
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next())
        { // List the file system contents
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(),formatBytes(fileSize).c_str());
        }
        Serial.printf("\n");
    }
}
String STORAGE::getContentType(String filename)
{
    if (server.hasArg("download"))
        return "application/octet-stream";
    else if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}
bool STORAGE::handleFileRead(String path)
{
    DBG_OUTPUT_PORT.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {
        if (SPIFFS.exists(pathWithGz))
            path += ".gz";
        File file = SPIFFS.open(path, "r");

        //if ((contentType != "text/html") || (contentType != "text/htm")) {
        //  bool isPublic = true;
        //  String cache = String(isPublic ? "public" : "private") +", max-age=" + String(86400) + ", must-revalidate";
        //  server.sendHeader("Cache-Control", cache);
        //}

        size_t sent = server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}
String STORAGE::formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}
void STORAGE::handleFileUpload()
{ // upload a new file to the SPIFFS
    HTTPUpload &upload = server.upload();
    String path;
    if (upload.status == UPLOAD_FILE_START)
    {
        path = upload.filename;
        if (!path.startsWith("/"))
            path = "/" + path;
        if (!path.endsWith(".gz"))
        {                                     // The file server always prefers a compressed version of a file
            String pathWithGz = path + ".gz"; // So if an uploaded file is not compressed, the existing compressed
            if (SPIFFS.exists(pathWithGz))    // version of that file must be deleted (if it exists)
                SPIFFS.remove(pathWithGz);
        }
        Serial.print("handleFileUpload Name: ");
        Serial.println(path);
        fsUploadFile = SPIFFS.open(path, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
        path = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {                         // If the file was successfully created
            fsUploadFile.close(); // Close the file again
            Serial.print("handleFileUpload Size: ");
            Serial.println(upload.totalSize);
            server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
            server.send(303);
        }
        else
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}