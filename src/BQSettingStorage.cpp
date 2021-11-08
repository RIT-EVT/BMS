#include <BMS/BQSettingStorage.hpp>
#include <BMS/BMSLogger.hpp>

///////////////////////////////////////////////////////////////////////////////
// Functions for interacting with the BQSettingsStorage through CANopen
///////////////////////////////////////////////////////////////////////////////
/**
 * Function to get the size of the BQStorage. The size is in bytes to store
 * all of the BQSettings.
 *
 * @param obj[in] CANopen stack object dictionary element (not used)
 * @param node[in] CANopen stack node (not used)
 * @param width[in] The width of the data in bytes as it is stored in the
 *  object dictionary (not used)
 * @param priv[in] Private data, pointer to an instance of the BQSettingsStorage
 * @return Number of bytes to store all BQSettings
 */
static uint32_t COBQSettingSize(struct CO_OBJ_T* obj, struct CO_NODE_T* node,
        uint32_t width, void* priv) {
    BMS::BQSettingsStorage* storage = (BMS::BQSettingsStorage*)priv;

    (void)obj;
    (void)node;
    (void)width;

    if(!storage) {
        BMS::LOGGER.log(BMS::BMSLogger::LogLevel::ERROR, "Storage not provided");
        return 0;
    }

    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::DEBUG, "Width value %u", width);

    uint32_t numSettings = storage->getNumSettings();
    uint32_t sizeOfEachSetting = BMS::BQSetting::ARRAY_SIZE;
    uint32_t totalBytes = numSettings * sizeOfEachSetting;

    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::DEBUG, "Number of settings: %u", numSettings);
    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::DEBUG, "Total bytes: %u", totalBytes);

    return totalBytes;
}

/**
 * Read a given amount of data into the buffer.
 *
 * NOT YET SUPPORTED.
 *
 * @param obj[in] CANopen stack object dictionary eleement (not used)
 * @param node[in] CANopen stack node (not used)
 * @param buf[out] The buffer to populate with data
 * @param len[in] The amount of data to read
 * @param priv[in] The private data (BQSettingsStorage)
 * @return CO_ERR_NONE on success
 */
static CO_ERR COBQSettingRead(CO_OBJ_T* obj, CO_NODE_T* node, void *buf, uint32_t len, void* priv) {

    (void)obj;
    (void)node;
    (void)buf;
    (void)len;

    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::WARNING, "Read not supported for BQSettings");

    return CO_ERR_NONE;
}

/**
 * Write out the given settings. The size of the data will be provided and
 * the buffer will contain the bytes to parse.
 *
 * @param obj[in] CANopen stack object dictionary element (not used)
 * @param node[in] CANopen stack node (not used)
 * @param buf[in] Bytes of data to read in
 * @param len[in] The number of bytes in the buf
 * @param priv[in] The private data (BQSettingsStorage)
 *
 * @return CO_ERR_NONE on success
 */
static CO_ERR COBQSettingWrite(CO_OBJ* obj, CO_NODE_T* node, void* buf, uint32_t len, void* priv) {
    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::INFO, "WRITE REQUEST MADE");
    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::INFO, "Bytes incoming: %u", len);

    uint8_t* buffer = (uint8_t*)buf;

    for(int i = 0; i < len; i++) {
        BMS::LOGGER.log(BMS::BMSLogger::LogLevel::INFO, "BYTE %u: %u", i, buffer[i]);
    }

    return CO_ERR_NONE;
}

/**
 * Internal logic to reset the read/write setup.
 *
 * @param obj[in] CANopen stack object dictionary element (not used)
 * @param node[in] CANopen stack node (not used)
 * @param func[in] The control function to execute
 * @param para[in] Input parameter data
 * @param priv[in] The private data (BQSettingsStorage)
 */
static CO_ERR COBQSettingCtrl(CO_OBJ* obj, CO_NODE_T* node, uint16_t func, uint32_t para, void* priv) {
    (void)obj;
    (void)node;
    (void)para;
    (void)priv;

    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::INFO, "CTRL FUNCTION EXECUTED");
    BMS::LOGGER.log(BMS::BMSLogger::LogLevel::INFO, "Function %u", func);

    return CO_ERR_NONE;
}


namespace BMS {

BQSettingsStorage::BQSettingsStorage() {
    canOpenInterface.Ctrl = COBQSettingCtrl;
    canOpenInterface.Read = COBQSettingRead;
    canOpenInterface.Write = COBQSettingWrite;
    canOpenInterface.Size = COBQSettingSize;
    canOpenInterface.Private = this;

    numSettings = 0;
}

uint32_t BQSettingsStorage::getNumSettings() {
    return numSettings;
}

void BQSettingsStorage::setNumSettings(uint32_t numSettings) {
    this->numSettings = numSettings;
}

void BQSettingsStorage::readSetting(uint32_t index, BQSetting& setting) {
    // TODO: NOT YET IMPLEMENTED
}

void BQSettingsStorage::writeSetting(uint32_t index, BQSetting& setting) {
    // TODO: NOT YET IMPLEMENTED
}


}  // namespace BMS
