#ifndef SCD4_H
#define SCD4_H

#include "mbed.h"
#include "MbedCRC.h"
#include <cmath>
#include <chrono>
#include <cstdint>

using rtos::Kernel::Clock;
using std::chrono::operator""ms;

#define SCD4X_ADDR (0x62 << 1)

#define SCD4X_POLY (0x31)
#define SCD4X_WIDTH_POLY (8)
#define SCD4X_INIT_XOR (0xFF)
#define SCD4X_FINAL_XOR (0)
#define SCD4X_REFLECT_IN (false)
#define SCD4X_REFLECT_OUT (false)

#define READ_WAIT_TIME (5s)
#define SELF_TEST_WAIT_TIME (10s)
#define FORCED_CALIBRATION_WAIT_TIME (400ms)

/*!
 * \brief Sensor data
 */
typedef struct
{
    /** Temperature in Celsius degrees */
    double temperature;
    /** Relative humidity in % */
    double rh;
    /** CO2 in ppm */
    uint16_t co2;
} scd4x_measurement_t;

class SCD4
{
public:
    /*!
     * \brief Constructor
     */
    SCD4(PinName i2c_sda, PinName i2c_scl);
    /*!
     * \brief Commands error codes
     */
    enum class ErrorType
    {
        /** Command successful */
        Ok = 0,
        /** Error returned by I2C communication */
        I2cError,
        /** Requested read size is too large */
        ReadSizeTooLarge,
        /** CRC mismatch in received frame */
        CrcError,
        /** Sensor data is not ready to be read */
        DataNotReady,
        /** Forced calibration failure */
        FrcError,
        /** Self test failure */
        SelfTestError,
    };

    /*!
     * \brief Send command to start periodic measurement
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType start_periodic_measurement();//CLEMENT

    /*!
     * \brief Send command to start periodic measurement
     *
     * \param data pointer to data to store read values
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType read_measurement(scd4x_measurement_t *data);//PIERRE

    /*!
     * \brief Send command to stop periodic measurement
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType stop_periodic_measurement();//CLEMENT

    /*!
     * \brief Set temperature offset
     *
     * \param t temperature in Celsius degrees to set
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType set_temperature_offset(float t);//PIERRE

    /*!
     * \brief Get temperature offset
     *
     * \param t pointer to data to store read temperature in Celsius degrees
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType get_temperature_offset(float *t);//CLEMENT

    /*!
     * \brief Set sensor altitude
     *
     * \param alt altitude in meters to set
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType set_sensor_altitude(uint16_t alt);//PIERRE

    /*!
     * \brief Get sensor altitude
     *
     * \param alt pointer to data to store read altitude in meters
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType get_sensor_altitude(uint16_t *alt);//CLEMENT

    /*!
     * \brief Set ambient pressure
     *
     * \param hpa pressure in hPa to set
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType set_ambient_pressure(uint16_t hpa);//PIERRE

    /*!
     * \brief Read data ready status
     *
     * \return Ok if data ready, DataNotReady if data not ready, the reason of failure otherwise
     */
    ErrorType get_data_ready_status();//PIERRE

    /*!
     * \brief Start a single shot measurement of CO2 level, temperature and humidity
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType measure_single_shot();//CLEMENT


    enum class Command : uint16_t
    {
        StartPeriodicMeasurement = 0x21b1,
        ReadMeasurement = 0xec05,
        StopPeriodicMeasurement = 0x3f86,
        SetTemperatureOffset = 0x241d,
        GetTemperatureOffset = 0x2318,
        SetSensorAltitude = 0x2427,
        GetSensorAltitude = 0x2322,
        SetAmbientPressure = 0xe000,
        PerformForcedRecalibration = 0x362f,
        SetAutomaticSelfCalibrationEnabled = 0x2416,
        GetAutomaticSelfCalibrationEnabled = 0x2313,
        StartLowPowerPeriodicMeasurement = 0x21ac,
        GetDataReadyStatus = 0xe4b8,
        PersistSettings = 0x3615,
        GetSerialNumber = 0x3682,
        PerformSelfTest = 0x3639,
        PerformFactoryReset = 0x3632,
        Reinit = 0x3646,
        MeasureSingleShot = 0x219d,
        MeasureSingleShotRhtOnly = 0x2196,
    };

    /*!
     * \brief Send a command to the sensor
     *
     * \param cmd command to send
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType sendCommand(Command cmd);
    /*!
     * \brief Read data from the sensor
     *
     * \param cmd command to read data from
     * \param len number of 16bit words to read from the sensor
     * \param val_out pointer to store read data
     * \param exec_time time to wait between acknowledgement of the command and the start of the
     *                  read transaction
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType read(
        Command cmd, uint8_t len, char *val_out);
    /*!
     * \brief Write data to the sensor
     *
     * \param cmd command to write data to
     * \param len number of 16bit words to write
     * \param val_in pointer to the data to write
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType write(Command cmd, uint8_t len, char *val_in);
    /*!
     * \brief Read data from the sensor
     *
     * \param cmd command to send and fetch data
     * \param val_in pointer to the data to write
     * \param val_out pointer to store read data
     * \param exec_time time to wait between acknowledgement of the write command and the start of
     *                  the read transaction
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType send_and_fetch( // perform_forced_calibration
        Command cmd, uint16_t *val_in, uint16_t *val_out, Clock::duration_u32 exec_time = 1ms);

private:
    I2C _bus;
    char computeCRC(char *data, uint8_t len);
    bool checkCRC(char *data, uint8_t len);

    
};

#endif