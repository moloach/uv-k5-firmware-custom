/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifndef APP_FM_H
#define APP_FM_H

#ifdef ENABLE_FMRADIO

#include "driver/keyboard.h"

#define FM_CHANNEL_UP	0x01
#define FM_CHANNEL_DOWN	0xFF

// number of storable FM presets
#define FM_CHANNEL_COUNT        50u

// EEPROM layout of the preset frequencies (uint16, units of 10kHz):
//   CH  1..20 -> 0x0E40..0x0E67, kept at the stock location so the radio
//               stays compatible with the PC programming software
//   CH 21..50 -> 0x02028..0x02063, the free gap between the welcome text
//               block (ends at 0x02025) and the power-on picture (0x02080)
//               nothing else in the firmware or the PC tools uses this area
#define FM_EEPROM_BLOCK0_ADDR   0x0E40u
#define FM_EEPROM_BLOCK0_COUNT  20u
#define FM_EEPROM_BLOCK1_ADDR   0x02028u
#define FM_EEPROM_BLOCK1_COUNT  (FM_CHANNEL_COUNT - FM_EEPROM_BLOCK0_COUNT)

enum {
    FM_SCAN_OFF = 0U,
};
extern bool              FM_EXIT_FLAG;

extern uint16_t          gFM_Channels[FM_CHANNEL_COUNT];
extern bool              gFmRadioMode;
extern uint8_t           gFmRadioCountdown_500ms;
extern volatile uint16_t gFmPlayCountdown_10ms;
extern volatile int8_t   gFM_ScanState;
extern uint8_t           gFM_ChannelPosition;
// counts down (500ms ticks) after a key press while FM RSSI info is refreshed
extern uint8_t           gFM_RssiRefreshCountdown_500ms;
// Doubts about          whether this should be signed or not
extern uint16_t          gFM_FrequencyDeviation;
extern bool              gFM_FoundFrequency;
extern bool              gFM_AutoScan;
extern uint16_t          gFM_RestoreCountdown_10ms;

bool    FM_CheckValidChannel(uint8_t Channel);
uint8_t FM_FindNextChannel(uint8_t Channel, uint8_t Direction);
int     FM_ConfigureChannelState(void);
void    FM_TurnOff(void);
void    FM_EraseChannels(void);
void    FM_LoadChannels(void);
void    FM_SaveChannels(void);

void    FM_Tune(uint16_t Frequency, int8_t Step, bool bFlag);
void    FM_PlayAndUpdate(void);
int     FM_CheckFrequencyLock(uint16_t Frequency, uint16_t LowerLimit);
void    FM_GetSignalQuality(uint8_t *pRssi, uint8_t *pSnr);

void    FM_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);

void    FM_Play(void);
void    FM_Start(void);

#endif

#endif

