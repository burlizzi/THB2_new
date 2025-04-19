/*
 ringservice.c
*/

/*********************************************************************
	INCLUDES
*/
#include "bcomdef.h"
#include "types.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "gattservapp.h"
#include "peripheral.h"

#include "ringservice.h"
#include "sensors.h"
/*********************************************************************
	MACROS
*/
/*********************************************************************
	CONSTANTS
*/

#define RING_LEVEL_VALUE_IDX		2 // Position of ringery level in attribute array
#define RING_LEVEL_VALUE_CCCD_IDX	3 // Position of ringery level CCCD in attribute array
//FE2C1238-8366-4814-8EB0-01DE32100BEA
//#define RING_SERV_UUID                 0xFE2C //google fast pair service
#define RING_SERV_UUID                 0x12F4 //google sound service
//#define RING_LEVEL_UUID                0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x38, 0x12, 0x2C, 0xFE
//8E0C0001-1D68-FB92-BF61-48377421680E
#define RING_LEVEL_UUID                    0x0E, 0x68, 0x21, 0x74, 0x37, 0x48, 0x61, 0xBF, 0x92, 0xFB, 0x68, 0x1D, 0x01, 0x00, 0x0C, 0x8E
//#define SOUND_LEVEL_UUID                    0x8E, 0x0C, 0x00, 0x01, 0x1D, 0x68, 0xFB, 0x92, 0xBF, 0x61, 0x48, 0x37, 0x74, 0x21, 0x68, 0x0E                

#define RING_CHAR_LEN 20
/*********************************************************************
	TYPEDEFS
*/

/*********************************************************************
	GLOBAL VARIABLES
*/
// Ringery service
CONST uint8 ringServUUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(RING_SERV_UUID), HI_UINT16(RING_SERV_UUID)
};

// Ringery level characteristic
CONST uint8 ringLevelUUID[ATT_UUID_SIZE] =
{
	RING_LEVEL_UUID
};

// Characteristic value
static uint8 ringCharValue[RING_CHAR_LEN] = {0};

/*********************************************************************
	EXTERNAL VARIABLES
*/

/*********************************************************************
	EXTERNAL FUNCTIONS
*/

/*********************************************************************
	LOCAL VARIABLES
*/

// Application callback
static ringServiceCB_t ringServiceCB;

/*********************************************************************
	Profile Attributes - variables
*/

// Ringery Service attribute
static CONST gattAttrType_t ringService = { ATT_BT_UUID_SIZE, ringServUUID };

// Ringery level characteristic
static uint8 ringLevelProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY;

static gattCharCfg_t ringLevelClientCharCfg[GATT_MAX_NUM_CONN];

/*********************************************************************
	Profile Attributes - Table
*/

static gattAttribute_t ringAttrTbl[] =
{
	// Ringery Service
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
		GATT_PERMIT_READ,						  /* permissions */
		0,										  /* handle */
		(uint8*)& ringService					  /* pValue */
	},

	// Ringery Level Declaration
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&ringLevelProps
	},

	// Ringery Level Value
	{
		{ ATT_UUID_SIZE, ringLevelUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		ringCharValue
	},

	// Ringery Level Client Characteristic Configuration
	{
		{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		(uint8*)& ringLevelClientCharCfg
	}
};


/*********************************************************************
	LOCAL FUNCTIONS
*/
static uint8 ringReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
							 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t ringWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
								  uint8* pValue, uint16 len, uint16 offset );
static void ringNotifyCB( linkDBItem_t* pLinkItem );
//static void ringNotifyLevel( void );

/*********************************************************************
	PROFILE CALLBACKS
*/
// Ringery Service Callbacks
CONST gattServiceCBs_t ringCBs =
{
	ringReadAttrCB,	 // Read callback function pointer
	ringWriteAttrCB, // Write callback function pointer
	NULL			 // Authorization callback function pointer
};

/*********************************************************************
	PUBLIC FUNCTIONS
*/

/*********************************************************************
	@fn		 Ring_AddService

	@brief	 Initializes the Ringery Service by registering
			GATT attributes with the GATT server.

	@return	 Success or Failure
*/
bStatus_t Ring_AddService( void )
{
	uint8 status = SUCCESS;
	// Initialize Client Characteristic Configuration attributes
	GATTServApp_InitCharCfg( INVALID_CONNHANDLE, ringLevelClientCharCfg );
	// Register GATT attribute list and CBs with GATT Server App
	status = GATTServApp_RegisterService( ringAttrTbl,
										  GATT_NUM_ATTRS( ringAttrTbl ),
										  &ringCBs );
	return ( status );
}

/*********************************************************************
	@fn		 Ring_Register

	@brief	 Register a callback function with the Ringery Service.

	@param	 pfnServiceCB - Callback function.

	@return	 None.
*/
extern void Ring_Register( ringServiceCB_t pfnServiceCB )
{
	ringServiceCB = pfnServiceCB;
}
uint8 data[100]="hello";
/*********************************************************************
	@fn			 ringReadAttrCB

	@brief		 Read an attribute.

	@param		 connHandle - connection message was received on
	@param		 pAttr - pointer to attribute
	@param		 pValue - pointer to data to be read
	@param		 pLen - length of data to be read
	@param		 offset - offset of the first octet to be read
	@param		 maxLen - maximum length of data to be read

	@return		 Success or Failure
*/
static uint8 ringReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
							 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
	(void)connHandle;
	(void)maxLen;
	bStatus_t status = SUCCESS;

	// Make sure it's not a blob operation (no attributes in the profile are long)
	if ( offset > 0 )
	{
		return ( ATT_ERR_ATTR_NOT_LONG );
	}

//	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1] );

	// Measure ringery level if reading level
//	if ( memcmp(pAttr->type.uuid , ringLevelUUID,16) )
	{
				*pLen = 5;
		memcpy(pValue,data,5);
	}
	

	return ( status );
}

/*********************************************************************
	@fn		 ringWriteAttrCB

	@brief	 Validate attribute data prior to a write operation

	@param	 connHandle - connection message was received on
	@param	 pAttr - pointer to attribute
	@param	 pValue - pointer to data to be written
	@param	 len - length of data
	@param	 offset - offset of the first octet to be written

	@return	 Success or Failure
*/
static bStatus_t ringWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
								  uint8* pValue, uint16 len, uint16 offset )
{
	bStatus_t status = SUCCESS;
	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    LUCA_LOG("ringWriteAttrCB uuid:%x\n",uuid);

    uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );
    memcpy(data,pValue,len);
    if (charCfg==0x0300)
    {
        pwm_buzzer_start();
    }

    LUCA_LOG("pValue(%d-%d):\n",len,offset);
    for (size_t i = 0; i < len; i++)
    {
        LUCA_LOG(" %x",pValue[i]);
    }
    LUCA_LOG("\n");
    
    switch ( uuid )
	{
	case GATT_CLIENT_CHAR_CFG_UUID:
		status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
												 offset, GATT_CLIENT_CFG_NOTIFY );

		if ( status == SUCCESS )
		{
            
			if ( ringServiceCB )
			{
				(*ringServiceCB)( (charCfg == GATT_CFG_NO_OPERATION) ?
								  RING_LEVEL_NOTI_DISABLED :
								  RING_LEVEL_NOTI_ENABLED);
			}
		}

		break;

	default:
		status = ATT_ERR_ATTR_NOT_FOUND;
		break;
	}

	return ( status );
}

/*********************************************************************
	@fn			 ringNotifyCB

	@brief		 Send a notification of the level state characteristic.

	@param		 connHandle - linkDB item

	@return		 None.
*/
static void ringNotifyCB( linkDBItem_t* pLinkItem )
{
	if ( pLinkItem->stateFlags & LINK_CONNECTED )
	{
		uint16 value = GATTServApp_ReadCharCfg( pLinkItem->connectionHandle,
												ringLevelClientCharCfg );

		if ( value & GATT_CLIENT_CFG_NOTIFY )
		{
			attHandleValueNoti_t noti;
			noti.handle = ringAttrTbl[RING_LEVEL_VALUE_IDX].handle;
			noti.len = 1;
			noti.value[0] = 0;
			GATT_Notification( pLinkItem->connectionHandle, &noti, FALSE );
		}
	}
}

/*********************************************************************
	@fn		 ringNotifyLevelState

	@brief	 Send a notification of the ringery level state
			characteristic if a connection is established.

	@return	 None.
*/
void RingNotifyLevel( void )
{
	// Execute linkDB callback to send notification
	linkDB_PerformFunc( ringNotifyCB );
}

/*********************************************************************
	@fn			 Ring_HandleConnStatusCB

	@brief		 Ringery Service link status change handler function.

	@param		 connHandle - connection handle
	@param		 changeType - type of change

	@return		 none
*/
void Ring_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
	// Make sure this is not loopback connection
	if ( connHandle != LOOPBACK_CONNHANDLE )
	{
		// Reset Client Char Config if connection has dropped
		if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )		 ||
				( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
				  ( !linkDB_Up( connHandle ) ) ) )
		{
			GATTServApp_InitCharCfg( connHandle, ringLevelClientCharCfg );
		}
	}
}


/*********************************************************************
*********************************************************************/
