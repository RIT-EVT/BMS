#include <BMS/BMS.hpp>
#include <stdint.h>

namespace BMS {

BMS::BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq) : bqSettingsStorage(bqSettingsStorage), bq(bq) {
}

CO_OBJ_T* BMS::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t BMS::getObjectDictionarySize() {
    return OBJECT_DIRECTIONARY_SIZE;
}


}  // namespace BMS
