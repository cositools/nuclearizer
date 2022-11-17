/*
  2 Livetime Packet Parser
  3 
  4 Written by Clio Sleator
  5 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
 
 //****************** Declaring Structs to hold parsed data ******************//
 
struct LivetimePacket{
 
    //Livetime
	uint64_t ClkVal;
	uint16_t PacketCounter;
	uint32_t UnixTime;
	uint16_t PacketSize;
	uint16_t Sync;
	uint8_t PacketID;

    uint8_t CCHasLivetime[12];
    uint16_t TotalLivetime[12];
    uint16_t ShieldLivetime[12];

};
 
//declare functions
void ParseLivetime(struct LivetimePacket* packet, uint8_t* RawData);

