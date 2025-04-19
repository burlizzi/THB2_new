/*
 fastpairservice.h
*/


#ifndef FASTPAIRSERVICE_H
#define FASTPAIRSERVICE_H

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
#define FASTPAIR_LEVEL_NOTI_ENABLED			1
#define FASTPAIR_LEVEL_NOTI_DISABLED		2

/*********************************************************************
	TYPEDEFS
*/

// FastPairery Service callback function
typedef void (*fastpairServiceCB_t)(uint8 event);

// FastPairery measure HW setup function
typedef void (*fastpairServiceSetupCB_t)(void);

// FastPairery measure percentage calculation function
typedef uint8 (*fastpairServiceCalcCB_t)(uint16 adcVal);

// FastPairery measure HW teardown function
typedef void (*fastpairServiceTeardownCB_t)(void);

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
	@fn		 FastPair_AddService

	@brief	 Initializes the FastPairery service by registering
			GATT attributes with the GATT server.

	@return	 Success or Failure
*/
extern bStatus_t FastPair_AddService( void );

/*********************************************************************
	@fn		 FastPair_Register

	@brief	 Register a callback function with the FastPairery Service.

	@param	 pfnServiceCB - Callback function.

	@return	 None.
*/
extern void FastPair_Register( fastpairServiceCB_t pfnServiceCB );

/*********************************************************************
	@fn		 FastPair_SetParameter

	@brief	 Set a FastPairery Service parameter.

	@param	 param - Profile parameter ID
	@param	 len - length of data to right
	@param	 value - pointer to data to write.	This is dependent on
			the parameter ID and WILL be cast to the appropriate
			data type (example: data type of uint16 will be cast to
			uint16 pointer).

	@return	 bStatus_t
*/
extern bStatus_t FastPair_SetParameter( uint8 param, uint8 len, void* value );

/*********************************************************************
	@fn		 FastPair_GetParameter

	@brief	 Get a FastPairery parameter.

	@param	 param - Profile parameter ID
	@param	 value - pointer to data to get.  This is dependent on
			the parameter ID and WILL be cast to the appropriate
			data type (example: data type of uint16 will be cast to
			uint16 pointer).

	@return	 bStatus_t
*/
extern bStatus_t FastPair_GetParameter( uint8 param, void* value );

/*********************************************************************
	@fn			 FastPair_MeasLevel

	@brief		 Measure the fastpairery level and update the fastpairery
				level value in the service characteristics.	 If
				the fastpairery level-state characteristic is configured
				for notification and the fastpairery level has changed
				since the last measurement, then a notification
				will be sent.

	@return		 Success or Failure
*/
extern bStatus_t FastPair_MeasLevel( void );

/*********************************************************************
	@fn		 FastPair_Setup

	@brief	 Set up which ADC source is to be used. Defaults to VDD/3.

	@param	 adc_ch - ADC Channel, e.g. HAL_ADC_CHN_AIN6
	@param	 minVal - max fastpairery level
	@param	 maxVal - min fastpairery level
	@param	 sCB - HW setup callback
	@param	 tCB - HW tear down callback
	@param	 cCB - percentage calculation callback

	@return	 none.
*/
extern void FastPair_Setup( uint8 adc_ch, uint16 minVal, uint16 maxVal,
						fastpairServiceSetupCB_t sCB, fastpairServiceTeardownCB_t tCB,
						fastpairServiceCalcCB_t cCB );

/*********************************************************************
	@fn			 FastPair_HandleConnStatusCB

	@brief		 FastPairery Service link status change handler function.

	@param		 connHandle - connection handle
	@param		 changeType - type of change

	@return		 none
*/
void FastPair_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
	@fn		 fastpairNotifyLevelState

	@brief	 Send a notification of the fastpairery level state
			characteristic if a connection is established.

	@return	 None.
*/
void FastPairNotifyLevel( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* FASTPAIRSERVICE_H */
