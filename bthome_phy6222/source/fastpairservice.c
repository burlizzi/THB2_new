/*
 fastpairservice.c
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

#include "fastpairservice.h"
#include "sensors.h"
#include "findmy_beacon.h"
/*********************************************************************
	MACROS
*/
/*********************************************************************
	CONSTANTS
*/

#define FASTPAIR_LEVEL_VALUE_IDX		2 // Position of fastpairery level in attribute array
#define FASTPAIR_LEVEL_VALUE_CCCD_IDX	3 // Position of fastpairery level CCCD in attribute array
//FE2C1233-8366-4814-8EB0-01DE32100BEA
//FE2C1238-8366-4814-8EB0-01DE32100BEA
#define FASTPAIR_SERV_UUID                 0xFE2C //google fast pair service
//#define FASTPAIR_SERV_UUID                 0x12F4 //google sound service
#define FASTPAIR_LEVEL_UUID                0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x38, 0x12, 0x2C, 0xFE
#define FASTPAIR_MODE_UUID                 0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x33, 0x12, 0x2C, 0xFE
#define FASTPAIR_PAIR_UUID                 0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x34, 0x12, 0x2C, 0xFE
#define FASTPAIR_KEY_UUID                  0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x35, 0x12, 0x2C, 0xFE
#define FASTPAIR_ACCOUNT_UUID              0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x36, 0x12, 0x2C, 0xFE
#define FASTPAIR_ADDITIONAL_UUID           0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x37, 0x12, 0x2C, 0xFE

//FE2C1234-8366-4814-8EB0-01DE32100BEA
//8E0C0001-1D68-FB92-BF61-48377421680E
//#define FASTPAIR_LEVEL_UUID                    0x0E, 0x68, 0x21, 0x74, 0x37, 0x48, 0x61, 0xBF, 0x92, 0xFB, 0x68, 0x1D, 0x01, 0x00, 0x0C, 0x8E
//#define SOUND_LEVEL_UUID                    0x8E, 0x0C, 0x00, 0x01, 0x1D, 0x68, 0xFB, 0x92, 0xBF, 0x61, 0x48, 0x37, 0x74, 0x21, 0x68, 0x0E                

#define FASTPAIR_CHAR_LEN 20
/*********************************************************************
	TYPEDEFS
*/

/*********************************************************************
	GLOBAL VARIABLES
*/
// FastPairery service
CONST uint8 fastpairServUUID[ATT_BT_UUID_SIZE] =
{
	LO_UINT16(FASTPAIR_SERV_UUID), HI_UINT16(FASTPAIR_SERV_UUID)
};

// FastPairery level characteristic
CONST uint8 fastpairLevelUUID[ATT_UUID_SIZE] =
{
	FASTPAIR_LEVEL_UUID
};
// FastPairery level characteristic
CONST uint8 fastpairPairlUUID[ATT_UUID_SIZE] =
{
	FASTPAIR_PAIR_UUID
};
// FastPairery level characteristic
CONST uint8 fastpairModeUUID[ATT_UUID_SIZE] =
{
	FASTPAIR_MODE_UUID
};
// FastPairery level characteristic
CONST uint8 fastpairKeyUUID[ATT_UUID_SIZE] =
{
	FASTPAIR_KEY_UUID
};
// FastPairery level characteristic
CONST uint8 fastpairAccountUUID[ATT_UUID_SIZE] =
{
	FASTPAIR_ACCOUNT_UUID
};
// FastPairery level characteristic
CONST uint8 fastpairAdditionalUUID[ATT_UUID_SIZE] =
{
	FASTPAIR_ADDITIONAL_UUID
};

// Characteristic value
static uint8 fastpairCharValue[FASTPAIR_CHAR_LEN] = {0};

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
static fastpairServiceCB_t fastpairServiceCB;

/*********************************************************************
	Profile Attributes - variables
*/

// FastPairery Service attribute
static CONST gattAttrType_t fastpairService = { ATT_BT_UUID_SIZE, fastpairServUUID };

// FastPairery level characteristic
static uint8 fastpairLevelProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY;
static uint8 fastpairModeProps = GATT_PROP_READ ;
static uint8 fastpairPairlProps =  GATT_PROP_WRITE | GATT_PROP_NOTIFY;
static uint8 fastpairKeyProps = GATT_PROP_WRITE | GATT_PROP_NOTIFY;
static uint8 fastpairAccountProps = GATT_PROP_WRITE;
static gattCharCfg_t fastpairLevelClientCharCfg[GATT_MAX_NUM_CONN];

/*********************************************************************
	Profile Attributes - Table
*/

static gattAttribute_t fastpairAttrTbl[] =
{
	// FastPairery Service
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
		GATT_PERMIT_READ,						  /* permissions */
		0,										  /* handle */
		(uint8*)& fastpairService					  /* pValue */
	},

	// FastPairery Level Declaration
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&fastpairLevelProps
	},

	// FastPairery Level Value
	{
		{ ATT_UUID_SIZE, fastpairLevelUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		fastpairCharValue
	},
	{
		{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		(uint8*)& fastpairLevelClientCharCfg
	},

	// FastPairery Level Declaration
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&fastpairModeProps
	},

	// FastPairery Level Value
	{
		{ ATT_UUID_SIZE, fastpairModeUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		fastpairCharValue
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&fastpairPairlProps
	},
	// FastPairery Level Value
	{
		{ ATT_UUID_SIZE, fastpairPairlUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		fastpairCharValue
	},
	{
		{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		(uint8*)& fastpairLevelClientCharCfg
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&fastpairKeyProps
	},
	// FastPairery Level Value
	{
		{ ATT_UUID_SIZE, fastpairKeyUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		fastpairCharValue
	},
	{
		{ ATT_BT_UUID_SIZE, clientCharCfgUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		(uint8*)& fastpairLevelClientCharCfg
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&fastpairAccountProps
	},
	// FastPairery Level Value
	{
		{ ATT_UUID_SIZE, fastpairAccountUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		fastpairCharValue
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&fastpairLevelProps
	},
	// FastPairery Level Value
	{
		{ ATT_UUID_SIZE, fastpairAdditionalUUID },
		GATT_PERMIT_READ | GATT_PERMIT_WRITE,
		0,
		fastpairCharValue
	},

	// FastPairery Level Client Characteristic Configuration
};


/*********************************************************************
	LOCAL FUNCTIONS
*/
static uint8 fastpairReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
							 uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );
static bStatus_t fastpairWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
								  uint8* pValue, uint16 len, uint16 offset );
static void fastpairNotifyCB( linkDBItem_t* pLinkItem );
//static void fastpairNotifyLevel( void );

/*********************************************************************
	PROFILE CALLBACKS
*/
// FastPairery Service Callbacks
CONST gattServiceCBs_t fastpairCBs =
{
	fastpairReadAttrCB,	 // Read callback function pointer
	fastpairWriteAttrCB, // Write callback function pointer
	NULL			 // Authorization callback function pointer
};

/*********************************************************************
	PUBLIC FUNCTIONS
*/

/*********************************************************************
	@fn		 FastPair_AddService

	@brief	 Initializes the FastPairery Service by registering
			GATT attributes with the GATT server.

	@return	 Success or Failure
*/
bStatus_t FastPair_AddService( void )
{
	uint8 status = SUCCESS;
	// Initialize Client Characteristic Configuration attributes
	GATTServApp_InitCharCfg( INVALID_CONNHANDLE, fastpairLevelClientCharCfg );
	// Register GATT attribute list and CBs with GATT Server App
	status = GATTServApp_RegisterService( fastpairAttrTbl,
										  GATT_NUM_ATTRS( fastpairAttrTbl ),
										  &fastpairCBs );
	return ( status );
}

/*********************************************************************
	@fn		 FastPair_Register

	@brief	 Register a callback function with the FastPairery Service.

	@param	 pfnServiceCB - Callback function.

	@return	 None.
*/
extern void FastPair_Register( fastpairServiceCB_t pfnServiceCB )
{
	fastpairServiceCB = pfnServiceCB;
}
extern uint8 data[100];
/*********************************************************************
	@fn			 fastpairReadAttrCB

	@brief		 Read an attribute.

	@param		 connHandle - connection message was received on
	@param		 pAttr - pointer to attribute
	@param		 pValue - pointer to data to be read
	@param		 pLen - length of data to be read
	@param		 offset - offset of the first octet to be read
	@param		 maxLen - maximum length of data to be read

	@return		 Success or Failure
*/
static uint8 fastpairReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
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

	//uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1] );
    for (size_t i = 0; i < pAttr->type.len; i++)
    {
        dbg_printf(" %x",pAttr->type.uuid[i]);
    }    

	// Measure fastpairery level if reading level
	if ( memcmp(pAttr->type.uuid , fastpairLevelUUID,16)==0 )
	{	

        pValue[0] = 0x01;
		*pLen = 9;
		memcpy(pValue+1,google_findmy_key,8);
		for (size_t i = 0; i < 21; i++)
		{
			LUCA_LOG(",%x",pValue[i]);
		}    
		LUCA_LOG(" fastpairReadAttrCB(%d)  uuid:%x\n",pAttr->type.len,pAttr->type.uuid[12]);
	
	}
	

	return ( status );
}

/*********************************************************************
	@fn		 fastpairWriteAttrCB

	@brief	 Validate attribute data prior to a write operation

	@param	 connHandle - connection message was received on
	@param	 pAttr - pointer to attribute
	@param	 pValue - pointer to data to be written
	@param	 len - length of data
	@param	 offset - offset of the first octet to be written

	@return	 Success or Failure
*/
static bStatus_t fastpairWriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
								  uint8* pValue, uint16 len, uint16 offset )
{
	bStatus_t status = SUCCESS;
	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    dbg_printf("fastpairWriteAttrCB uuid:%x\n",uuid);

    uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );
    memcpy(data,pValue,len);
    if (pValue[0]==5)
    {
        pwm_buzzer_start();
    }

    dbg_printf("pValue(%d-%d):\n",len,offset);
    for (size_t i = 0; i < len; i++)
    {
        dbg_printf(" %x",pValue[i]);
    }
    dbg_printf("\n");
    
    switch ( uuid )
	{
	case GATT_CLIENT_CHAR_CFG_UUID:
		status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
												 offset, GATT_CLIENT_CFG_NOTIFY );

		if ( status == SUCCESS )
		{
            
			if ( fastpairServiceCB )
			{
				(*fastpairServiceCB)( (charCfg == GATT_CFG_NO_OPERATION) ?
								  FASTPAIR_LEVEL_NOTI_DISABLED :
								  FASTPAIR_LEVEL_NOTI_ENABLED);
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
	@fn			 fastpairNotifyCB

	@brief		 Send a notification of the level state characteristic.

	@param		 connHandle - linkDB item

	@return		 None.
*/
static void fastpairNotifyCB( linkDBItem_t* pLinkItem )
{
	if ( pLinkItem->stateFlags & LINK_CONNECTED )
	{
		uint16 value = GATTServApp_ReadCharCfg( pLinkItem->connectionHandle,
												fastpairLevelClientCharCfg );

		if ( value & GATT_CLIENT_CFG_NOTIFY )
		{
			attHandleValueNoti_t noti;
			noti.handle = fastpairAttrTbl[FASTPAIR_LEVEL_VALUE_IDX].handle;
			noti.len = 1;
			noti.value[0] = 0;
			GATT_Notification( pLinkItem->connectionHandle, &noti, FALSE );
		}
	}
}

/*********************************************************************
	@fn		 fastpairNotifyLevelState

	@brief	 Send a notification of the fastpairery level state
			characteristic if a connection is established.

	@return	 None.
*/
void FastPairNotifyLevel( void )
{
	// Execute linkDB callback to send notification
	linkDB_PerformFunc( fastpairNotifyCB );
}

/*********************************************************************
	@fn			 FastPair_HandleConnStatusCB

	@brief		 FastPairery Service link status change handler function.

	@param		 connHandle - connection handle
	@param		 changeType - type of change

	@return		 none
*/
void FastPair_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
	// Make sure this is not loopback connection
	if ( connHandle != LOOPBACK_CONNHANDLE )
	{
		// Reset Client Char Config if connection has dropped
		if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )		 ||
				( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
				  ( !linkDB_Up( connHandle ) ) ) )
		{
			GATTServApp_InitCharCfg( connHandle, fastpairLevelClientCharCfg );
		}
	}
}


/*********************************************************************
*********************************************************************/
