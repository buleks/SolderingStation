#include <cstdlib>

#include "hardwareConfig.h"
#include "i2c.h"
#include "i2c_memory.h"
#include "iwdg.h"


void write_block(uint16_t address, gsl::span<const uint8_t> data) {
    HAL_I2C_Mem_Write(
        &hi2c1, 0b10100000, address, 0x02, const_cast<uint8_t*>(data.data()), data.size(), config::memory_timeout_ms);
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(5);
}

void read_block(uint16_t address, gsl::span<uint8_t> data) {
    HAL_I2C_Mem_Read(&hi2c1,0b10100000, address & 0xFF, 0x02, data.data(), data.size(), config::memory_timeout_ms);
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(5);
}

template <typename Operation, typename Data>
void i2cMemoryBlockOperation(Operation operation, uint16_t address, gsl::span<Data> data) {
    // at least one block
    while (data.size() >= 8) {
        // operation on first 8 bytes
        operation(address, data.first(8));

        // skip first 8 bytes
        data = data.subspan(8);
        address += 8;
    }

    // anything left in the buffer?
    if (data.size() > 0) {
        operation(address, data);
    }
}

void i2cMemoryWriteBlock(uint16_t address, gsl::span<const uint8_t> data) {
    i2cMemoryBlockOperation(write_block, address, data);
}

void i2cMemoryReadBlock(uint16_t address, gsl::span<uint8_t> data) {
    i2cMemoryBlockOperation(read_block, address, data);
}
