#ifndef _CCS811_H
#define _CCS811_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Wire.h>


/*I2C ADDRESS*/
#define CCS811_I2C_ADDRESS1                      0x5A
#define CCS811_I2C_ADDRESS2                      0x5B

#define CCS811_REG_STATUS                        0x00
#define CCS811_REG_MEAS_MODE                     0x01
#define CCS811_REG_ALG_RESULT_DATA               0x02
#define CCS811_REG_RAW_DATA                      0x03
#define CCS811_REG_ENV_DATA                      0x05
#define CCS811_REG_NTC                           0x06
#define CCS811_REG_THRESHOLDS                    0x10
#define CCS811_REG_BASELINE                      0x11
#define CCS811_REG_HW_ID                         0x20
#define CCS811_REG_HW_VERSION                    0x21
#define CCS811_REG_FW_BOOT_VERSION               0x23
#define CCS811_REG_FW_APP_VERSION                0x24
#define CCS811_REG_INTERNAL_STATE                0xA0
#define CCS811_REG_ERROR_ID                      0xE0
#define CCS811_REG_SW_RESET                      0xFF

#define CCS811_BOOTLOADER_APP_ERASE              0xF1
#define CCS811_BOOTLOADER_APP_DATA               0xF2
#define CCS811_BOOTLOADER_APP_VERIFY             0xF3
#define CCS811_BOOTLOADER_APP_START              0xF4

#define CCS811_HW_ID                             0x81
//Open the macro to see the detailed program execution process.
//#define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

class CCS811
{
public:
    #define ERR_OK             0      //OK 
    #define ERR_DATA_BUS      -1      //error in data bus
    #define ERR_IC_VERSION    -2      //chip version mismatch
    
    uint8_t _deviceAddr;
    typedef enum{
        eMode0, //Idle (Measurements are disabled in this mode)
        eMode1, //Constant power mode, IAQ measurement every second
        eMode2, //Pulse heating mode IAQ measurement every 10 seconds
        eMode3, //Low power pulse heating mode IAQ measurement every 60 seconds
        eMode4  //Constant power mode, sensor measurement every 250ms 1xx: Reserved modes (For future use)
    }eDRIVE_MODE_t;
    
    typedef enum{
        eClosed,      //Idle (Measurements are disabled in this mode)
        eCycle_1s,    //Constant power mode, IAQ measurement every second
        eCycle_10s,   //Pulse heating mode IAQ measurement every 10 seconds
        eCycle_60s,   //Low power pulse heating mode IAQ measurement every 60 seconds
        eCycle_250ms  //Constant power mode, sensor measurement every 250ms 1xx: Reserved modes (For future use)
    }eCycle_t;
    /**
     * @brief Constructor 
     * @param Input in Wire address
     */
    CCS811(TwoWire *pWire = &Wire, uint8_t deviceAddr = 0x5A){_pWire = pWire; _deviceAddr = deviceAddr;};
    
              /**
               * @brief Constructor
               * @return Return 0 if initialization succeeds, otherwise return non-zero.
               */
    int       begin();
              /**
               * @brief Judge if there is data to read 
               * @return Return 1 if there is, otherwise return 0. 
               */
    bool      checkDataReady();
              /**
               * @brief Reset sensor, clear all configured data.
               */
    void      softReset(),
              /**
               * @brief Set environment parameter 
               * @param temperature Set temperature value, unit: centigrade, range (-40~85℃)
               * @param humidity    Set humidity value, unit: RH, range (0~100)
               */
              setInTempHum(float temperature, float humidity),
              /**
               * @brief Measurement parameter configuration 
               * @param thresh:0 for Interrupt mode operates normally; 1 for interrupt mode only asserts the nINT signal (driven low) if the new
               * @param interrupt:0 for Interrupt generation is disabled; 1 for the nINT signal is asserted (driven low) when a new sample is ready in
               * @param mode:in typedef enum eDRIVE_MODE_t
               */
              setMeasurementMode(uint8_t thresh, uint8_t interrupt, eDRIVE_MODE_t mode),
              /**
               * @brief Measurement parameter configuration 
               * @param mode:in typedef enum eDRIVE_MODE_t
               */
              setMeasCycle(eCycle_t cycle),
              /**
               * @brief Set interrupt thresholds 
               * @param lowToMed: interrupt triggered value in range low to middle 
               * @param medToHigh: interrupt triggered value in range middle to high 
               */
              setThresholds(uint16_t lowToMed, uint16_t medToHigh);
              /**
               * @brief Get current configured parameter
               * @return configuration code, needs to be converted into binary code to analyze
               *         The 2nd: Interrupt mode (if enabled) operates normally,1: Interrupt mode (if enabled) only asserts the nINT signal (driven low) if the new
               *         The 3rd: Interrupt generation is disabled,1: The nINT signal is asserted (driven low) when a new sample is ready in
               *         The 4th: 6th: in typedef enum eDRIVE_MODE_t
               */
    uint8_t   getMeasurementMode();

              /**
               * @brief Get the current carbon dioxide concentration
               * @return current carbon dioxide concentration, unit:ppm
               */
    uint16_t  getCO2PPM(),
              /**
               * @brief Get current TVOC concentration
               * @return Return current TVOC concentration, unit: ppb
               */
              getTVOCPPB();
    uint16_t  readBaseLine();
    void      writeBaseLine(uint16_t baseLine);
    
protected:

    typedef struct{
        /*
         * The CCS811 received an I²C write request addressed to this station but with invalid register address ID
         */
        uint8_t sWRITE_REG_INVALID: 1;
        /*
         * The CCS811 received an I²C read request to a mailbox ID that is invalid
         */
        uint8_t sREAD_REG_INVALID: 1;
        /*
         * The CCS811 received an I²C request to write an unsupported mode to MEAS_MODE
         */
        uint8_t sMEASMODE_INVALID: 1;
        /*
         * The sensor resistance measurement has reached or exceeded the maximum range
         */
        uint8_t sMAX_RESISTANCE: 1;
        /*
         * The The Heater current in the CCS811 is not in range
         */
        uint8_t sHEATER_FAULT: 1;
        /*
         * The Heater voltage is not being applied correctly
         */
        uint8_t sHEATER_SUPPLY: 1;
    } __attribute__ ((packed))sError_id;
    
    typedef struct{
        /* 
         * ALG_RESULT_DATA crosses one of the thresholds set in the THRESHOLDS register 
         * by more than the hysteresis value (also in the THRESHOLDS register)
         */
        uint8_t sINT_THRESH: 1;
        /* 
         * At the end of each measurement cycle (250ms, 1s, 10s, 60s) a flag is set in the
         * STATUS register regardless of the setting of this bit.
         */
        uint8_t sINT_DATARDY: 1;
        /* 
         * A new sample is placed in ALG_RESULT_DATA and RAW_DATA registers and the
         * DATA_READY bit in the STATUS register is set at the defined measurement interval.
         */
        uint8_t sDRIVE_MODE: 3;
    } __attribute__ ((packed))sMeas_mode;
    
    typedef struct{
        /* 
         * This bit is cleared by reading ERROR_ID
         * It is not sufficient to read the ERROR field of ALG_RESULT_DATA and STATUS
         */
        uint8_t sERROR: 1;
        /* 
         * ALG_RESULT_DATA is read on the I²C interface
         */
        uint8_t sDATA_READY: 1;
        uint8_t sAPP_VALID: 1;
        /* 
         * After issuing a VERIFY command the application software must wait 70ms before 
         * issuing any transactions to CCS811 over the I²C interface
         */
        uint8_t sAPP_VERIFY: 1;
        /* 
         * After issuing the ERASE command the application software must wait 500ms 
         * before issuing any transactions to the CCS811 over the I2C interface.
         */
        uint8_t sAPP_ERASE: 1;
        uint8_t sFW_MODE: 1;
    } __attribute__ ((packed))sStatus;
    
    
    void getData(void);
    
    void writeConfig();
         
    virtual void writeReg(uint8_t reg, const void* pBuf, size_t size);
    virtual uint8_t readReg(uint8_t reg, const void* pBuf, size_t size);
    
    

private:
    TwoWire *_pWire;
    
    uint16_t eCO2;
    uint16_t eTVOC;
};

#endif
