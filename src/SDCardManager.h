#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

#include <Arduino.h>
#include <vector>

class SDCardManager
{
public:
  SDCardManager();

  // Initialize SD card
  bool begin();

  // Check if SD card is ready
  bool isReady() const { return sdReady; }

  // Get list of top N files from SD card root directory
  std::vector<String> getTopFiles(int maxFiles);

private:
  bool sdReady;
};

#endif // SD_CARD_MANAGER_H
