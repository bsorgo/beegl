#include <Storage.h>
namespace beegl
{
bool storage_setup()
{
    Serial.printf("[%s] Begin SPIFFS\n",TAG_STORAGE);
    if (!SPIFFS.begin(true))
    {
        return false;
        Serial.printf("[%s] An Error has occurred while mounting SPIFFS\n",TAG_STORAGE);
    }
#ifdef FILESYSTEM_SD
    Serial.printf("[%s]Begin SD\n",TAG_STORAGE);
    if (!SD.begin(4))
    {
        Serial.printf("[%s] An Error has occurred while mounting SD\n",TAG_STORAGE);
        return false;
    }
#endif
    return true;
}
}