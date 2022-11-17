////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ _   //
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __| |  //
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |  | |  //
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |  | |  //
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |  |_|  //
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|  (_)  //
//                                                                        //
//     WARNING! This is an automatically generated file.  DO NOT EDIT     //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GCUSettingsParser.h"

struct GCUSettingsPacket* ParseGCUSettingsPacket(uint8_t* RawData) {
  uint16_t byteIndex = 0;
  uint8_t bitIndex = 7;
  struct GCUSettingsPacket* packet;
  
  packet = MakeNewGCUSettingsPacket();
  ParseAllocatedGCUSettingsPacket(RawData, packet);
  return packet;
}

void ParseGCUSettingsPacketWrapper(uint8_t* RawData, void* packet_bytes) {
  ParseAllocatedGCUSettingsPacket(RawData, (struct GCUSettingsPacket*)packet_bytes);
}

void ParseAllocatedGCUSettingsPacket(uint8_t* RawData, struct GCUSettingsPacket* packet) {
  uint16_t i;
  packet->Sync = (((uint16_t)RawData[0]) << 8) | ((uint16_t)RawData[1]);
  packet->PacketID = ((uint8_t)RawData[2]);
  packet->UnixTime = (((uint32_t)RawData[3]) << 16) | (((uint32_t)RawData[4]) << 8) | ((uint32_t)RawData[5]);
  packet->PacketCounter = (((uint16_t)RawData[6]) << 8) | ((uint16_t)RawData[7]);
  packet->PacketSize = (((uint16_t)RawData[8]) << 8) | ((uint16_t)RawData[9]);
  packet->OPAHeartBeatTimeout = (((uint16_t)RawData[10]) << 8) | ((uint16_t)RawData[11]);
  packet->OPBHeartBeatTimeout = (((uint16_t)RawData[12]) << 8) | ((uint16_t)RawData[13]);
  packet->LOSMaxQueueSize = (((uint32_t)RawData[14]) << 24) | (((uint32_t)RawData[15]) << 16) | (((uint32_t)RawData[16]) << 8) | ((uint32_t)RawData[17]);
  packet->DroppedMaxQueueSize = (((uint32_t)RawData[18]) << 24) | (((uint32_t)RawData[19]) << 16) | (((uint32_t)RawData[20]) << 8) | ((uint32_t)RawData[21]);
  packet->RelayMaxQueueSize = (((uint32_t)RawData[22]) << 24) | (((uint32_t)RawData[23]) << 16) | (((uint32_t)RawData[24]) << 8) | ((uint32_t)RawData[25]);
  packet->DiskMaxQueueSize = (((uint32_t)RawData[26]) << 24) | (((uint32_t)RawData[27]) << 16) | (((uint32_t)RawData[28]) << 8) | ((uint32_t)RawData[29]);
  packet->DiskMaxFileSize = (((uint32_t)RawData[30]) << 24) | (((uint32_t)RawData[31]) << 16) | (((uint32_t)RawData[32]) << 8) | ((uint32_t)RawData[33]);
  packet->RTAMaxQueueSize = (((uint32_t)RawData[34]) << 24) | (((uint32_t)RawData[35]) << 16) | (((uint32_t)RawData[36]) << 8) | ((uint32_t)RawData[37]);
  packet->RTAMaxComptonQueueSize = (((uint32_t)RawData[38]) << 24) | (((uint32_t)RawData[39]) << 16) | (((uint32_t)RawData[40]) << 8) | ((uint32_t)RawData[41]);
  packet->RTAMaxNumTriggers = (((uint16_t)RawData[42]) << 8) | ((uint16_t)RawData[43]);
  packet->RTAComptonWindow = ((uint8_t)RawData[44]);
  packet->GRBMaxQueueSize = (((uint32_t)RawData[45]) << 24) | (((uint32_t)RawData[46]) << 16) | (((uint32_t)RawData[47]) << 8) | ((uint32_t)RawData[48]);
  packet->GRBQueueTDiff = (((uint32_t)RawData[49]) << 24) | (((uint32_t)RawData[50]) << 16) | (((uint32_t)RawData[51]) << 8) | ((uint32_t)RawData[52]);
  packet->ShieldSampleRate100ms = (((uint16_t)RawData[53]) << 8) | ((uint16_t)RawData[54]);
  packet->HkpShieldInterval = (((uint16_t)RawData[55]) << 8) | ((uint16_t)RawData[56]);
  packet->ShieldAlgoInterval = (uint16_t*)malloc(3*sizeof(uint16_t));
  packet->ShieldTriggerSigma = (uint16_t*)malloc(3*sizeof(uint16_t));
  packet->GRBAlgoMinSamples = (uint16_t*)malloc(3*sizeof(uint16_t));
  packet->GRBAlgoMaxSamples = (uint16_t*)malloc(3*sizeof(uint16_t));
  algo : for (i = 0; i < 3; i += 1) {
    packet->ShieldAlgoInterval[i] = (((uint16_t)RawData[57 + 8*i + 0]) << 8) | ((uint16_t)RawData[57 + 8*i + 1]);
    packet->ShieldTriggerSigma[i] = (((uint16_t)RawData[57 + 8*i + 2]) << 8) | ((uint16_t)RawData[57 + 8*i + 3]);
    packet->GRBAlgoMinSamples[i] = (((uint16_t)RawData[57 + 8*i + 4]) << 8) | ((uint16_t)RawData[57 + 8*i + 5]);
    packet->GRBAlgoMaxSamples[i] = (((uint16_t)RawData[57 + 8*i + 6]) << 8) | ((uint16_t)RawData[57 + 8*i + 7]);
  }
  packet->GPSSampleRate100ms = (((uint16_t)RawData[81]) << 8) | ((uint16_t)RawData[82]);
  packet->GPSDiagInterval = (((uint16_t)RawData[83]) << 8) | ((uint16_t)RawData[84]);
  packet->HkpPeriod100ms = (((uint16_t)RawData[85]) << 8) | ((uint16_t)RawData[86]);
  packet->CryoPID = ((uint8_t)RawData[87]);
  packet->CryoTTarget = (((uint16_t)RawData[88]) << 8) | ((uint16_t)RawData[89]);
  packet->CryoPWOut = (((uint16_t)RawData[90]) << 8) | ((uint16_t)RawData[91]);
  packet->CryoMax = (((uint16_t)RawData[92]) << 8) | ((uint16_t)RawData[93]);
  packet->OPAUDPByteLimit = (((uint16_t)RawData[94]) << 8) | ((uint16_t)RawData[95]);
  packet->OPBUDPByteLimit = (((uint16_t)RawData[96]) << 8) | ((uint16_t)RawData[97]);
  packet->RpiTemp_Brd0_Ch0 = (((uint16_t)RawData[98]) << 8) | ((uint16_t)RawData[99]);
  packet->RpiTemp_Brd0_Ch1 = (((uint16_t)RawData[100]) << 8) | ((uint16_t)RawData[101]);
  packet->RpiTemp_Brd0_Ch2 = (((uint16_t)RawData[102]) << 8) | ((uint16_t)RawData[103]);
  packet->RpiTemp_Brd0_Ch3 = (((uint16_t)RawData[104]) << 8) | ((uint16_t)RawData[105]);
  packet->RpiTemp_Brd0_Ch4 = (((uint16_t)RawData[106]) << 8) | ((uint16_t)RawData[107]);
  packet->RpiTemp_Brd0_Ch5 = (((uint16_t)RawData[108]) << 8) | ((uint16_t)RawData[109]);
  packet->RpiTemp_Brd0_Ch6 = (((uint16_t)RawData[110]) << 8) | ((uint16_t)RawData[111]);
  packet->RpiTemp_Brd0_Ch7 = (((uint16_t)RawData[112]) << 8) | ((uint16_t)RawData[113]);
  packet->RpiTemp_Brd1_Ch0 = (((uint16_t)RawData[114]) << 8) | ((uint16_t)RawData[115]);
  packet->RpiTemp_Brd1_Ch1 = (((uint16_t)RawData[116]) << 8) | ((uint16_t)RawData[117]);
  packet->RpiTemp_Brd1_Ch2 = (((uint16_t)RawData[118]) << 8) | ((uint16_t)RawData[119]);
  packet->RpiTemp_Brd1_Ch3 = (((uint16_t)RawData[120]) << 8) | ((uint16_t)RawData[121]);
  packet->RpiTemp_Brd1_Ch4 = (((uint16_t)RawData[122]) << 8) | ((uint16_t)RawData[123]);
  packet->RpiTemp_Brd1_Ch5 = (((uint16_t)RawData[124]) << 8) | ((uint16_t)RawData[125]);
  packet->RpiTemp_Brd1_Ch6 = (((uint16_t)RawData[126]) << 8) | ((uint16_t)RawData[127]);
  packet->RpiTemp_Brd1_Ch7 = (((uint16_t)RawData[128]) << 8) | ((uint16_t)RawData[129]);
  packet->RpiTemp_Brd2_Ch0 = (((uint16_t)RawData[130]) << 8) | ((uint16_t)RawData[131]);
  packet->RpiTemp_Brd2_Ch1 = (((uint16_t)RawData[132]) << 8) | ((uint16_t)RawData[133]);
  packet->RpiTemp_Brd2_Ch2 = (((uint16_t)RawData[134]) << 8) | ((uint16_t)RawData[135]);
  packet->RpiTemp_Brd2_Ch3 = (((uint16_t)RawData[136]) << 8) | ((uint16_t)RawData[137]);
  packet->RpiTemp_Brd2_Ch4 = (((uint16_t)RawData[138]) << 8) | ((uint16_t)RawData[139]);
  packet->RpiTemp_Brd2_Ch5 = (((uint16_t)RawData[140]) << 8) | ((uint16_t)RawData[141]);
  packet->RpiTemp_Brd2_Ch6 = (((uint16_t)RawData[142]) << 8) | ((uint16_t)RawData[143]);
  packet->RpiTemp_Brd2_Ch7 = (((uint16_t)RawData[144]) << 8) | ((uint16_t)RawData[145]);
  packet->RpiTemp_Brd3_Ch0 = (((uint16_t)RawData[146]) << 8) | ((uint16_t)RawData[147]);
  packet->RpiTemp_Brd3_Ch1 = (((uint16_t)RawData[148]) << 8) | ((uint16_t)RawData[149]);
  packet->RpiTemp_Brd3_Ch2 = (((uint16_t)RawData[150]) << 8) | ((uint16_t)RawData[151]);
  packet->RpiTemp_Brd3_Ch3 = (((uint16_t)RawData[152]) << 8) | ((uint16_t)RawData[153]);
  packet->RpiTemp_Brd3_Ch4 = (((uint16_t)RawData[154]) << 8) | ((uint16_t)RawData[155]);
  packet->RpiTemp_Brd3_Ch5 = (((uint16_t)RawData[156]) << 8) | ((uint16_t)RawData[157]);
  packet->RpiTemp_Brd3_Ch6 = (((uint16_t)RawData[158]) << 8) | ((uint16_t)RawData[159]);
  packet->RpiTemp_Brd3_Ch7 = (((uint16_t)RawData[160]) << 8) | ((uint16_t)RawData[161]);
  packet->MagPiX = (((uint16_t)RawData[162]) << 8) | ((uint16_t)RawData[163]);
  packet->MagPiY = (((uint16_t)RawData[164]) << 8) | ((uint16_t)RawData[165]);
  packet->MagPiZ = (((uint16_t)RawData[166]) << 8) | ((uint16_t)RawData[167]);
  packet->TStat0LowerTemp = (((uint16_t)RawData[168]) << 8) | ((uint16_t)RawData[169]);
  packet->TStat0UpperTemp = (((uint16_t)RawData[170]) << 8) | ((uint16_t)RawData[171]);
  packet->TStat1LowerTemp = (((uint16_t)RawData[172]) << 8) | ((uint16_t)RawData[173]);
  packet->TStat1UpperTemp = (((uint16_t)RawData[174]) << 8) | ((uint16_t)RawData[175]);
  packet->TStat2LowerTemp = (((uint16_t)RawData[176]) << 8) | ((uint16_t)RawData[177]);
  packet->TStat2UpperTemp = (((uint16_t)RawData[178]) << 8) | ((uint16_t)RawData[179]);
  packet->TStat3LowerTemp = (((uint16_t)RawData[180]) << 8) | ((uint16_t)RawData[181]);
  packet->TStat3UpperTemp = (((uint16_t)RawData[182]) << 8) | ((uint16_t)RawData[183]);
  packet->TStat4LowerTemp = (((uint16_t)RawData[184]) << 8) | ((uint16_t)RawData[185]);
  packet->TStat4UpperTemp = (((uint16_t)RawData[186]) << 8) | ((uint16_t)RawData[187]);
  packet->TStat5LowerTemp = (((uint16_t)RawData[188]) << 8) | ((uint16_t)RawData[189]);
  packet->TStat5UpperTemp = (((uint16_t)RawData[190]) << 8) | ((uint16_t)RawData[191]);
  packet->TStat6LowerTemp = (((uint16_t)RawData[192]) << 8) | ((uint16_t)RawData[193]);
  packet->TStat6UpperTemp = (((uint16_t)RawData[194]) << 8) | ((uint16_t)RawData[195]);
  packet->TStat7LowerTemp = (((uint16_t)RawData[196]) << 8) | ((uint16_t)RawData[197]);
  packet->TStat7UpperTemp = (((uint16_t)RawData[198]) << 8) | ((uint16_t)RawData[199]);
}

struct GCUSettingsPacket* MakeNewGCUSettingsPacket(void) { 
  uint8_t i = 0;
  struct GCUSettingsPacket* packet = (struct GCUSettingsPacket*) malloc (sizeof(struct GCUSettingsPacket));
  memset(packet, 0, sizeof(struct GCUSettingsPacket));
  return packet;
}

void FreeAllocatedGCUSettingsPacket(struct GCUSettingsPacket* packet) {
  uint16_t i;


  free(packet->ShieldAlgoInterval);
  free(packet->ShieldTriggerSigma);
  free(packet->GRBAlgoMinSamples);
  free(packet->GRBAlgoMaxSamples);
}


void printGCUSettingsPacket(struct GCUSettingsPacket* packet) {
  uint16_t i;

  printf("packet->Sync = 0x%04x\n", packet->Sync);
  printf("packet->PacketID = 0x%02x\n", packet->PacketID);
  printf("packet->UnixTime = 0x%06x\n", packet->UnixTime);
  printf("packet->PacketCounter = 0x%04x\n", packet->PacketCounter);
  printf("packet->PacketSize = 0x%04x\n", packet->PacketSize);
  printf("packet->OPAHeartBeatTimeout = 0x%04x\n", packet->OPAHeartBeatTimeout);
  printf("packet->OPBHeartBeatTimeout = 0x%04x\n", packet->OPBHeartBeatTimeout);
  printf("packet->LOSMaxQueueSize = 0x%08x\n", packet->LOSMaxQueueSize);
  printf("packet->DroppedMaxQueueSize = 0x%08x\n", packet->DroppedMaxQueueSize);
  printf("packet->RelayMaxQueueSize = 0x%08x\n", packet->RelayMaxQueueSize);
  printf("packet->DiskMaxQueueSize = 0x%08x\n", packet->DiskMaxQueueSize);
  printf("packet->DiskMaxFileSize = 0x%08x\n", packet->DiskMaxFileSize);
  printf("packet->RTAMaxQueueSize = 0x%08x\n", packet->RTAMaxQueueSize);
  printf("packet->RTAMaxComptonQueueSize = 0x%08x\n", packet->RTAMaxComptonQueueSize);
  printf("packet->RTAMaxNumTriggers = 0x%04x\n", packet->RTAMaxNumTriggers);
  printf("packet->RTAComptonWindow = 0x%02x\n", packet->RTAComptonWindow);
  printf("packet->GRBMaxQueueSize = 0x%08x\n", packet->GRBMaxQueueSize);
  printf("packet->GRBQueueTDiff = 0x%08x\n", packet->GRBQueueTDiff);
  printf("packet->ShieldSampleRate100ms = 0x%04x\n", packet->ShieldSampleRate100ms);
  printf("packet->HkpShieldInterval = 0x%04x\n", packet->HkpShieldInterval);
  algo : for (i = 0; i < 3; i += 1) {
    printf("packet->ShieldAlgoInterval[%0d] = 0x%04x\n", i, packet->ShieldAlgoInterval[i]);
    printf("packet->ShieldTriggerSigma[%0d] = 0x%04x\n", i, packet->ShieldTriggerSigma[i]);
    printf("packet->GRBAlgoMinSamples[%0d] = 0x%04x\n", i, packet->GRBAlgoMinSamples[i]);
    printf("packet->GRBAlgoMaxSamples[%0d] = 0x%04x\n", i, packet->GRBAlgoMaxSamples[i]);
  }
  printf("packet->GPSSampleRate100ms = 0x%04x\n", packet->GPSSampleRate100ms);
  printf("packet->GPSDiagInterval = 0x%04x\n", packet->GPSDiagInterval);
  printf("packet->HkpPeriod100ms = 0x%04x\n", packet->HkpPeriod100ms);
  printf("packet->CryoPID = 0x%02x\n", packet->CryoPID);
  printf("packet->CryoTTarget = 0x%04x\n", packet->CryoTTarget);
  printf("packet->CryoPWOut = 0x%04x\n", packet->CryoPWOut);
  printf("packet->CryoMax = 0x%04x\n", packet->CryoMax);
  printf("packet->OPAUDPByteLimit = 0x%04x\n", packet->OPAUDPByteLimit);
  printf("packet->OPBUDPByteLimit = 0x%04x\n", packet->OPBUDPByteLimit);
  printf("packet->RpiTemp_Brd0_Ch0 = 0x%04x\n", packet->RpiTemp_Brd0_Ch0);
  printf("packet->RpiTemp_Brd0_Ch1 = 0x%04x\n", packet->RpiTemp_Brd0_Ch1);
  printf("packet->RpiTemp_Brd0_Ch2 = 0x%04x\n", packet->RpiTemp_Brd0_Ch2);
  printf("packet->RpiTemp_Brd0_Ch3 = 0x%04x\n", packet->RpiTemp_Brd0_Ch3);
  printf("packet->RpiTemp_Brd0_Ch4 = 0x%04x\n", packet->RpiTemp_Brd0_Ch4);
  printf("packet->RpiTemp_Brd0_Ch5 = 0x%04x\n", packet->RpiTemp_Brd0_Ch5);
  printf("packet->RpiTemp_Brd0_Ch6 = 0x%04x\n", packet->RpiTemp_Brd0_Ch6);
  printf("packet->RpiTemp_Brd0_Ch7 = 0x%04x\n", packet->RpiTemp_Brd0_Ch7);
  printf("packet->RpiTemp_Brd1_Ch0 = 0x%04x\n", packet->RpiTemp_Brd1_Ch0);
  printf("packet->RpiTemp_Brd1_Ch1 = 0x%04x\n", packet->RpiTemp_Brd1_Ch1);
  printf("packet->RpiTemp_Brd1_Ch2 = 0x%04x\n", packet->RpiTemp_Brd1_Ch2);
  printf("packet->RpiTemp_Brd1_Ch3 = 0x%04x\n", packet->RpiTemp_Brd1_Ch3);
  printf("packet->RpiTemp_Brd1_Ch4 = 0x%04x\n", packet->RpiTemp_Brd1_Ch4);
  printf("packet->RpiTemp_Brd1_Ch5 = 0x%04x\n", packet->RpiTemp_Brd1_Ch5);
  printf("packet->RpiTemp_Brd1_Ch6 = 0x%04x\n", packet->RpiTemp_Brd1_Ch6);
  printf("packet->RpiTemp_Brd1_Ch7 = 0x%04x\n", packet->RpiTemp_Brd1_Ch7);
  printf("packet->RpiTemp_Brd2_Ch0 = 0x%04x\n", packet->RpiTemp_Brd2_Ch0);
  printf("packet->RpiTemp_Brd2_Ch1 = 0x%04x\n", packet->RpiTemp_Brd2_Ch1);
  printf("packet->RpiTemp_Brd2_Ch2 = 0x%04x\n", packet->RpiTemp_Brd2_Ch2);
  printf("packet->RpiTemp_Brd2_Ch3 = 0x%04x\n", packet->RpiTemp_Brd2_Ch3);
  printf("packet->RpiTemp_Brd2_Ch4 = 0x%04x\n", packet->RpiTemp_Brd2_Ch4);
  printf("packet->RpiTemp_Brd2_Ch5 = 0x%04x\n", packet->RpiTemp_Brd2_Ch5);
  printf("packet->RpiTemp_Brd2_Ch6 = 0x%04x\n", packet->RpiTemp_Brd2_Ch6);
  printf("packet->RpiTemp_Brd2_Ch7 = 0x%04x\n", packet->RpiTemp_Brd2_Ch7);
  printf("packet->RpiTemp_Brd3_Ch0 = 0x%04x\n", packet->RpiTemp_Brd3_Ch0);
  printf("packet->RpiTemp_Brd3_Ch1 = 0x%04x\n", packet->RpiTemp_Brd3_Ch1);
  printf("packet->RpiTemp_Brd3_Ch2 = 0x%04x\n", packet->RpiTemp_Brd3_Ch2);
  printf("packet->RpiTemp_Brd3_Ch3 = 0x%04x\n", packet->RpiTemp_Brd3_Ch3);
  printf("packet->RpiTemp_Brd3_Ch4 = 0x%04x\n", packet->RpiTemp_Brd3_Ch4);
  printf("packet->RpiTemp_Brd3_Ch5 = 0x%04x\n", packet->RpiTemp_Brd3_Ch5);
  printf("packet->RpiTemp_Brd3_Ch6 = 0x%04x\n", packet->RpiTemp_Brd3_Ch6);
  printf("packet->RpiTemp_Brd3_Ch7 = 0x%04x\n", packet->RpiTemp_Brd3_Ch7);
  printf("packet->MagPiX = 0x%04x\n", packet->MagPiX);
  printf("packet->MagPiY = 0x%04x\n", packet->MagPiY);
  printf("packet->MagPiZ = 0x%04x\n", packet->MagPiZ);
  printf("packet->TStat0LowerTemp = 0x%04x\n", packet->TStat0LowerTemp);
  printf("packet->TStat0UpperTemp = 0x%04x\n", packet->TStat0UpperTemp);
  printf("packet->TStat1LowerTemp = 0x%04x\n", packet->TStat1LowerTemp);
  printf("packet->TStat1UpperTemp = 0x%04x\n", packet->TStat1UpperTemp);
  printf("packet->TStat2LowerTemp = 0x%04x\n", packet->TStat2LowerTemp);
  printf("packet->TStat2UpperTemp = 0x%04x\n", packet->TStat2UpperTemp);
  printf("packet->TStat3LowerTemp = 0x%04x\n", packet->TStat3LowerTemp);
  printf("packet->TStat3UpperTemp = 0x%04x\n", packet->TStat3UpperTemp);
  printf("packet->TStat4LowerTemp = 0x%04x\n", packet->TStat4LowerTemp);
  printf("packet->TStat4UpperTemp = 0x%04x\n", packet->TStat4UpperTemp);
  printf("packet->TStat5LowerTemp = 0x%04x\n", packet->TStat5LowerTemp);
  printf("packet->TStat5UpperTemp = 0x%04x\n", packet->TStat5UpperTemp);
  printf("packet->TStat6LowerTemp = 0x%04x\n", packet->TStat6LowerTemp);
  printf("packet->TStat6UpperTemp = 0x%04x\n", packet->TStat6UpperTemp);
  printf("packet->TStat7LowerTemp = 0x%04x\n", packet->TStat7LowerTemp);
  printf("packet->TStat7UpperTemp = 0x%04x\n", packet->TStat7UpperTemp);
}

void getGCUSettingsPacketAllocatedSizes(struct GCUSettingsPacket* packet, uint16_t** sizesPtr) {
  uint16_t* sizes;
  
  sizes = (uint16_t*)malloc(4*sizeof(uint16_t));
  sizes[0] = (uint16_t)3;
  sizes[1] = (uint16_t)3;
  sizes[2] = (uint16_t)3;
  sizes[3] = (uint16_t)3;
  
  *sizesPtr = sizes;
}
