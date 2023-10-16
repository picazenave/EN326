#include "SCD4.hpp"

#define U16_TO_BYTE_ARRAY(u, ba) \
    do                           \
    {                            \
        ba[0] = (u >> 8) & 0xFF; \
        ba[1] = u & 0xFF;        \
    } while (0)

#define BYTE_ARRAY_TO_U16(ba, u)    \
    do                              \
    {                               \
        u = (ba[0] << 8) | (ba[1]); \
    } while (0)

SCD4::SCD4(PinName i2c_sda, PinName i2c_scl) : _bus(i2c_sda, i2c_scl)
{
}

// FIXME
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

SCD4::ErrorType SCD4::sendCommand(Command cmd)
{
    char command[2] = {(char)(((uint16_t)cmd & 0xFF00) >> 8), (char)((uint16_t)0xb1 & 0x00FF)};
    _bus.lock();
    int r = _bus.write(SCD4X_ADDR, command, 2);
    _bus.unlock();

    return r == 0 ? ErrorType::Ok : ErrorType::I2cError;
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
    char command[2] = {(char)(((uint16_t)cmd & 0xFF00) >> 8), (char)((uint16_t)0xb1 & 0x00FF)};
    _bus.lock();
    _bus.write(SCD4X_ADDR, command, 2);

    _bus.write(SCD4X_ADDR, val_in, len);
    _bus.unlock();
    return ErrorType::Ok; // FIXME
}

// FIXME check CRC
SCD4::ErrorType SCD4::read_measurement(scd4x_measurement_t *data)
{
    ErrorType r;
    char value[9] = {0};
    r = this->read(Command::ReadMeasurement, 9, value);

    data->co2 = (value[0] << 8) + value[1];
    uint16_t a = (value[3] << 8) + value[4];
    // FIXME offset
    int offset = -45;
    data->temperature = (float)offset + (float)175 * (a / (float)(1 << 16));
    a = (value[6] << 8) + value[7];
    data->rh = (float)100 * (a / (float)(1 << 16));

    return r;
}
// PIERRE

SCD4::ErrorType SCD4::set_temperature_offset(float t)
{
    ErrorType r;
    char value[3];
    int16_t offset = t * (float)(1 << 16) / 175;
    U16_TO_BYTE_ARRAY(offset, value);
    value[2] = computeCRC(value, 2);
    r = this->write(Command::SetTemperatureOffset, 2, value);
    return r;
} // PIERRE

SCD4::ErrorType SCD4::set_sensor_altitude(uint16_t alt)
{
    if (alt > 3000)
        return ErrorType::DataNotReady;

    ErrorType r;
    char value[3];
    U16_TO_BYTE_ARRAY(alt, value);
    value[2] = computeCRC(value, 2);
    r = this->write(Command::SetSensorAltitude, 2, value);
    return r;
} // PIERRE

SCD4::ErrorType SCD4::set_ambient_pressure(uint16_t hpa)
{
    if (hpa / 100 < 700 || hpa / 100 > 1200)
        return ErrorType::DataNotReady;

    ErrorType r;
    char value[3];
    U16_TO_BYTE_ARRAY((hpa / 100), value);
    value[2] = computeCRC(value, 2);
    r = this->write(Command::SetAmbientPressure, 2, value);
    return r;
} // PIERRE

SCD4::ErrorType SCD4::get_data_ready_status()
{
    ErrorType r;
    char value[3] = {0};
    r = this->read(Command::GetDataReadyStatus, 3, value);
    if (r != ErrorType::Ok)
        return ErrorType::I2cError;
    int16_t enabled;
    BYTE_ARRAY_TO_U16(value, enabled);
    printf("get_data_ready returns : %X\n",enabled);
    if ((enabled & 0xFFF) == 0)
        return ErrorType::DataNotReady;
    else
        return ErrorType::Ok;
} // PIERRE

SCD4::ErrorType SCD4::start_periodic_measurement()
{
    return this->sendCommand(Command::StartPeriodicMeasurement);
}

SCD4::ErrorType SCD4::stop_periodic_measurement()
{
    return this->sendCommand(Command::StopPeriodicMeasurement);
}

SCD4::ErrorType SCD4::get_temperature_offset(float *t)
{
    SCD4::ErrorType error;
    char value_offset[3] = {0};
    error = this->read(Command::GetTemperatureOffset, 3, value_offset);
    float word_0 = (value_offset[0] << 8) + value_offset[1];
    // printf("\n\nword[0] %x, word 1, %x\n\n", value_offset[0], value_offset[1]);
    *t = (175 * word_0) / (float)(1 << 16);
    return error;
}

SCD4::ErrorType SCD4::get_sensor_altitude(uint16_t *alt)
{
    SCD4::ErrorType error;
    char value_alt[3] = {0};
    error = this->read(Command::GetSensorAltitude, 3, value_alt);
    float word_0 = (value_alt[0] << 8) + value_alt[1];
    *alt = word_0;
    return error;
}

SCD4::ErrorType SCD4::measure_single_shot()
{
    return this->sendCommand(Command::MeasureSingleShot);
}

SCD4::ErrorType SCD4::start_low_power_periodic_measurement(){
    return this->sendCommand(Command::StartLowPowerPeriodicMeasurement);
}