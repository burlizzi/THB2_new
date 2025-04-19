/*
 ringservice.h
*/


#ifndef RINGSERVICE_H
#define RINGSERVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
	INCLUDES
*/

/*********************************************************************
	CONSTANTS
*/

// Callback events
#define RING_LEVEL_NOTI_ENABLED			1
#define RING_LEVEL_NOTI_DISABLED		2

/*********************************************************************
	TYPEDEFS
*/

// Ringery Service callback function
typedef void (*ringServiceCB_t)(uint8 event);

// Ringery measure HW setup function
typedef void (*ringServiceSetupCB_t)(void);

// Ringery measure percentage calculation function
typedef uint8 (*ringServiceCalcCB_t)(uint16 adcVal);

// Ringery measure HW teardown function
typedef void (*ringServiceTeardownCB_t)(void);

/*********************************************************************
	MACROS
*/

/*********************************************************************
	Profile Callbacks
*/


/*********************************************************************
	API FUNCTIONS
*/

/*********************************************************************
	@fn		 Ring_AddService

	@brief	 Initializes the Ringery service by registering
			GATT attributes with the GATT server.

	@return	 Success or Failure
*/
extern bStatus_t Ring_AddService( void );

/*********************************************************************
	@fn		 Ring_Register

	@brief	 Register a callback function with the Ringery Service.

	@param	 pfnServiceCB - Callback function.

	@return	 None.
*/
extern void Ring_Register( ringServiceCB_t pfnServiceCB );

/*********************************************************************
	@fn		 Ring_SetParameter

	@brief	 Set a Ringery Service parameter.

	@param	 param - Profile parameter ID
	@param	 len - length of data to right
	@param	 value - pointer to data to write.	This is dependent on
			the parameter ID and WILL be cast to the appropriate
			data type (example: data type of uint16 will be cast to
			uint16 pointer).

	@return	 bStatus_t
*/
extern bStatus_t Ring_SetParameter( uint8 param, uint8 len, void* value );

/*********************************************************************
	@fn		 Ring_GetParameter

	@brief	 Get a Ringery parameter.

	@param	 param - Profile parameter ID
	@param	 value - pointer to data to get.  This is dependent on
			the parameter ID and WILL be cast to the appropriate
			data type (example: data type of uint16 will be cast to
			uint16 pointer).

	@return	 bStatus_t
*/
extern bStatus_t Ring_GetParameter( uint8 param, void* value );

/*********************************************************************
	@fn			 Ring_MeasLevel

	@brief		 Measure the ringery level and update the ringery
				level value in the service characteristics.	 If
				the ringery level-state characteristic is configured
				for notification and the ringery level has changed
				since the last measurement, then a notification
				will be sent.

	@return		 Success or Failure
*/
extern bStatus_t Ring_MeasLevel( void );

/*********************************************************************
	@fn		 Ring_Setup

	@brief	 Set up which ADC source is to be used. Defaults to VDD/3.

	@param	 adc_ch - ADC Channel, e.g. HAL_ADC_CHN_AIN6
	@param	 minVal - max ringery level
	@param	 maxVal - min ringery level
	@param	 sCB - HW setup callback
	@param	 tCB - HW tear down callback
	@param	 cCB - percentage calculation callback

	@return	 none.
*/
extern void Ring_Setup( uint8 adc_ch, uint16 minVal, uint16 maxVal,
						ringServiceSetupCB_t sCB, ringServiceTeardownCB_t tCB,
						ringServiceCalcCB_t cCB );

/*********************************************************************
	@fn			 Ring_HandleConnStatusCB

	@brief		 Ringery Service link status change handler function.

	@param		 connHandle - connection handle
	@param		 changeType - type of change

	@return		 none
*/
void Ring_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
	@fn		 ringNotifyLevelState

	@brief	 Send a notification of the ringery level state
			characteristic if a connection is established.

	@return	 None.
*/
void RingNotifyLevel( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* RINGSERVICE_H */
