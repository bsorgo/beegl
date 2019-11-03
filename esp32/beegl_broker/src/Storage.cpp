#include <Storage.h>
namespace beegl
{
bool storage_setup()
{
    Serial.println("[SPIFFS] Begin SPIFFS");
    if (!SPIFFS.begin(true))
    {
        return false;
        Serial.println("[SPIFFS] An Error has occurred while mounting SPIFFS");
    }
#ifdef FILESYSTEM_SD
    Serial.println("[SD] Begin SD");
    if (!SD.begin(4))
    {
        Serial.println("[SD] An Error has occurred while mounting SD");
        return false;
    }
#endif
    return true;
}
}