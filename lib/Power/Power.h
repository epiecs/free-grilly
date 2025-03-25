#include <Arduino.h>

// ----------------------------------------- //
//                Definitions                //
// ----------------------------------------- //

#define BAT_I2C                            0x55   // I2C address
#define BAT_DEVICE_ID	                   0x0421 // Default device ID
#define BAT_FLAGS   	                   0x06   // Flags
#define BAT_FLAG_CHARGE  	               (1<<8) // Charge flag bit definition
#define BAT_TIMEOUT         		       200    // Timeout that can be used after writing

#define BAT_CONTROL_DEVICE_TYPE		       0x01   // Control code for initialization with the IC
#define BAT_VOLTAGE         		       0x04   // Voltage (V) 
#define BAT_CURRENT_AVG         	       0x10   // Average current (A) 
#define BAT_CURRENT_STBY         	       0x12   // Standby current (A) 
#define BAT_CURRENT_MAX         	       0x14   // Maximum current (A) 
#define BAT_POWER                	       0x18   // Power (W)
#define BAT_SOC_FILTERED           	       0x1C   // State of charge (%) filtered
#define BAT_SOC_UNFILTERED                 0x30   // State of charge (%) unfiltered
#define BAT_TEMP                           0x02   // Temperature of the battery (C or F)

#define PWR_SCREEN_LED                     4      // GPIO of the screen LED
#define PWR_IC		                       13     // GPIO of the IC's
#define PWR_PROBES		                   27     // GPIO of the probes


typedef enum {
	AVG,  
	STBY, 
	MAX   
} current_measure;

typedef enum {
	FILTERED,  
	UNFILTERED, 
} soc_measure;

typedef enum {
	C,      
	F 
} temp_unit;

typedef enum {
	ENABLE,      
	DISABLE 
} pwr_state;


class bat {
public:
    bat();
    /**
	    Verifies communication with the battery IC.
		@return true if communication was successful.
	*/
    bool init(void);       

    // ----------------------------------------- //
    //          BATTERY CHARACTERISTICS          //
    // ----------------------------------------- //
    /**
	    Reads and returns the battery voltage
		@return battery voltage measured in V
	*/
	float voltage(void);
    /**
	    Reads and returns the specified current
		@param current_measure enum specifying current value to be read
		@return battery current measured in A
	*/
    float current(current_measure type = AVG);
    /**
	    Reads and returns measured average power
		@return average power in W
	*/
	float power(void);
    /**
	    Reads and returns specified state of charge measurement
		@return state of charge measurement in %
	*/
    uint16_t soc(soc_measure type = FILTERED);
    /**
	    Reads and returns specified temperature measurement
		@param temp_unit enum specifying celcius or farenheit
		@return specified temperature measurement in degrees C or F
	*/
	float temp(temp_unit type = C);
    /**
	    Check if the battery is charging
		@return true if battery is charging
	*/
    bool chargeFlag(void);

private:
    // ----------------------------------------- //
    //              INIT FUNCTION                //
    // ----------------------------------------- //
    uint8_t _deviceAddress;  // Stores the battery ic I2C address
    /**
	    Read the device type - should be 0x0421
		@return 16-bit value read from DEVICE_TYPE subcommand
	*/ 
    uint16_t deviceType(void);    
    /**
	    Read the flags() command
		@return 16-bit representation of flags() command register
	*/
	uint16_t flags(void); 
    /**
        Read a 16-bit command word from the battery ic
        @param subAddress is the command to be read from
        @return 16-bit value of the command's contents
	*/	
	uint16_t readWord(uint16_t subAddress);
    /**
        Read a 16-bit subcommand() from the battery ic control()
        @param function is the subcommand of control() to be read
        @return 16-bit value of the subcommand's contents
    */	
    uint16_t readControlWord(uint16_t function);   


    // ----------------------------------------- //
    //        I2C READ AND WRITE ROUTINES        //
    // ----------------------------------------- //
	/**
	    Read a specified number of bytes over I2C at a given subAddress
		@param subAddress is the 8-bit address of the data to be read dest is the data buffer to be written to count is the number of bytes to be read
		@return true on success
	*/
	uint16_t i2cReadBytes(uint8_t subAddress, uint8_t * dest, uint8_t count);
	/**
	    Write a specified number of bytes over I2C to a given subAddress
		@param subAddress is the 8-bit address of the data to be written to src is the data buffer to be written count is the number of bytes to be written
		@return true on success
	*/
	uint16_t i2cWriteBytes(uint8_t subAddress, uint8_t * src, uint8_t count);
};

class pwr {
public:
	pwr();
    // ----------------------------------------- //
    //          POWER DELIVERY FUNCTIONS         //
    // ----------------------------------------- //
	/**
	    initializes the GPIO pins to output.
		@return true if GPIO pins are set correctly
	*/
	bool init(void);   
	/**
	    Enable or disable the power to the given power rail
		@param pwr_state enum specifying the enable or disable of the power rail
		@param GPIO GPIO pin of the power rail
		@return true on success
	*/
    bool setPowerRail(pwr_state type = DISABLE, int GPIO = PWR_SCREEN_LED);
};

extern bat battery;
extern pwr power;