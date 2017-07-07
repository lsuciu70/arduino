#include <FS.h>

void setup()
{
  Serial.begin(115200);
  Serial.println();

  if(!SPIFFS.begin())
  {
    Serial.println("File system was NOT mounted successfully");
    return;
  }
  Serial.println("File system was mounted successfully");
  Serial.println("Read info ...");
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  Serial.println("Read info done.");

  Serial.print("totalBytes:    ");Serial.println(fs_info.totalBytes);
  Serial.print("usedBytes:     ");Serial.println(fs_info.usedBytes);
  Serial.print("blockSize:     ");Serial.println(fs_info.blockSize);
  Serial.print("pageSize:      ");Serial.println(fs_info.pageSize);
  Serial.print("maxOpenFiles:  ");Serial.println(fs_info.maxOpenFiles);
  Serial.print("maxPathLength: ");Serial.println(fs_info.maxPathLength);
  if(SPIFFS.exists("/"))
  {
    Serial.println("/ exists");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      Serial.println(dir.fileName());
//    File f = dir.openFile("r");
//    Serial.println(f.size());
    }
  }
  else
  {
    Serial.println("/ does not exist");
  }
}

void loop()
{
}
