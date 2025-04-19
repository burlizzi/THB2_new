/*
 * findmy_beacon.h
 *
 *  Created on: 8 дек. 2024 г.
 *      Author: pvvx
 */

 #ifndef SOURCE_FINDMY_BEACON_H_
 #define SOURCE_FINDMY_BEACON_H_
 
 #define SIZE_APPLE_FINDMY_KEY		28
 
 extern uint8_t apple_findmy_key[SIZE_APPLE_FINDMY_KEY];
 extern uint8_t apple_findmy_key_new[SIZE_APPLE_FINDMY_KEY];
 
 #define SIZE_GOOGLE_FINDMY_KEY		20
 
 extern uint8_t google_findmy_key[SIZE_GOOGLE_FINDMY_KEY];
 extern uint8_t google_findmy_key_new[SIZE_GOOGLE_FINDMY_KEY];
 
 #define SIZE_IDENTITY_FINDMY_KEY    36
 
 extern uint8_t identity_findmy_key[SIZE_IDENTITY_FINDMY_KEY];
 extern uint8_t identity_findmy_key_new[SIZE_IDENTITY_FINDMY_KEY];
 
 
 uint8_t apple_findmy_beacon(void * padbuf);
 uint8_t google_rolling_findmy_beacon(void * padbuf);
 uint8_t google_findmy_beacon(void * padbuf);
 
 #endif /* SOURCE_FINDMY_BEACON_H_ */
 