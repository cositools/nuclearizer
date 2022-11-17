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

#ifndef GCUSETTINGSPACKET_H
#define GCUSETTINGSPACKET_H

#include <stdint.h>

struct GCUSettingsPacket {
  uint16_t Sync;
  uint8_t PacketID;
  uint32_t UnixTime;
  uint16_t PacketCounter;
  uint16_t PacketSize;
  uint16_t OPAHeartBeatTimeout;
  uint16_t OPBHeartBeatTimeout;
  uint32_t LOSMaxQueueSize;
  uint32_t DroppedMaxQueueSize;
  uint32_t RelayMaxQueueSize;
  uint32_t DiskMaxQueueSize;
  uint32_t DiskMaxFileSize;
  uint32_t RTAMaxQueueSize;
  uint32_t RTAMaxComptonQueueSize;
  uint16_t RTAMaxNumTriggers;
  uint8_t RTAComptonWindow;
  uint32_t GRBMaxQueueSize;
  uint32_t GRBQueueTDiff;
  uint16_t ShieldSampleRate100ms;
  uint16_t HkpShieldInterval;
  uint16_t* ShieldAlgoInterval;
  uint16_t* ShieldTriggerSigma;
  uint16_t* GRBAlgoMinSamples;
  uint16_t* GRBAlgoMaxSamples;
  uint16_t GPSSampleRate100ms;
  uint16_t GPSDiagInterval;
  uint16_t HkpPeriod100ms;
  uint8_t CryoPID;
  uint16_t CryoTTarget;
  uint16_t CryoPWOut;
  uint16_t CryoMax;
  uint16_t OPAUDPByteLimit;
  uint16_t OPBUDPByteLimit;
  uint16_t RpiTemp_Brd0_Ch0;
  uint16_t RpiTemp_Brd0_Ch1;
  uint16_t RpiTemp_Brd0_Ch2;
  uint16_t RpiTemp_Brd0_Ch3;
  uint16_t RpiTemp_Brd0_Ch4;
  uint16_t RpiTemp_Brd0_Ch5;
  uint16_t RpiTemp_Brd0_Ch6;
  uint16_t RpiTemp_Brd0_Ch7;
  uint16_t RpiTemp_Brd1_Ch0;
  uint16_t RpiTemp_Brd1_Ch1;
  uint16_t RpiTemp_Brd1_Ch2;
  uint16_t RpiTemp_Brd1_Ch3;
  uint16_t RpiTemp_Brd1_Ch4;
  uint16_t RpiTemp_Brd1_Ch5;
  uint16_t RpiTemp_Brd1_Ch6;
  uint16_t RpiTemp_Brd1_Ch7;
  uint16_t RpiTemp_Brd2_Ch0;
  uint16_t RpiTemp_Brd2_Ch1;
  uint16_t RpiTemp_Brd2_Ch2;
  uint16_t RpiTemp_Brd2_Ch3;
  uint16_t RpiTemp_Brd2_Ch4;
  uint16_t RpiTemp_Brd2_Ch5;
  uint16_t RpiTemp_Brd2_Ch6;
  uint16_t RpiTemp_Brd2_Ch7;
  uint16_t RpiTemp_Brd3_Ch0;
  uint16_t RpiTemp_Brd3_Ch1;
  uint16_t RpiTemp_Brd3_Ch2;
  uint16_t RpiTemp_Brd3_Ch3;
  uint16_t RpiTemp_Brd3_Ch4;
  uint16_t RpiTemp_Brd3_Ch5;
  uint16_t RpiTemp_Brd3_Ch6;
  uint16_t RpiTemp_Brd3_Ch7;
  uint16_t MagPiX;
  uint16_t MagPiY;
  uint16_t MagPiZ;
  uint16_t TStat0LowerTemp;
  uint16_t TStat0UpperTemp;
  uint16_t TStat1LowerTemp;
  uint16_t TStat1UpperTemp;
  uint16_t TStat2LowerTemp;
  uint16_t TStat2UpperTemp;
  uint16_t TStat3LowerTemp;
  uint16_t TStat3UpperTemp;
  uint16_t TStat4LowerTemp;
  uint16_t TStat4UpperTemp;
  uint16_t TStat5LowerTemp;
  uint16_t TStat5UpperTemp;
  uint16_t TStat6LowerTemp;
  uint16_t TStat6UpperTemp;
  uint16_t TStat7LowerTemp;
  uint16_t TStat7UpperTemp;
};

struct GCUSettingsPacket* MakeNewGCUSettingsPacket(void);
struct GCUSettingsPacket* ParseGCUSettingsPacket(uint8_t* RawData);
void ParseAllocatedGCUSettingsPacket(uint8_t* RawData, struct GCUSettingsPacket* packet);
void ParseGCUSettingsPacketWrapper(uint8_t* RawData, void* packet_bytes);
void printGCUSettingsPacket(struct GCUSettingsPacket* packet);

#endif
