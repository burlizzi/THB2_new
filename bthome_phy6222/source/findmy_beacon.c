/*
 * findmy_beacon.c
 *
 *  Created on: 8 дек. 2024 г.
 *      Author: pvvx
 */
#include "log.h"

#include "rom_sym_def.h"
#include "types.h"
#include "config.h"
#include "uECC.h"
#include "aes.h"
#include "uECC_vli.h"


#if (DEV_SERVICES & SERVICE_FINDMY)
#include "findmy_beacon.h"
#include "sensors.h"
#include "thb2_main.h"
#include "flash_eep.h"


uint8_t apple_findmy_key[SIZE_APPLE_FINDMY_KEY];
uint8_t apple_findmy_key_new[SIZE_APPLE_FINDMY_KEY];


uint8_t google_findmy_key[SIZE_GOOGLE_FINDMY_KEY];
uint8_t google_findmy_key_new[SIZE_GOOGLE_FINDMY_KEY];

uint8_t identity_findmy_key[SIZE_IDENTITY_FINDMY_KEY];
uint8_t identity_findmy_key_new[SIZE_IDENTITY_FINDMY_KEY];



typedef struct __attribute__((packed)) _adv_google_bthome_noencrypt_t {
	uint8_t		head[8];
	uint8_t		key[SIZE_GOOGLE_FINDMY_KEY];
	uint8_t		state;
} adv_google_findmy_t, * padv_google_findmy_t;


typedef struct __attribute__((packed)) _adv_bthome_noencrypt_t {
	uint8_t		head[6];
	uint8_t		state;
/* State:
 Bits 0—1: Reserved.
 Bit  2: Maintained
 Bits 3—4: Reserved
 Bits 5: 0b1
 Bits 6—7: Battery state:
  0 = Full
  1 = Medium
  2 = Low
  3 = Critically low
 */
	uint8_t		key[22];
	uint8_t		key0_bit67;
	uint8_t 	end;
} adv_findmy_t, * padv_findmy_t;


static const uint8 google_findmy_head[] = {
	0x02,   // Length
	0x01,   // Flags data type value
	0x06,   // Flags data
	0x19,   // Length
	0x16,   // Service data data type value
	0xAA,   // 16-bit service UUID
	0xFE,   // 16-bit service UUID
	0x41   // FMDN frame type with unwanted tracking protection mode indication
};



static const uint8 apple_findmy_head[] = {
	0x1e, /* Length (30) */
	0xff, /* Manufacturer Specific Data (type 0xff) */
	0x4c, 0x00, /* Company ID (Apple) */
	0x12, 0x19 /* Offline Finding type and length */
};

void set_addr_from_key(uint8_t * addr, uint8_t *public_key)
{
    addr[5] = public_key[0] | 0b11000000;
    addr[4] = public_key[1];
    addr[3] = public_key[2];
    addr[2] = public_key[3];
    addr[1] = public_key[4];
    addr[0] = public_key[5];
}

uint8_t apple_findmy_beacon(void * padbuf) {
	//dbg_printf("apple\n");
	set_addr_from_key(ownPublicAddr,apple_findmy_key);
	padv_findmy_t p = (padv_findmy_t)padbuf;
	memcpy(p->head, apple_findmy_head, sizeof(apple_findmy_head));
	memcpy(p->key, &apple_findmy_key[6], sizeof(p->key));
	p->key0_bit67 = apple_findmy_key[0] >> 6;
	if(measured_data.battery > 80)
		p->state = 0 << 6; // Full
	else if(measured_data.battery > 60)
		p->state = 1 << 6; // Medium
	else if(measured_data.battery > 25)
		p->state = 2 << 6; // Low
	else
		p->state = 3 << 6; // Critically low
	p->end = 0;
	return sizeof(adv_findmy_t);
}


typedef struct __attribute__((packed)) _ephemeral_id_t {
	uint8_t		header1[11];
	uint8_t		exponent1;
	uint8_t		time1[4];
	uint8_t		header2[11];
	uint8_t		exponent2;
	uint8_t		time2[4];
} ephemeral_id_t, * pephemeral_id_t;

#undef LOG
#define LOG(...) log_printf(__VA_ARGS__)
uint32_t counter=0;
uint8_t generate_eid(uint8_t* key)
{	
	uint32_t activation_date=*(uint32_t*)(&identity_findmy_key[32]);
	
	counter=clkt.utc_time_sec-activation_date;
	//uint64_t mask = ~((1 << 1024) - 1);
    counter &= 0xFFFFFFFFFFFFFC00;
	LOG("counter %lu\n",counter);
	LOG("activation_date %lu\n",activation_date);

	uint8_t* c=&counter;
	ephemeral_id_t data={
		.header1={ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
		.exponent1 = 10,
		.time1={c[3],c[2],c[1],c[0]},
		.header2={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		.exponent2 = 10,
		.time2={c[3],c[2],c[1],c[0]},
	};
    struct AES_ctx ctx;
	for (uint8_t i = 0; i < 36; i++)
    {
        LOG("%02x",identity_findmy_key[i]);
    }
	LOG("\n");
	for (uint8_t i = 0; i < 32; i++)
    {
        LOG("%02x",((uint8_t*)&data)[i]);
    }
	LOG("\n");

	AES_init_ctx(&ctx,identity_findmy_key);
	AES_CBC_encrypt_buffer(&ctx,(uint8_t*)&data,32);
	for (uint8_t i = 0; i < 32; i++)
    {
        LOG("%02x",((uint8_t*)&data)[i]);
    }
	LOG("\n");

	uECC_Curve curve=uECC_secp160r1();
	uint8_t data1[64]={0};
	for (size_t i = 0; i < 32; i++)
	{
		data1[31-i]=((uint8_t*)&data)[i];
	}
	uint8_t r[32]={0};

	uECC_vli_mmod(r,data1,uECC_curve_n(curve),32/uECC_WORD_SIZE);
	for (size_t i = 0; i < 20; i++)
	{
		LOG("%02x",r[i]);
	}

	LOG("->trovato\n");

	if(EccPoint_compute_public_key(r,r,curve)==0)
		LOG("errore\n");

	for (uint8_t i = 0; i < 20; i++)
    {
        LOG("%02x",r[19-i]);
    }
	LOG("\n");
	for (size_t i = 0; i < 20; i++)
	{
		data1[19-i]=r[i];
	}


	memcpy(key,data1,SIZE_GOOGLE_FINDMY_KEY);
	AES_init_ctx(&ctx,identity_findmy_key);
	AES_CBC_encrypt_buffer(&ctx,(uint8_t*)&r,32);
	return r[0];
}


uint32_t next_time=0;

uint8_t google_rolling_findmy_beacon(void * padbuf) {
		padv_google_findmy_t p = (padv_google_findmy_t)padbuf;
	dbg_printf("roll\n");
	if (clkt.utc_time_sec>next_time)
	{

		p->state = 0;
		if(measured_data.battery > 80)
			p->state = 0 << 5; // Full
		else if(measured_data.battery > 60)
			p->state = 1 << 5; // Medium
		else if(measured_data.battery > 25)
			p->state = 2 << 5; // Low
		else
			p->state = 3 << 5; // Critically low

			p->state|=128;
		p->state  ^= generate_eid(google_findmy_key);
		next_time=clkt.utc_time_sec+1024;
	}

	dbg_printf("done, %u %lu\n",counter,clkt.utc_time_sec);
}
uint8_t google_findmy_beacon(void * padbuf) {
	//dbg_printf("google %d\n",measured_data.motion);
	if(flash_read_cfg(ownPublicAddr, EEP_ID_MAC, MAC_LEN) != MAC_LEN) 
		LUCA_LOG("error reading mac\n");
	padv_google_findmy_t p = (padv_google_findmy_t)padbuf;
	memcpy(p->head, google_findmy_head, sizeof(google_findmy_head));
	memcpy(p->key, google_findmy_key, sizeof(p->key));
	return sizeof(adv_google_findmy_t);
}
#endif	// USE_FINDMY
