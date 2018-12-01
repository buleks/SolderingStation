#include <cstdlib>

#include <config.h>
#include "hardwareConfig.h"
#include "i2c.h"
#include "i2c_memory.h"
#include "iwdg.h"


    uint16_t get_memory_address(uint16_t address) {
    if (core::config::memory_type == core::config::MemoryType::MEM_24C64 ||
        core::config::memory_type == core::config::MemoryType::MEM_24C32) {
        return address;
    } else {
        return address & 0xFF;
    }
}

constexpr uint16_t get_memory_address_size() {
    if (core::config::memory_type == core::config::MemoryType::MEM_24C64 ||
        core::config::memory_type == core::config::MemoryType::MEM_24C32) {
        return 2;
    } else {
        return 1;
    }
}

constexpr uint8_t get_i2c_address(uint16_t memory_address) {
    if (core::config::memory_type == core::config::MemoryType::MEM_24C32 ||
        core::config::memory_type == core::config::MemoryType::MEM_24C64) {
        return config::memory_base_address;
    } else {
        uint8_t page = static_cast<uint8_t>(memory_address >> 8);
        page &= 0b111;
        uint8_t address_7b = config::memory_base_address | page;
        return (address_7b << 1);
    }
}

void write_block(uint16_t address, gsl::span<const uint8_t> data) {
    HAL_I2C_Mem_Write(&hi2c1,
        get_i2c_address(address),
        get_memory_address(address),
        get_memory_address_size(),
        const_cast<uint8_t*>(data.data()),
        data.size(),
        config::memory_timeout_ms);
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(5);
}

void read_block(uint16_t address, gsl::span<uint8_t> data) {
    HAL_I2C_Mem_Read(
        &hi2c1, get_i2c_address(address), get_memory_address(address), get_memory_address_size(), data.data(), data.size(), config::memory_timeout_ms);

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
