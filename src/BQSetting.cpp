#include <BMS/BQSetting.hpp>

namespace BMS {

BQSetting::fromArray(uint8_t buffer[ARRAY_SIZE]) {
    address = buffer
}

}  // namespace BMS
