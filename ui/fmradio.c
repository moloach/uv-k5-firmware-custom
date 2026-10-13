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

#ifdef ENABLE_FMRADIO

#include <string.h>

#include "app/fm.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/fmradio.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"
#include "chinese.h"

// signal strength info: row 1 = RSSI/SNR text, last row = level bar
static void FM_DrawSignalInfo(void)
{
    static uint8_t  smoothRssi;
    static uint8_t  lastSnr;
    static uint16_t lastFreq;
    char     buf[20];

    // only poll the BK1080 while scanning or shortly after a key press:
    // continuous register reads can inject clicks into the FM audio
    if (gFM_ScanState != FM_SCAN_OFF || gFM_RssiRefreshCountdown_500ms > 0) {
        uint8_t rssi, snr;

        FM_GetSignalQuality(&rssi, &snr);
        lastSnr = snr;

        if (gEeprom.FM_FrequencyPlaying != lastFreq) {
            lastFreq    = gEeprom.FM_FrequencyPlaying;
            smoothRssi  = rssi;
        } else {
            smoothRssi += (rssi - smoothRssi) / 2;
        }
    }

    sprintf(buf, "RSSI %3u  SNR %2u", smoothRssi, lastSnr);
    UI_PrintStringSmall(buf, 0, 127, 1);

    // bottom line: hollow bar frame, filled proportionally to the RSSI
    // (kept well inside the visible glass area, the LCD has hidden columns)
    const uint8_t barStart = 16;
    const uint8_t barEnd   = 111;    // 96 pixels wide
    uint8_t *pLine = gFrameBuffer[6];
    memset(pLine, 0, 128);
    for (uint8_t x = barStart; x <= barEnd; x++)
        pLine[x] = 0x81;

    const uint8_t width = ((uint16_t) smoothRssi * 96) / 255;
    for (uint8_t x = 0; x < width; x++)
        pLine[barStart + x] = 0xFF;
}

void UI_DisplayFM(void)
{
    char String[16] = {0};
    char *pPrintStr = String;
UI_DisplayClear();
    FM_DrawSignalInfo();
    UI_PrintStringSmall("FM", 0, 127, 0);

    if (gAskToSave) {
        pPrintStr = 存置问;
    } else if (gAskToDelete) {
        pPrintStr = 删除问;
    } else if (gFM_ScanState == FM_SCAN_OFF) {
        if (gEeprom.FM_IsMrMode) {
            sprintf(String, "MR(CH%02u)", gEeprom.FM_SelectedChannel + 1);
            pPrintStr = String;
        } else {
            pPrintStr = "VFO";
            for (unsigned int i = 0; i < FM_CHANNEL_COUNT; i++) {
                if (gEeprom.FM_FrequencyPlaying == gFM_Channels[i]) {
                    sprintf(String, "VFO(CH%02u)", i + 1);
                    pPrintStr = String;
                    break;
                }
            }
        }
    } else if (gFM_AutoScan) {
        sprintf(String, "A-SCAN(%u)", gFM_ChannelPosition + 1);
        pPrintStr = String;
    } else {
        pPrintStr = "M-SCAN";
    }

    UI_PrintStringSmall(pPrintStr, 0, 127, 2);

    memset(String, 0, sizeof(String));
    if (gAskToSave || (gEeprom.FM_IsMrMode && gInputBoxIndex > 0)) {
        UI_GenerateChannelString(String, gFM_ChannelPosition);
    } else if (gAskToDelete) {
        sprintf(String, "CH-%02u", gEeprom.FM_SelectedChannel + 1);
    } else {
        if (gInputBoxIndex == 0) {
            sprintf(String, "%3d.%d", gEeprom.FM_FrequencyPlaying / 10, gEeprom.FM_FrequencyPlaying % 10);
        } else {
            const char * ascii = INPUTBOX_GetAscii();
            sprintf(String, "%.3s.%.1s",ascii, ascii + 3);
        }

        UI_DisplayFrequency(String, 32, 4, gInputBoxIndex == 0);
        ST7565_BlitFullScreen();
        return;
    }

    UI_PrintStringSmall(String, 0, 127, 4);

    ST7565_BlitFullScreen();
}

#endif
