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
#include "driver/bk1080.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/fmradio.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"


void UI_DisplayFM(void)
{
	char String[16] = {0};
	char *pPrintStr = String;
	UI_DisplayClear();

	UI_PrintString("FM", 2, 0, 0, 8);

	sprintf(String, "%d%s-%dM", 
		BK1080_GetFreqLoLimit(gEeprom.FM_Band)/10,
		gEeprom.FM_Band == 0 ? ".5" : "",
		BK1080_GetFreqHiLimit(gEeprom.FM_Band)/10
		);
	
	UI_PrintStringSmallNormal(String, 1, 0, 6);

	//uint8_t spacings[] = {20,10,5};
	//sprintf(String, "%d0k", spacings[gEeprom.FM_Space % 3]);
	//UI_PrintStringSmallNormal(String, 127 - 4*7, 0, 6);

	if (gAskToSave) {
		pPrintStr = "SAVE?";
	} else if (gAskToDelete) {
		pPrintStr = "DEL?";
	} else if (gFM_ScanState == FM_SCAN_OFF) {
		if (gEeprom.FM_IsMrMode) {
			sprintf(String, "MR(CH%02u)", gEeprom.FM_SelectedChannel + 1);
			pPrintStr = String;
		} else {
			pPrintStr = "VFO";
			for (unsigned int i = 0; i < 20; i++) {
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

	UI_PrintStringSmallNormal(pPrintStr, 0, 127, 3); // memory, vfo, scan      ; fonte + petite @PBA v1.6

	const uint16_t val_07 = BK1080_ReadRegister(0x07);   // affiche RSSI, ST mode, SNR  @PBA v1.3
	const uint16_t val_0A = BK1080_ReadRegister(0x0A);
	const uint8_t rssi_value = val_0A & 0x00ff;			// RSSI value					@PBA v1.6c
	sprintf(String, "%2u/%2u%s",
		rssi_value,							// RSSI en dBµV, max 75dBµV (datasheet) -> 95		@PBA v1.6
		val_07 & 0x000f,					// SNR en dB, max 15
		((val_0A >> 8) & 1u) ? "S" : "m");	// Stéréo mode ou mono	
	UI_PrintStringSmallNormal(String, 84, 127, 6);		// @PBA v1.5
// Affichage barre RSSI sur la ligne 5					@PBA v1.6c
	const uint8_t rssi_bars = MIN(8, rssi_value / 10); 	// On convertit le RSSI en nombre de barres (max 8 barres) 8/80
	uint8_t *p_line = gFrameBuffer[5];					// ligne 5
	memset(p_line, 0, LCD_WIDTH);						// On efface la ligne 5
	for(uint8_t i = 0; i < rssi_bars; i++) {
			for(uint8_t j = 0; j < 4; j++)
			p_line[88 + i * 5 + j] = (~(0x7F >> (i+1))) & 0x7F;   		// xpos 88
	}


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

		UI_DisplayFrequency(String, 36, 1, gInputBoxIndex == 0);  // frequency
		ST7565_BlitFullScreen();
		return;
	}

	UI_PrintString(String, 0, 127, 1, 10);
	
	ST7565_BlitFullScreen();
}

#endif
