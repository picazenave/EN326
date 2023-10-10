#include "SCD4.hpp"

SCD4::SCD4(PinName i2c_sda, PinName i2c_scl) : _bus(i2c_sda, i2c_scl)
{
}

bool checkCRC(char *data, uint8_t len)
{
    return true;
}

char SCD4::computeCRC(char *data, uint8_t len)
{
    uint32_t crc;
    MbedCRC<SCD4X_POLY, SCD4X_WIDTH_POLY> ct(
        SCD4X_INIT_XOR, SCD4X_FINAL_XOR, SCD4X_REFLECT_IN, SCD4X_REFLECT_OUT);

    ct.compute(data, len, &crc);
    return (crc & 0xFF);
}

int SCD4::sendCommand(Command cmd)
{
    char command[2] = {(char)(((uint16_t)cmd & 0xFF00) >> 8), (char)((uint16_t)0xb1 & 0x00FF)};
    _bus.lock();
    _bus.write(SCD4X_ADDR, command, 2);
    _bus.unlock();

    return 0; // FIXME
}

SCD4::ErrorType SCD4::read(
    Command cmd, uint8_t len, char *val_out)
{
    char command[2] = {(char)(((uint16_t)cmd & 0xFF00) >> 8), (char)((uint16_t)0xb1 & 0x00FF)};
    _bus.lock();
    _bus.write(SCD4X_ADDR, command, 2);

    _bus.read(SCD4X_ADDR, val_out, len);
    _bus.unlock();
    return ErrorType::Ok; // FIXME
}


SCD4::ErrorType SCD4::write(Command cmd, uint8_t len, char *val_in)
{
    char command[2] ={(char)(((uint16_t)cmd & 0xFF00) >> 8), (char)((uint16_t)0xb1 & 0x00FF)};
    _bus.lock();
    _bus.write(SCD4X_ADDR, command, 2);

    _bus.write(SCD4X_ADDR, val_in, len);
    _bus.unlock();
    return ErrorType::Ok; // FIXME
}