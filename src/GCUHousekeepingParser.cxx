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
#include "GCUHousekeepingParser.h"

struct GCUHousekeepingPacket* ParseGCUHousekeepingPacket(uint8_t* RawData) {
  uint16_t byteIndex = 0;
  uint8_t bitIndex = 7;
  struct GCUHousekeepingPacket* packet;
  
  packet = MakeNewGCUHousekeepingPacket();
  ParseAllocatedGCUHousekeepingPacket(RawData, packet);
  return packet;
}

void ParseGCUHousekeepingPacketWrapper(uint8_t* RawData, void* packet_bytes) {
  ParseAllocatedGCUHousekeepingPacket(RawData, (struct GCUHousekeepingPacket*)packet_bytes);
}

void ParseAllocatedGCUHousekeepingPacket(uint8_t* RawData, struct GCUHousekeepingPacket* packet) {
  uint16_t i;
  uint16_t j;
  packet->Sync = (((uint16_t)RawData[0]) << 8) | ((uint16_t)RawData[1]);
  packet->PacketID = ((uint8_t)RawData[2]);
  packet->UnixTime = (((uint32_t)RawData[3]) << 16) | (((uint32_t)RawData[4]) << 8) | ((uint32_t)RawData[5]);
  packet->PacketCounter = (((uint16_t)RawData[6]) << 8) | ((uint16_t)RawData[7]);
  packet->PacketSize = (((uint16_t)RawData[8]) << 8) | ((uint16_t)RawData[9]);
  packet->Temp = (uint16_t*)malloc(32*sizeof(uint16_t));
  tempIter : for (i = 0; i < 32; i += 1) {
    packet->Temp[i] = (((uint16_t)RawData[10 + 2*i + 0]) << 8) | ((uint16_t)RawData[10 + 2*i + 1]);
  }
  packet->ADC = (uint16_t*)malloc(32*sizeof(uint16_t));
  for (j = 0; j <= 31; j += 1) {
    packet->ADC[j] = (((uint16_t)RawData[74 + 2*j + 0]) << 8) | ((uint16_t)RawData[74 + 2*j + 1]);
  }
  packet->CryoTipTemp = (((uint16_t)RawData[138]) << 8) | ((uint16_t)RawData[139]);
  packet->CryoPower = (((uint16_t)RawData[140]) << 8) | ((uint16_t)RawData[141]);
  packet->ClkVal = (((uint64_t)RawData[142]) << 40) | (((uint64_t)RawData[143]) << 32) | (((uint64_t)RawData[144]) << 24) | (((uint64_t)RawData[145]) << 16) | (((uint64_t)RawData[146]) << 8) | ((uint64_t)RawData[147]);
  packet->LastCommandIDs = (uint8_t*)malloc(3*sizeof(uint8_t));
  packet->LastCommandPayloads = (uint16_t*)malloc(3*sizeof(uint16_t));
  lastCommandsIter : for (i = 0; i < 3; i += 1) {
    packet->LastCommandIDs[i] = ((uint8_t)RawData[148 + 4*i + 1]);
    packet->LastCommandPayloads[i] = (((uint16_t)RawData[148 + 4*i + 2]) << 8) | ((uint16_t)RawData[148 + 4*i + 3]);
  }
  packet->NumGoodCommands = (((uint16_t)RawData[160]) << 8) | ((uint16_t)RawData[161]);
  packet->NumBadCommands = (((uint16_t)RawData[162]) << 8) | ((uint16_t)RawData[163]);
  packet->OPADataRate = ((uint8_t)RawData[164]);
  packet->OPBDataRate = ((uint8_t)RawData[165]);
  packet->NumAutoResyncs = ((uint8_t)RawData[166]);
  packet->HeaterBoxReadBack7 = ((uint8_t)((RawData[167] >> 7) & 0x01));
  packet->HeaterBoxReadBack6 = ((uint8_t)((RawData[167] >> 6) & 0x01));
  packet->HeaterBoxReadBack5 = ((uint8_t)((RawData[167] >> 5) & 0x01));
  packet->HeaterBoxReadBack4 = ((uint8_t)((RawData[167] >> 4) & 0x01));
  packet->HeaterBoxReadBack3 = ((uint8_t)((RawData[167] >> 3) & 0x01));
  packet->HeaterBoxReadBack2 = ((uint8_t)((RawData[167] >> 2) & 0x01));
  packet->HeaterBoxReadBack1 = ((uint8_t)((RawData[167] >> 1) & 0x01));
  packet->HeaterBoxReadBack0 = ((uint8_t)(RawData[167] & 0x01));
  packet->NumGCURestarts = (((uint16_t)RawData[168]) << 8) | ((uint16_t)RawData[169]);
  packet->NumBytesInRelayQueue = (((uint32_t)RawData[170]) << 24) | (((uint32_t)RawData[171]) << 16) | (((uint32_t)RawData[172]) << 8) | ((uint32_t)RawData[173]);
  packet->HeaterBoxSetting7 = ((uint8_t)((RawData[174] >> 7) & 0x01));
  packet->HeaterBoxSetting6 = ((uint8_t)((RawData[174] >> 6) & 0x01));
  packet->HeaterBoxSetting5 = ((uint8_t)((RawData[174] >> 5) & 0x01));
  packet->HeaterBoxSetting4 = ((uint8_t)((RawData[174] >> 4) & 0x01));
  packet->HeaterBoxSetting3 = ((uint8_t)((RawData[174] >> 3) & 0x01));
  packet->HeaterBoxSetting2 = ((uint8_t)((RawData[174] >> 2) & 0x01));
  packet->HeaterBoxSetting1 = ((uint8_t)((RawData[174] >> 1) & 0x01));
  packet->HeaterBoxSetting0 = ((uint8_t)(RawData[174] & 0x01));
  packet->TStat7Enabled = ((uint8_t)((RawData[175] >> 7) & 0x01));
  packet->TStat6Enabled = ((uint8_t)((RawData[175] >> 6) & 0x01));
  packet->TStat5Enabled = ((uint8_t)((RawData[175] >> 5) & 0x01));
  packet->TStat4Enabled = ((uint8_t)((RawData[175] >> 4) & 0x01));
  packet->TStat3Enabled = ((uint8_t)((RawData[175] >> 3) & 0x01));
  packet->TStat2Enabled = ((uint8_t)((RawData[175] >> 2) & 0x01));
  packet->TStat1Enabled = ((uint8_t)((RawData[175] >> 1) & 0x01));
  packet->TStat0Enabled = ((uint8_t)(RawData[175] & 0x01));
  packet->MagredEnabled = ((uint8_t)((RawData[176] >> 4) & 0x01));
  packet->MagredPID = ((uint8_t)((RawData[176] >> 3) & 0x01));
  packet->ClkbrdGPIO2 = ((uint8_t)((RawData[176] >> 2) & 0x01));
  packet->ClkbrdGPIO1 = ((uint8_t)((RawData[176] >> 1) & 0x01));
  packet->ClkbrdGPIO0 = ((uint8_t)(RawData[176] & 0x01));
  packet->MagredCounter = ((uint8_t)RawData[177]);
  packet->RelayNumPacketsDropped = (((uint32_t)RawData[178]) << 24) | (((uint32_t)RawData[179]) << 16) | (((uint32_t)RawData[180]) << 8) | ((uint32_t)RawData[181]);
  packet->RelayNumBytesDropped = (((uint64_t)RawData[182]) << 32) | (((uint64_t)RawData[183]) << 24) | (((uint64_t)RawData[184]) << 16) | (((uint64_t)RawData[185]) << 8);
  packet->RelayRawDataEnabled = ((uint8_t)((RawData[186] >> 2) & 0x01));
  packet->RelayComptonEnabled = ((uint8_t)((RawData[186] >> 1) & 0x01));
  packet->RelayQueueAccepting = ((uint8_t)(RawData[186] & 0x01));
  packet->LOSNumBytesInQueue = (((uint32_t)RawData[187]) << 24) | (((uint32_t)RawData[188]) << 16) | (((uint32_t)RawData[189]) << 8) | ((uint32_t)RawData[190]);
  packet->LOSBytesWritten = (((uint64_t)RawData[191]) << 32) | (((uint64_t)RawData[192]) << 24) | (((uint64_t)RawData[193]) << 16) | (((uint64_t)RawData[194]) << 8);
  packet->LOSNumPacketsDropped = (((uint32_t)RawData[195]) << 24) | (((uint32_t)RawData[196]) << 16) | (((uint32_t)RawData[197]) << 8) | ((uint32_t)RawData[198]);
  packet->LOSNumBytesDropped = (((uint64_t)RawData[199]) << 32) | (((uint64_t)RawData[200]) << 24) | (((uint64_t)RawData[201]) << 16) | (((uint64_t)RawData[202]) << 8);
  packet->LOSAutoClear = ((uint8_t)((RawData[203] >> 4) & 0x01));
  packet->LOSIsOpen = ((uint8_t)((RawData[203] >> 3) & 0x01));
  packet->LOSComptonEnabled = ((uint8_t)((RawData[203] >> 2) & 0x01));
  packet->LOSQueueAccepting = ((uint8_t)((RawData[203] >> 1) & 0x01));
  packet->LOSEnabled = ((uint8_t)(RawData[203] & 0x01));
  packet->DiskNumBytesInQueue = (((uint32_t)RawData[204]) << 24) | (((uint32_t)RawData[205]) << 16) | (((uint32_t)RawData[206]) << 8) | ((uint32_t)RawData[207]);
  packet->DiskANumBytesWritten = (((uint64_t)RawData[208]) << 32) | (((uint64_t)RawData[209]) << 24) | (((uint64_t)RawData[210]) << 16) | (((uint64_t)RawData[211]) << 8);
  packet->DiskBNumBytesWritten = (((uint64_t)RawData[212]) << 32) | (((uint64_t)RawData[213]) << 24) | (((uint64_t)RawData[214]) << 16) | (((uint64_t)RawData[215]) << 8);
  packet->DiskCNumBytesWritten = (((uint64_t)RawData[216]) << 32) | (((uint64_t)RawData[217]) << 24) | (((uint64_t)RawData[218]) << 16) | (((uint64_t)RawData[219]) << 8);
  packet->DiskCIsMounted = ((uint8_t)((RawData[220] >> 6) & 0x01));
  packet->DiskBIsMounted = ((uint8_t)((RawData[220] >> 5) & 0x01));
  packet->DiskAIsMounted = ((uint8_t)((RawData[220] >> 4) & 0x01));
  packet->DiskCNotNull = ((uint8_t)((RawData[220] >> 3) & 0x01));
  packet->DiskBNotNull = ((uint8_t)((RawData[220] >> 2) & 0x01));
  packet->DiskANotNull = ((uint8_t)((RawData[220] >> 1) & 0x01));
  packet->DiskComptonEnabled = ((uint8_t)(RawData[220] & 0x01));
  packet->GRBNumBytesInQueue = (((uint32_t)RawData[221]) << 24) | (((uint32_t)RawData[222]) << 16) | (((uint32_t)RawData[223]) << 8) | ((uint32_t)RawData[224]);
  packet->GRBTdiff = (((uint32_t)RawData[225]) << 24) | (((uint32_t)RawData[226]) << 16) | (((uint32_t)RawData[227]) << 8) | ((uint32_t)RawData[228]);
  packet->GRBNumTriggers = (((uint32_t)RawData[229]) << 24) | (((uint32_t)RawData[230]) << 16) | (((uint32_t)RawData[231]) << 8) | ((uint32_t)RawData[232]);
  packet->ShieldAlgoTrigEnabled2 = ((uint8_t)((RawData[233] >> 6) & 0x01));
  packet->ShieldAlgoTrigEnabled1 = ((uint8_t)((RawData[233] >> 5) & 0x01));
  packet->ShieldAlgoTrigEnabled0 = ((uint8_t)((RawData[233] >> 4) & 0x01));
  packet->GRBAutoTriggerEnabled = ((uint8_t)((RawData[233] >> 3) & 0x01));
  packet->GRBTriggerActive = ((uint8_t)((RawData[233] >> 2) & 0x01));
  packet->GRBQueueAccepting = ((uint8_t)((RawData[233] >> 1) & 0x01));
  packet->RTANumBytesInQueue = (((uint32_t)RawData[234]) << 24) | (((uint32_t)RawData[235]) << 16) | (((uint32_t)RawData[236]) << 8) | ((uint32_t)RawData[237]);
  packet->RTAAcceptingSingleDetectorEvents = ((uint8_t)((RawData[238] >> 5) & 0x01));
  packet->RTAAutoRebootEnabled = ((uint8_t)((RawData[238] >> 4) & 0x01));
  packet->RTAComptonAnalysisEnabled = ((uint8_t)((RawData[238] >> 3) & 0x01));
  packet->RTAComptonQueueAutoClearEnabled = ((uint8_t)((RawData[238] >> 2) & 0x01));
  packet->RTAQueueAccepting = ((uint8_t)((RawData[238] >> 1) & 0x01));
  packet->RTAEnabled = ((uint8_t)(RawData[238] & 0x01));
  packet->OpenPortANumBytesWritten = (((uint64_t)RawData[239]) << 32) | (((uint64_t)RawData[240]) << 24) | (((uint64_t)RawData[241]) << 16) | (((uint64_t)RawData[242]) << 8);
  packet->UnixTimeMSB = ((uint8_t)RawData[243]);
  packet->OpenPortAUDPSocket = ((uint8_t)((RawData[245] >> 4) & 0x01));
  packet->OpenPortAEnabled = ((uint8_t)(RawData[245] & 0x01));
  packet->OpenPortBNumBytesWritten = (((uint64_t)RawData[246]) << 32) | (((uint64_t)RawData[247]) << 24) | (((uint64_t)RawData[248]) << 16) | (((uint64_t)RawData[249]) << 8);
  packet->OpenPortBUDPSocket = ((uint8_t)((RawData[252] >> 4) & 0x01));
  packet->OpenPortBEnabled = ((uint8_t)(RawData[252] & 0x01));
  packet->NumRestartsCC = (uint8_t*)malloc(12*sizeof(uint8_t));
  for (i = 0; i < 12; i += 1) {
    packet->NumRestartsCC[i] = ((uint8_t)RawData[253 + 1*i + 0]);
  }
  packet->SerialDialupOpen = ((uint8_t)((RawData[265] >> 5) & 0x01));
  packet->SerialCryoOpen = ((uint8_t)((RawData[265] >> 4) & 0x01));
  packet->SerialMagOpen = ((uint8_t)((RawData[265] >> 3) & 0x01));
  packet->SerialGPSOpen = ((uint8_t)((RawData[265] >> 2) & 0x01));
  packet->SerialSIP2Open = ((uint8_t)((RawData[265] >> 1) & 0x01));
  packet->SerialSIP1Open = ((uint8_t)(RawData[265] & 0x01));
  packet->DiskAUsage = ((uint8_t)RawData[266]);
  packet->DiskBUsage = ((uint8_t)RawData[267]);
  packet->DiskCUsage = ((uint8_t)RawData[268]);
  packet->HVStatusCC11 = ((uint8_t)((RawData[269] >> 3) & 0x01));
  packet->HVStatusCC10 = ((uint8_t)((RawData[269] >> 2) & 0x01));
  packet->HVStatusCC9 = ((uint8_t)((RawData[269] >> 1) & 0x01));
  packet->HVStatusCC8 = ((uint8_t)(RawData[269] & 0x01));
  packet->HVStatusCC7 = ((uint8_t)((RawData[270] >> 7) & 0x01));
  packet->HVStatusCC6 = ((uint8_t)((RawData[270] >> 6) & 0x01));
  packet->HVStatusCC5 = ((uint8_t)((RawData[270] >> 5) & 0x01));
  packet->HVStatusCC4 = ((uint8_t)((RawData[270] >> 4) & 0x01));
  packet->HVStatusCC3 = ((uint8_t)((RawData[270] >> 3) & 0x01));
  packet->HVStatusCC2 = ((uint8_t)((RawData[270] >> 2) & 0x01));
  packet->HVStatusCC1 = ((uint8_t)((RawData[270] >> 1) & 0x01));
  packet->HVStatusCC0 = ((uint8_t)(RawData[270] & 0x01));
  packet->HVStatusShield5 = ((uint8_t)((RawData[271] >> 5) & 0x01));
  packet->HVStatusShield4 = ((uint8_t)((RawData[271] >> 4) & 0x01));
  packet->HVStatusShield3 = ((uint8_t)((RawData[271] >> 3) & 0x01));
  packet->HVStatusShield2 = ((uint8_t)((RawData[271] >> 2) & 0x01));
  packet->HVStatusShield1 = ((uint8_t)((RawData[271] >> 1) & 0x01));
  packet->HVStatusShield0 = ((uint8_t)(RawData[271] & 0x01));
  packet->ShieldThreshold = (uint8_t*)malloc(6*sizeof(uint8_t));
  for (i = 0; i < 6; i += 1) {
    packet->ShieldThreshold[i] = ((uint8_t)RawData[272 + 1*i + 0]);
  }
  packet->DiskCEnabled = ((uint8_t)((RawData[278] >> 7) & 0x01));
  packet->DiskBEnabled = ((uint8_t)((RawData[278] >> 6) & 0x01));
  packet->DiskAEnabled = ((uint8_t)((RawData[278] >> 5) & 0x01));
  packet->GlobalClockSupervisorResyncEnabled = ((uint8_t)((RawData[278] >> 4) & 0x01));
  packet->GPSMagEnabled = ((uint8_t)((RawData[278] >> 3) & 0x01));
  packet->GPSEnabled = ((uint8_t)((RawData[278] >> 2) & 0x01));
  packet->EthernetReallocateEnabled = ((uint8_t)((RawData[278] >> 1) & 0x01));
  packet->GlobalPROMLoadSuccessful = ((uint8_t)(RawData[278] & 0x01));
  packet->RTAContinuityOK7 = ((uint8_t)((RawData[279] >> 7) & 0x01));
  packet->RTAContinuityOK6 = ((uint8_t)((RawData[279] >> 6) & 0x01));
  packet->RTAContinuityOK5 = ((uint8_t)((RawData[279] >> 5) & 0x01));
  packet->RTAContinuityOK4 = ((uint8_t)((RawData[279] >> 4) & 0x01));
  packet->RTAContinuityOK3 = ((uint8_t)((RawData[279] >> 3) & 0x01));
  packet->RTAContinuityOK2 = ((uint8_t)((RawData[279] >> 2) & 0x01));
  packet->RTAContinuityOK1 = ((uint8_t)((RawData[279] >> 1) & 0x01));
  packet->RTAContinuityOK0 = ((uint8_t)(RawData[279] & 0x01));
  packet->GlobalSyncStatusCC3 = ((uint8_t)((RawData[280] >> 7) & 0x01));
  packet->GlobalSyncStatusCC2 = ((uint8_t)((RawData[280] >> 6) & 0x01));
  packet->GlobalSyncStatusCC1 = ((uint8_t)((RawData[280] >> 5) & 0x01));
  packet->GlobalSyncStatusCC0 = ((uint8_t)((RawData[280] >> 4) & 0x01));
  packet->RTAContinuityOK11 = ((uint8_t)((RawData[280] >> 3) & 0x01));
  packet->RTAContinuityOK10 = ((uint8_t)((RawData[280] >> 2) & 0x01));
  packet->RTAContinuityOK9 = ((uint8_t)((RawData[280] >> 1) & 0x01));
  packet->RTAContinuityOK8 = ((uint8_t)(RawData[280] & 0x01));
  packet->GlobalSyncStatusCC11 = ((uint8_t)((RawData[281] >> 7) & 0x01));
  packet->GlobalSyncStatusCC10 = ((uint8_t)((RawData[281] >> 6) & 0x01));
  packet->GlobalSyncStatusCC9 = ((uint8_t)((RawData[281] >> 5) & 0x01));
  packet->GlobalSyncStatusCC8 = ((uint8_t)((RawData[281] >> 4) & 0x01));
  packet->GlobalSyncStatusCC7 = ((uint8_t)((RawData[281] >> 3) & 0x01));
  packet->GlobalSyncStatusCC6 = ((uint8_t)((RawData[281] >> 2) & 0x01));
  packet->GlobalSyncStatusCC5 = ((uint8_t)((RawData[281] >> 1) & 0x01));
  packet->GlobalSyncStatusCC4 = ((uint8_t)(RawData[281] & 0x01));
  packet->RTANumEventsActive7 = ((uint8_t)((RawData[282] >> 7) & 0x01));
  packet->RTANumEventsActive6 = ((uint8_t)((RawData[282] >> 6) & 0x01));
  packet->RTANumEventsActive5 = ((uint8_t)((RawData[282] >> 5) & 0x01));
  packet->RTANumEventsActive4 = ((uint8_t)((RawData[282] >> 4) & 0x01));
  packet->RTANumEventsActive3 = ((uint8_t)((RawData[282] >> 3) & 0x01));
  packet->RTANumEventsActive2 = ((uint8_t)((RawData[282] >> 2) & 0x01));
  packet->RTANumEventsActive1 = ((uint8_t)((RawData[282] >> 1) & 0x01));
  packet->RTANumEventsActive0 = ((uint8_t)(RawData[282] & 0x01));
  packet->RTAAnalysisEnabled3 = ((uint8_t)((RawData[283] >> 7) & 0x01));
  packet->RTAAnalysisEnabled2 = ((uint8_t)((RawData[283] >> 6) & 0x01));
  packet->RTAAnalysisEnabled1 = ((uint8_t)((RawData[283] >> 5) & 0x01));
  packet->RTAAnalysisEnabled0 = ((uint8_t)((RawData[283] >> 4) & 0x01));
  packet->RTANumEventsActive11 = ((uint8_t)((RawData[283] >> 3) & 0x01));
  packet->RTANumEventsActive10 = ((uint8_t)((RawData[283] >> 2) & 0x01));
  packet->RTANumEventsActive9 = ((uint8_t)((RawData[283] >> 1) & 0x01));
  packet->RTANumEventsActive8 = ((uint8_t)(RawData[283] & 0x01));
  packet->RTAAnalysisEnabled11 = ((uint8_t)((RawData[284] >> 7) & 0x01));
  packet->RTAAnalysisEnabled10 = ((uint8_t)((RawData[284] >> 6) & 0x01));
  packet->RTAAnalysisEnabled9 = ((uint8_t)((RawData[284] >> 5) & 0x01));
  packet->RTAAnalysisEnabled8 = ((uint8_t)((RawData[284] >> 4) & 0x01));
  packet->RTAAnalysisEnabled7 = ((uint8_t)((RawData[284] >> 3) & 0x01));
  packet->RTAAnalysisEnabled6 = ((uint8_t)((RawData[284] >> 2) & 0x01));
  packet->RTAAnalysisEnabled5 = ((uint8_t)((RawData[284] >> 1) & 0x01));
  packet->RTAAnalysisEnabled4 = ((uint8_t)(RawData[284] & 0x01));
  packet->GRBLastTrigTime = (((uint32_t)RawData[285]) << 24) | (((uint32_t)RawData[286]) << 16) | (((uint32_t)RawData[287]) << 8) | ((uint32_t)RawData[288]);
  packet->ShieldNumAlgoTriggers0 = ((uint8_t)RawData[289]);
  packet->ShieldNumAlgoTriggers1 = ((uint8_t)RawData[290]);
  packet->ShieldNumAlgoTriggers2 = ((uint8_t)RawData[291]);
  packet->WhichSIP = ((uint8_t)((RawData[292] >> 2) & 0x01));
  packet->TelemUDPMode = ((uint8_t)(RawData[292] & 0x01));
  packet->SIPLon = (((uint16_t)RawData[293]) << 8) | ((uint16_t)RawData[294]);
  packet->SIPLat = (((uint16_t)RawData[295]) << 8) | ((uint16_t)RawData[296]);
  packet->SIPAlt = (((uint16_t)RawData[297]) << 8) | ((uint16_t)RawData[298]);
  packet->SIPTime = (((uint32_t)RawData[299]) << 24) | (((uint32_t)RawData[300]) << 16) | (((uint32_t)RawData[301]) << 8) | ((uint32_t)RawData[302]);
  packet->SIPWeek = (((uint16_t)RawData[303]) << 8) | ((uint16_t)RawData[304]);
  packet->SIPPressure = (((uint16_t)RawData[305]) << 8) | ((uint16_t)RawData[306]);
  packet->SIPHealth = (((uint16_t)RawData[307]) << 8) | ((uint16_t)RawData[308]);
  packet->FlightCodeCPUUsage = ((uint8_t)RawData[309]);
  packet->FlightCodeMemUsage = ((uint8_t)RawData[310]);
  packet->MagredCPUUsage = ((uint8_t)RawData[311]);
  packet->MagredMemUsage = ((uint8_t)RawData[312]);
  packet->EthernetTotalNumRawDataframes = (((uint32_t)RawData[317]) << 24) | (((uint32_t)RawData[318]) << 16) | (((uint32_t)RawData[319]) << 8) | ((uint32_t)RawData[320]);
  packet->EthernetTotalNumComptonDataframes = (((uint32_t)RawData[321]) << 24) | (((uint32_t)RawData[322]) << 16) | (((uint32_t)RawData[323]) << 8) | ((uint32_t)RawData[324]);
  packet->DoublePumpOverrideEnabled = ((uint8_t)((RawData[325] >> 7) & 0x01));
  packet->PumpEPromCodeGood = ((uint8_t)((RawData[325] >> 6) & 0x01));
  packet->PumpTachAValid = ((uint8_t)((RawData[325] >> 5) & 0x01));
  packet->PumpTachBValid = ((uint8_t)((RawData[325] >> 4) & 0x01));
  packet->PumpDACASetting = ((uint8_t)RawData[326]);
  packet->PumpDACBSetting = ((uint8_t)RawData[327]);
  packet->PumpTachALast = (((uint16_t)RawData[328]) << 8) | ((uint16_t)RawData[329]);
  packet->PumpTachAHigh = (((uint16_t)RawData[330]) << 8) | ((uint16_t)RawData[331]);
  packet->PumpTachALow = (((uint16_t)RawData[332]) << 8) | ((uint16_t)RawData[333]);
  packet->PumpTachBLast = (((uint16_t)RawData[334]) << 8) | ((uint16_t)RawData[335]);
  packet->PumpTachBHigh = (((uint16_t)RawData[336]) << 8) | ((uint16_t)RawData[337]);
  packet->PumpTachBLow = (((uint16_t)RawData[338]) << 8) | ((uint16_t)RawData[339]);
  packet->PumpLevelSensor = (((uint16_t)RawData[340]) << 8) | ((uint16_t)RawData[341]);
  packet->ShieldNumSamples = ((uint8_t)RawData[342]);
  packet->NumCounts = (uint32_t*)malloc(packet->ShieldNumSamples*sizeof(uint32_t));
  packet->TimeInterval = (uint32_t*)malloc(packet->ShieldNumSamples*sizeof(uint32_t));
  packet->LiveTimeFraction = (uint8_t*)malloc(packet->ShieldNumSamples*sizeof(uint8_t));
  shieldSample : for (i = 0; i < packet->ShieldNumSamples; i += 1) {
    packet->NumCounts[i] = (((uint32_t)RawData[343 + 9*i + 0]) << 24) | (((uint32_t)RawData[343 + 9*i + 1]) << 16) | (((uint32_t)RawData[343 + 9*i + 2]) << 8) | ((uint32_t)RawData[343 + 9*i + 3]);
    packet->TimeInterval[i] = (((uint32_t)RawData[343 + 9*i + 4]) << 24) | (((uint32_t)RawData[343 + 9*i + 5]) << 16) | (((uint32_t)RawData[343 + 9*i + 6]) << 8) | ((uint32_t)RawData[343 + 9*i + 7]);
    packet->LiveTimeFraction[i] = ((uint8_t)RawData[343 + 9*i + 8]);
  }
}

struct GCUHousekeepingPacket* MakeNewGCUHousekeepingPacket(void) { 
  uint8_t i = 0;
  struct GCUHousekeepingPacket* packet = (struct GCUHousekeepingPacket*) malloc (sizeof(struct GCUHousekeepingPacket));
  memset(packet, 0, sizeof(struct GCUHousekeepingPacket));
  return packet;
}

void FreeAllocatedGCUHousekeepingPacket(struct GCUHousekeepingPacket* packet) {
  uint16_t i;
  uint16_t j;


  free(packet->Temp);
  free(packet->ADC);
  free(packet->LastCommandIDs);
  free(packet->LastCommandPayloads);
  free(packet->NumRestartsCC);
  free(packet->ShieldThreshold);
  free(packet->NumCounts);
  free(packet->TimeInterval);
  free(packet->LiveTimeFraction);
}


void printGCUHousekeepingPacket(struct GCUHousekeepingPacket* packet) {
  uint16_t i;
  uint16_t j;

  printf("packet->Sync = 0x%04x\n", packet->Sync);
  printf("packet->PacketID = 0x%02x\n", packet->PacketID);
  printf("packet->UnixTime = 0x%06x\n", packet->UnixTime);
  printf("packet->PacketCounter = 0x%04x\n", packet->PacketCounter);
  printf("packet->PacketSize = 0x%04x\n", packet->PacketSize);
  tempIter : for (i = 0; i < 32; i += 1) {
    printf("packet->Temp[%0d] = 0x%04x\n", i, packet->Temp[i]);
  }
  for (j = 0; j <= 31; j += 1) {
    printf("packet->ADC[%0d] = 0x%04x\n", j, packet->ADC[j]);
  }
  printf("packet->CryoTipTemp = 0x%04x\n", packet->CryoTipTemp);
  printf("packet->CryoPower = 0x%04x\n", packet->CryoPower);
  printf("packet->ClkVal = 0x%012lx\n", (uint64_t)(packet->ClkVal));
  lastCommandsIter : for (i = 0; i < 3; i += 1) {
    printf("packet->LastCommandIDs[%0d] = 0x%02x\n", i, packet->LastCommandIDs[i]);
    printf("packet->LastCommandPayloads[%0d] = 0x%04x\n", i, packet->LastCommandPayloads[i]);
  }
  printf("packet->NumGoodCommands = 0x%04x\n", packet->NumGoodCommands);
  printf("packet->NumBadCommands = 0x%04x\n", packet->NumBadCommands);
  printf("packet->OPADataRate = 0x%02x\n", packet->OPADataRate);
  printf("packet->OPBDataRate = 0x%02x\n", packet->OPBDataRate);
  printf("packet->NumAutoResyncs = 0x%02x\n", packet->NumAutoResyncs);
  printf("packet->HeaterBoxReadBack7 = 0x%01x\n", packet->HeaterBoxReadBack7);
  printf("packet->HeaterBoxReadBack6 = 0x%01x\n", packet->HeaterBoxReadBack6);
  printf("packet->HeaterBoxReadBack5 = 0x%01x\n", packet->HeaterBoxReadBack5);
  printf("packet->HeaterBoxReadBack4 = 0x%01x\n", packet->HeaterBoxReadBack4);
  printf("packet->HeaterBoxReadBack3 = 0x%01x\n", packet->HeaterBoxReadBack3);
  printf("packet->HeaterBoxReadBack2 = 0x%01x\n", packet->HeaterBoxReadBack2);
  printf("packet->HeaterBoxReadBack1 = 0x%01x\n", packet->HeaterBoxReadBack1);
  printf("packet->HeaterBoxReadBack0 = 0x%01x\n", packet->HeaterBoxReadBack0);
  printf("packet->NumGCURestarts = 0x%04x\n", packet->NumGCURestarts);
  printf("packet->NumBytesInRelayQueue = 0x%08x\n", packet->NumBytesInRelayQueue);
  printf("packet->HeaterBoxSetting7 = 0x%01x\n", packet->HeaterBoxSetting7);
  printf("packet->HeaterBoxSetting6 = 0x%01x\n", packet->HeaterBoxSetting6);
  printf("packet->HeaterBoxSetting5 = 0x%01x\n", packet->HeaterBoxSetting5);
  printf("packet->HeaterBoxSetting4 = 0x%01x\n", packet->HeaterBoxSetting4);
  printf("packet->HeaterBoxSetting3 = 0x%01x\n", packet->HeaterBoxSetting3);
  printf("packet->HeaterBoxSetting2 = 0x%01x\n", packet->HeaterBoxSetting2);
  printf("packet->HeaterBoxSetting1 = 0x%01x\n", packet->HeaterBoxSetting1);
  printf("packet->HeaterBoxSetting0 = 0x%01x\n", packet->HeaterBoxSetting0);
  printf("packet->TStat7Enabled = 0x%01x\n", packet->TStat7Enabled);
  printf("packet->TStat6Enabled = 0x%01x\n", packet->TStat6Enabled);
  printf("packet->TStat5Enabled = 0x%01x\n", packet->TStat5Enabled);
  printf("packet->TStat4Enabled = 0x%01x\n", packet->TStat4Enabled);
  printf("packet->TStat3Enabled = 0x%01x\n", packet->TStat3Enabled);
  printf("packet->TStat2Enabled = 0x%01x\n", packet->TStat2Enabled);
  printf("packet->TStat1Enabled = 0x%01x\n", packet->TStat1Enabled);
  printf("packet->TStat0Enabled = 0x%01x\n", packet->TStat0Enabled);
  printf("packet->MagredEnabled = 0x%01x\n", packet->MagredEnabled);
  printf("packet->MagredPID = 0x%01x\n", packet->MagredPID);
  printf("packet->ClkbrdGPIO2 = 0x%01x\n", packet->ClkbrdGPIO2);
  printf("packet->ClkbrdGPIO1 = 0x%01x\n", packet->ClkbrdGPIO1);
  printf("packet->ClkbrdGPIO0 = 0x%01x\n", packet->ClkbrdGPIO0);
  printf("packet->MagredCounter = 0x%02x\n", packet->MagredCounter);
  printf("packet->RelayNumPacketsDropped = 0x%08x\n", packet->RelayNumPacketsDropped);
  printf("packet->RelayNumBytesDropped = 0x%08lx\n", (uint64_t)(packet->RelayNumBytesDropped));
  printf("packet->RelayRawDataEnabled = 0x%01x\n", packet->RelayRawDataEnabled);
  printf("packet->RelayComptonEnabled = 0x%01x\n", packet->RelayComptonEnabled);
  printf("packet->RelayQueueAccepting = 0x%01x\n", packet->RelayQueueAccepting);
  printf("packet->LOSNumBytesInQueue = 0x%08x\n", packet->LOSNumBytesInQueue);
  printf("packet->LOSBytesWritten = 0x%08lx\n", (uint64_t)(packet->LOSBytesWritten));
  printf("packet->LOSNumPacketsDropped = 0x%08x\n", packet->LOSNumPacketsDropped);
  printf("packet->LOSNumBytesDropped = 0x%08lx\n", (uint64_t)(packet->LOSNumBytesDropped));
  printf("packet->LOSAutoClear = 0x%01x\n", packet->LOSAutoClear);
  printf("packet->LOSIsOpen = 0x%01x\n", packet->LOSIsOpen);
  printf("packet->LOSComptonEnabled = 0x%01x\n", packet->LOSComptonEnabled);
  printf("packet->LOSQueueAccepting = 0x%01x\n", packet->LOSQueueAccepting);
  printf("packet->LOSEnabled = 0x%01x\n", packet->LOSEnabled);
  printf("packet->DiskNumBytesInQueue = 0x%08x\n", packet->DiskNumBytesInQueue);
  printf("packet->DiskANumBytesWritten = 0x%08lx\n", (uint64_t)(packet->DiskANumBytesWritten));
  printf("packet->DiskBNumBytesWritten = 0x%08lx\n", (uint64_t)(packet->DiskBNumBytesWritten));
  printf("packet->DiskCNumBytesWritten = 0x%08lx\n", (uint64_t)(packet->DiskCNumBytesWritten));
  printf("packet->DiskCIsMounted = 0x%01x\n", packet->DiskCIsMounted);
  printf("packet->DiskBIsMounted = 0x%01x\n", packet->DiskBIsMounted);
  printf("packet->DiskAIsMounted = 0x%01x\n", packet->DiskAIsMounted);
  printf("packet->DiskCNotNull = 0x%01x\n", packet->DiskCNotNull);
  printf("packet->DiskBNotNull = 0x%01x\n", packet->DiskBNotNull);
  printf("packet->DiskANotNull = 0x%01x\n", packet->DiskANotNull);
  printf("packet->DiskComptonEnabled = 0x%01x\n", packet->DiskComptonEnabled);
  printf("packet->GRBNumBytesInQueue = 0x%08x\n", packet->GRBNumBytesInQueue);
  printf("packet->GRBTdiff = 0x%08x\n", packet->GRBTdiff);
  printf("packet->GRBNumTriggers = 0x%08x\n", packet->GRBNumTriggers);
  printf("packet->ShieldAlgoTrigEnabled2 = 0x%01x\n", packet->ShieldAlgoTrigEnabled2);
  printf("packet->ShieldAlgoTrigEnabled1 = 0x%01x\n", packet->ShieldAlgoTrigEnabled1);
  printf("packet->ShieldAlgoTrigEnabled0 = 0x%01x\n", packet->ShieldAlgoTrigEnabled0);
  printf("packet->GRBAutoTriggerEnabled = 0x%01x\n", packet->GRBAutoTriggerEnabled);
  printf("packet->GRBTriggerActive = 0x%01x\n", packet->GRBTriggerActive);
  printf("packet->GRBQueueAccepting = 0x%01x\n", packet->GRBQueueAccepting);
  printf("packet->RTANumBytesInQueue = 0x%08x\n", packet->RTANumBytesInQueue);
  printf("packet->RTAAcceptingSingleDetectorEvents = 0x%01x\n", packet->RTAAcceptingSingleDetectorEvents);
  printf("packet->RTAAutoRebootEnabled = 0x%01x\n", packet->RTAAutoRebootEnabled);
  printf("packet->RTAComptonAnalysisEnabled = 0x%01x\n", packet->RTAComptonAnalysisEnabled);
  printf("packet->RTAComptonQueueAutoClearEnabled = 0x%01x\n", packet->RTAComptonQueueAutoClearEnabled);
  printf("packet->RTAQueueAccepting = 0x%01x\n", packet->RTAQueueAccepting);
  printf("packet->RTAEnabled = 0x%01x\n", packet->RTAEnabled);
  printf("packet->OpenPortANumBytesWritten = 0x%08lx\n", (uint64_t)(packet->OpenPortANumBytesWritten));
  printf("packet->UnixTimeMSB = 0x%02x\n", packet->UnixTimeMSB);
  printf("packet->OpenPortAUDPSocket = 0x%01x\n", packet->OpenPortAUDPSocket);
  printf("packet->OpenPortAEnabled = 0x%01x\n", packet->OpenPortAEnabled);
  printf("packet->OpenPortBNumBytesWritten = 0x%08lx\n", (uint64_t)(packet->OpenPortBNumBytesWritten));
  printf("packet->OpenPortBUDPSocket = 0x%01x\n", packet->OpenPortBUDPSocket);
  printf("packet->OpenPortBEnabled = 0x%01x\n", packet->OpenPortBEnabled);
  for (i = 0; i < 12; i += 1) {
    printf("packet->NumRestartsCC[%0d] = 0x%02x\n", i, packet->NumRestartsCC[i]);
  }
  printf("packet->SerialDialupOpen = 0x%01x\n", packet->SerialDialupOpen);
  printf("packet->SerialCryoOpen = 0x%01x\n", packet->SerialCryoOpen);
  printf("packet->SerialMagOpen = 0x%01x\n", packet->SerialMagOpen);
  printf("packet->SerialGPSOpen = 0x%01x\n", packet->SerialGPSOpen);
  printf("packet->SerialSIP2Open = 0x%01x\n", packet->SerialSIP2Open);
  printf("packet->SerialSIP1Open = 0x%01x\n", packet->SerialSIP1Open);
  printf("packet->DiskAUsage = 0x%02x\n", packet->DiskAUsage);
  printf("packet->DiskBUsage = 0x%02x\n", packet->DiskBUsage);
  printf("packet->DiskCUsage = 0x%02x\n", packet->DiskCUsage);
  printf("packet->HVStatusCC11 = 0x%01x\n", packet->HVStatusCC11);
  printf("packet->HVStatusCC10 = 0x%01x\n", packet->HVStatusCC10);
  printf("packet->HVStatusCC9 = 0x%01x\n", packet->HVStatusCC9);
  printf("packet->HVStatusCC8 = 0x%01x\n", packet->HVStatusCC8);
  printf("packet->HVStatusCC7 = 0x%01x\n", packet->HVStatusCC7);
  printf("packet->HVStatusCC6 = 0x%01x\n", packet->HVStatusCC6);
  printf("packet->HVStatusCC5 = 0x%01x\n", packet->HVStatusCC5);
  printf("packet->HVStatusCC4 = 0x%01x\n", packet->HVStatusCC4);
  printf("packet->HVStatusCC3 = 0x%01x\n", packet->HVStatusCC3);
  printf("packet->HVStatusCC2 = 0x%01x\n", packet->HVStatusCC2);
  printf("packet->HVStatusCC1 = 0x%01x\n", packet->HVStatusCC1);
  printf("packet->HVStatusCC0 = 0x%01x\n", packet->HVStatusCC0);
  printf("packet->HVStatusShield5 = 0x%01x\n", packet->HVStatusShield5);
  printf("packet->HVStatusShield4 = 0x%01x\n", packet->HVStatusShield4);
  printf("packet->HVStatusShield3 = 0x%01x\n", packet->HVStatusShield3);
  printf("packet->HVStatusShield2 = 0x%01x\n", packet->HVStatusShield2);
  printf("packet->HVStatusShield1 = 0x%01x\n", packet->HVStatusShield1);
  printf("packet->HVStatusShield0 = 0x%01x\n", packet->HVStatusShield0);
  for (i = 0; i < 6; i += 1) {
    printf("packet->ShieldThreshold[%0d] = 0x%02x\n", i, packet->ShieldThreshold[i]);
  }
  printf("packet->DiskCEnabled = 0x%01x\n", packet->DiskCEnabled);
  printf("packet->DiskBEnabled = 0x%01x\n", packet->DiskBEnabled);
  printf("packet->DiskAEnabled = 0x%01x\n", packet->DiskAEnabled);
  printf("packet->GlobalClockSupervisorResyncEnabled = 0x%01x\n", packet->GlobalClockSupervisorResyncEnabled);
  printf("packet->GPSMagEnabled = 0x%01x\n", packet->GPSMagEnabled);
  printf("packet->GPSEnabled = 0x%01x\n", packet->GPSEnabled);
  printf("packet->EthernetReallocateEnabled = 0x%01x\n", packet->EthernetReallocateEnabled);
  printf("packet->GlobalPROMLoadSuccessful = 0x%01x\n", packet->GlobalPROMLoadSuccessful);
  printf("packet->RTAContinuityOK7 = 0x%01x\n", packet->RTAContinuityOK7);
  printf("packet->RTAContinuityOK6 = 0x%01x\n", packet->RTAContinuityOK6);
  printf("packet->RTAContinuityOK5 = 0x%01x\n", packet->RTAContinuityOK5);
  printf("packet->RTAContinuityOK4 = 0x%01x\n", packet->RTAContinuityOK4);
  printf("packet->RTAContinuityOK3 = 0x%01x\n", packet->RTAContinuityOK3);
  printf("packet->RTAContinuityOK2 = 0x%01x\n", packet->RTAContinuityOK2);
  printf("packet->RTAContinuityOK1 = 0x%01x\n", packet->RTAContinuityOK1);
  printf("packet->RTAContinuityOK0 = 0x%01x\n", packet->RTAContinuityOK0);
  printf("packet->GlobalSyncStatusCC3 = 0x%01x\n", packet->GlobalSyncStatusCC3);
  printf("packet->GlobalSyncStatusCC2 = 0x%01x\n", packet->GlobalSyncStatusCC2);
  printf("packet->GlobalSyncStatusCC1 = 0x%01x\n", packet->GlobalSyncStatusCC1);
  printf("packet->GlobalSyncStatusCC0 = 0x%01x\n", packet->GlobalSyncStatusCC0);
  printf("packet->RTAContinuityOK11 = 0x%01x\n", packet->RTAContinuityOK11);
  printf("packet->RTAContinuityOK10 = 0x%01x\n", packet->RTAContinuityOK10);
  printf("packet->RTAContinuityOK9 = 0x%01x\n", packet->RTAContinuityOK9);
  printf("packet->RTAContinuityOK8 = 0x%01x\n", packet->RTAContinuityOK8);
  printf("packet->GlobalSyncStatusCC11 = 0x%01x\n", packet->GlobalSyncStatusCC11);
  printf("packet->GlobalSyncStatusCC10 = 0x%01x\n", packet->GlobalSyncStatusCC10);
  printf("packet->GlobalSyncStatusCC9 = 0x%01x\n", packet->GlobalSyncStatusCC9);
  printf("packet->GlobalSyncStatusCC8 = 0x%01x\n", packet->GlobalSyncStatusCC8);
  printf("packet->GlobalSyncStatusCC7 = 0x%01x\n", packet->GlobalSyncStatusCC7);
  printf("packet->GlobalSyncStatusCC6 = 0x%01x\n", packet->GlobalSyncStatusCC6);
  printf("packet->GlobalSyncStatusCC5 = 0x%01x\n", packet->GlobalSyncStatusCC5);
  printf("packet->GlobalSyncStatusCC4 = 0x%01x\n", packet->GlobalSyncStatusCC4);
  printf("packet->RTANumEventsActive7 = 0x%01x\n", packet->RTANumEventsActive7);
  printf("packet->RTANumEventsActive6 = 0x%01x\n", packet->RTANumEventsActive6);
  printf("packet->RTANumEventsActive5 = 0x%01x\n", packet->RTANumEventsActive5);
  printf("packet->RTANumEventsActive4 = 0x%01x\n", packet->RTANumEventsActive4);
  printf("packet->RTANumEventsActive3 = 0x%01x\n", packet->RTANumEventsActive3);
  printf("packet->RTANumEventsActive2 = 0x%01x\n", packet->RTANumEventsActive2);
  printf("packet->RTANumEventsActive1 = 0x%01x\n", packet->RTANumEventsActive1);
  printf("packet->RTANumEventsActive0 = 0x%01x\n", packet->RTANumEventsActive0);
  printf("packet->RTAAnalysisEnabled3 = 0x%01x\n", packet->RTAAnalysisEnabled3);
  printf("packet->RTAAnalysisEnabled2 = 0x%01x\n", packet->RTAAnalysisEnabled2);
  printf("packet->RTAAnalysisEnabled1 = 0x%01x\n", packet->RTAAnalysisEnabled1);
  printf("packet->RTAAnalysisEnabled0 = 0x%01x\n", packet->RTAAnalysisEnabled0);
  printf("packet->RTANumEventsActive11 = 0x%01x\n", packet->RTANumEventsActive11);
  printf("packet->RTANumEventsActive10 = 0x%01x\n", packet->RTANumEventsActive10);
  printf("packet->RTANumEventsActive9 = 0x%01x\n", packet->RTANumEventsActive9);
  printf("packet->RTANumEventsActive8 = 0x%01x\n", packet->RTANumEventsActive8);
  printf("packet->RTAAnalysisEnabled11 = 0x%01x\n", packet->RTAAnalysisEnabled11);
  printf("packet->RTAAnalysisEnabled10 = 0x%01x\n", packet->RTAAnalysisEnabled10);
  printf("packet->RTAAnalysisEnabled9 = 0x%01x\n", packet->RTAAnalysisEnabled9);
  printf("packet->RTAAnalysisEnabled8 = 0x%01x\n", packet->RTAAnalysisEnabled8);
  printf("packet->RTAAnalysisEnabled7 = 0x%01x\n", packet->RTAAnalysisEnabled7);
  printf("packet->RTAAnalysisEnabled6 = 0x%01x\n", packet->RTAAnalysisEnabled6);
  printf("packet->RTAAnalysisEnabled5 = 0x%01x\n", packet->RTAAnalysisEnabled5);
  printf("packet->RTAAnalysisEnabled4 = 0x%01x\n", packet->RTAAnalysisEnabled4);
  printf("packet->GRBLastTrigTime = 0x%08x\n", packet->GRBLastTrigTime);
  printf("packet->ShieldNumAlgoTriggers0 = 0x%02x\n", packet->ShieldNumAlgoTriggers0);
  printf("packet->ShieldNumAlgoTriggers1 = 0x%02x\n", packet->ShieldNumAlgoTriggers1);
  printf("packet->ShieldNumAlgoTriggers2 = 0x%02x\n", packet->ShieldNumAlgoTriggers2);
  printf("packet->WhichSIP = 0x%01x\n", packet->WhichSIP);
  printf("packet->TelemUDPMode = 0x%01x\n", packet->TelemUDPMode);
  printf("packet->SIPLon = 0x%04x\n", packet->SIPLon);
  printf("packet->SIPLat = 0x%04x\n", packet->SIPLat);
  printf("packet->SIPAlt = 0x%04x\n", packet->SIPAlt);
  printf("packet->SIPTime = 0x%08x\n", packet->SIPTime);
  printf("packet->SIPWeek = 0x%04x\n", packet->SIPWeek);
  printf("packet->SIPPressure = 0x%04x\n", packet->SIPPressure);
  printf("packet->SIPHealth = 0x%04x\n", packet->SIPHealth);
  printf("packet->FlightCodeCPUUsage = 0x%02x\n", packet->FlightCodeCPUUsage);
  printf("packet->FlightCodeMemUsage = 0x%02x\n", packet->FlightCodeMemUsage);
  printf("packet->MagredCPUUsage = 0x%02x\n", packet->MagredCPUUsage);
  printf("packet->MagredMemUsage = 0x%02x\n", packet->MagredMemUsage);
  printf("packet->EthernetTotalNumRawDataframes = 0x%08x\n", packet->EthernetTotalNumRawDataframes);
  printf("packet->EthernetTotalNumComptonDataframes = 0x%08x\n", packet->EthernetTotalNumComptonDataframes);
  printf("packet->DoublePumpOverrideEnabled = 0x%01x\n", packet->DoublePumpOverrideEnabled);
  printf("packet->PumpEPromCodeGood = 0x%01x\n", packet->PumpEPromCodeGood);
  printf("packet->PumpTachAValid = 0x%01x\n", packet->PumpTachAValid);
  printf("packet->PumpTachBValid = 0x%01x\n", packet->PumpTachBValid);
  printf("packet->PumpDACASetting = 0x%02x\n", packet->PumpDACASetting);
  printf("packet->PumpDACBSetting = 0x%02x\n", packet->PumpDACBSetting);
  printf("packet->PumpTachALast = 0x%04x\n", packet->PumpTachALast);
  printf("packet->PumpTachAHigh = 0x%04x\n", packet->PumpTachAHigh);
  printf("packet->PumpTachALow = 0x%04x\n", packet->PumpTachALow);
  printf("packet->PumpTachBLast = 0x%04x\n", packet->PumpTachBLast);
  printf("packet->PumpTachBHigh = 0x%04x\n", packet->PumpTachBHigh);
  printf("packet->PumpTachBLow = 0x%04x\n", packet->PumpTachBLow);
  printf("packet->PumpLevelSensor = 0x%04x\n", packet->PumpLevelSensor);
  printf("packet->ShieldNumSamples = 0x%02x\n", packet->ShieldNumSamples);
  shieldSample : for (i = 0; i < packet->ShieldNumSamples; i += 1) {
    printf("packet->NumCounts[%0d] = 0x%08x\n", i, packet->NumCounts[i]);
    printf("packet->TimeInterval[%0d] = 0x%08x\n", i, packet->TimeInterval[i]);
    printf("packet->LiveTimeFraction[%0d] = 0x%02x\n", i, packet->LiveTimeFraction[i]);
  }
}

void getGCUHousekeepingPacketAllocatedSizes(struct GCUHousekeepingPacket* packet, uint16_t** sizesPtr) {
  uint16_t* sizes;
  
  sizes = (uint16_t*)malloc(9*sizeof(uint16_t));
  sizes[0] = (uint16_t)32;
  sizes[1] = (uint16_t)32;
  sizes[2] = (uint16_t)3;
  sizes[3] = (uint16_t)3;
  sizes[4] = (uint16_t)12;
  sizes[5] = (uint16_t)6;
  sizes[6] = (uint16_t)packet->ShieldNumSamples;
  sizes[7] = (uint16_t)packet->ShieldNumSamples;
  sizes[8] = (uint16_t)packet->ShieldNumSamples;
  
  *sizesPtr = sizes;
}
