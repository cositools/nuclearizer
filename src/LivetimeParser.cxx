/*
Livetime Packet Parser

Written by Clio Sleator
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "LivetimeParser.h"


//////////////////////////////////////////////////////////////////////////////////

void ParseLivetime(struct LivetimePacket* packet, uint8_t* RawData){

	int dx = 10;

	packet->PacketCounter = (((uint16_t)RawData[6]) << 8) | ((uint16_t)RawData[7]);
	packet->UnixTime = (((uint32_t)RawData[3]) << 16) | (((uint32_t)RawData[4]) << 8) | ((uint32_t)RawData[5]);
	packet->PacketSize = (((uint16_t)RawData[8]) << 8) | ((uint16_t)RawData[9]);
	packet->Sync = (((uint16_t)RawData[0]) << 8) | ((uint16_t)RawData[1]);
	packet->PacketID = ((uint8_t)RawData[2]);

	packet->ClkVal = (((uint64_t)RawData[dx]) << 40) | (((uint64_t)RawData[dx+1]) << 32) | (((uint64_t)RawData[dx+2]) << 24) | (((uint64_t)RawData[dx+3]) << 16) | (((uint64_t)RawData[dx+4]) << 16) | ((uint64_t)RawData[dx+5]);

	

	packet->CCHasLivetime[11] = ((uint8_t)RawData[dx+6] >> 3) & 0x01;
	packet->CCHasLivetime[10] = ((uint8_t)RawData[dx+6] >> 2) & 0x01;
	packet->CCHasLivetime[9] = ((uint8_t)RawData[dx+6] >> 1) & 0x01;
	packet->CCHasLivetime[8] = ((uint8_t)RawData[dx+6]) & 0x01;

	packet->CCHasLivetime[7] = ((uint8_t)RawData[dx+7] >> 7) & 0x01;
	packet->CCHasLivetime[6] = ((uint8_t)RawData[dx+7] >> 6) & 0x01;
	packet->CCHasLivetime[5] = ((uint8_t)RawData[dx+7] >> 5) & 0x01;
	packet->CCHasLivetime[4] = ((uint8_t)RawData[dx+7] >> 4) & 0x01;
	packet->CCHasLivetime[3] = ((uint8_t)RawData[dx+7] >> 3) & 0x01;
	packet->CCHasLivetime[2] = ((uint8_t)RawData[dx+7] >> 2) & 0x01;
	packet->CCHasLivetime[1] = ((uint8_t)RawData[dx+7] >> 1) & 0x01;
	packet->CCHasLivetime[0] = ((uint8_t)RawData[dx+7]) & 0x01;


	//find first high bit
	//order is CC8-CC11; CC0-CC7	
	int order[12] = {8,9,10,11,0,1,2,3,4,5,6,7};
	int i;
	int index = dx+8;
	for (i=0; i<12; i++){
		if (packet->CCHasLivetime[order[i]] == 1){
			packet->TotalLivetime[order[i]] = (((uint16_t)RawData[index]) << 8) | ((uint16_t)RawData[index+1]);
			packet->ShieldLivetime[order[i]] = (((uint16_t)RawData[index+2]) << 8) | ((uint16_t)RawData[index+3]);
			index += 4;
		}
		//else, put -1 (Want to but in NoneType, but this is C. Just feel weird having it be uninitialized)
		else {
			packet->TotalLivetime[order[i]] = -1;
			packet->ShieldLivetime[order[i]] = -1;
		}
	}

};

int main(){

	FILE *fp;
	uint8_t buffer[8000648];
	size_t Len;
	int buffer_index;
	int packet_counter = 0;

	fp = fopen("COSIa122914_084931.dat","r");

	Len = fread(buffer,1,8000648,fp);
	for (buffer_index=0; buffer_index < sizeof(buffer); buffer_index++){
		if (buffer[buffer_index] == 0xeb && buffer[buffer_index+1] == 0x90 && buffer[buffer_index+2] == 0x06){
			packet_counter += 1;
			struct LivetimePacket myPacket;
			ParseLivetime(&myPacket,&buffer[buffer_index]);
		}
	}

	printf("%d packets\n",packet_counter);
	printf("done!\n");

	return 0;
}

