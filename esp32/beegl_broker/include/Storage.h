#ifndef Storage_h
#define Storage_h
#include <SPIFFS.h>
#ifdef FILESYSTEM_SD
#define FILESYSTEM SD
#include <SD.h>
#else
#define FILESYSTEM SPIFFS
#endif

bool storage_setup();

#endif