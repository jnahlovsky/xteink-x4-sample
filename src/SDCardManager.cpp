#include "SDCardManager.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>

SDCardManager::SDCardManager() : sdReady(false)
{
}

bool SDCardManager::begin()
{
  if (!SD.begin(SD_SPI_CS, SPI, SPI_FQ))
  {
    sdReady = false;
    return false;
  }

  sdReady = true;
  return true;
}

std::vector<String> SDCardManager::getTopFiles(int maxFiles)
{
  std::vector<String> fileList;

  // Ensure SD is initialized; try to init if needed
  if (!sdReady)
  {
    if (!begin())
    {
      return fileList; // Return empty list
    }
  }

  File root = SD.open("/");
  if (!root || !root.isDirectory())
  {
    if (root)
      root.close();
    return fileList; // Return empty list
  }

  int count = 0;
  for (File f = root.openNextFile(); f && count < maxFiles; f = root.openNextFile())
  {
    if (!f.isDirectory())
    {
      const char *name = f.name();
      // Ensure only name + extension, no leading path
      const char *basename = name;
      if (basename)
      {
        const char *slash = strrchr(basename, '/');
        if (slash && *(slash + 1))
          basename = slash + 1;
      }

      if (basename && *basename)
      {
        fileList.push_back(String(basename));
        count++;
      }
    }
    f.close();
  }

  root.close();
  return fileList;
}
