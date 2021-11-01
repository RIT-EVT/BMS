#include <BMS/BMS.hpp>
#include <BMS/BMSLogger.hpp>

namespace BMS {

CO_OBJ_T* BMS::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t BMS::getObjectDictionarySize() {
    return OBJECT_DIRECTIONARY_SIZE;
}

}  // namespace BMS
