// --------------------------------------------------------------
//
//  Thinkpad Fan Control
//
// --------------------------------------------------------------
//
//	This program and source code is in the public domain.
//
//	The author claims no copyright, copyleft, license or
//	whatsoever for the program itself (with exception of
//	WinIO driver).  You may use, reuse or distribute it's 
//	binaries or source code in any desired way or form,  
//	Useage of binaries or source shall be entirely and 
//	without exception at your own risk. 
// 
// --------------------------------------------------------------

#include "_prec.h"
#include "fancontrol.h"
#include "tools.h"
#include "TVicPort.h"

#define TP_ECOFFSET_FAN         (char)0x2F    // 1 byte (binary xyzz zzz)
#define TP_ECOFFSET_FANSPEED    (char)0x84    // 16 bit word, lo/hi byte
#define TP_ECOFFSET_TEMP0       (char)0x78    // 8 temp sensor bytes from here
#define TP_ECOFFSET_TEMP1       (char)0xC0    // 4 temp sensor bytes from here
#define TP_ECOFFSET_FAN_SWITCH  (char)0x31
#define TP_ECVALUE_SELFAN1      (char)0x0000
#define TP_ECVALUE_SELFAN2      (char)0x0001

//-------------------------------------------------------------------------
//  switch fan according to settings
//-------------------------------------------------------------------------
int
FANCONTROL::HandleData(void) {
	char obuf[256] = "", obuf2[128] = "",
		templist[256] = "", templist2[512],
		manlevel[16] = "", title2[128] = "";
	int i, maxtemp, imaxtemp, ok = 0;

	//
	// determine highest temp.
	//

	// build a list of sensors to ignore, separated by "|", e.g. "|XC1|BAT|CPU|"
	char what[16], list[128];
	sprintf_s(list, sizeof(list), "|%s|", this->IgnoreSensors);
	for (i = 0; list[i] != '\0'; i++) {
		if (list[i] == ',')
			list[i] = '|';
	}

	maxtemp = 0;
	imaxtemp = 0;
	int senstemp;
	for (i = 0; i < 12; i++) {
		sprintf_s(what, sizeof(what), "|%s|", this->State.SensorName[i]); // name (e.g. "|CPU|") to match against list above

		if (this->State.Sensors[i] != 0x80 && this - State.Sensors[i] != 0x00 && strstr(list, what) == 0) {
			int isens = this->State.Sensors[i];
			int ioffs = this->SensorOffset[i].offs;

			// do not apply offset if inside of temp range
			int calcTemp = isens - SensorOffset[i].offs;
			if (isens >= SensorOffset[i].hystMin && isens <= SensorOffset[i].hystMax)
				ioffs = 0;

			if (ShowBiasedTemps)
				senstemp = isens - ioffs;
			else
				senstemp = isens;

			if (senstemp < 128) {
				maxtemp = __max(senstemp, maxtemp);
				if (maxtemp <= senstemp)
					imaxtemp = i;
			}
		}
	}

	this->MaxTemp = maxtemp;
	this->iMaxTemp = imaxtemp;

	//
	// update dialog elements
	//

	// title string (for minimized window)
	if (Fahrenheit)
		sprintf_s(title2, sizeof(title2), "%d° F", this->MaxTemp * 9 / 5 + 32);
	else
		sprintf_s(title2, sizeof(title2), "%d° C", this->MaxTemp);

	// display fan state
	int fanctrl = this->State.FanCtrl;
	fanctrl2 = fanctrl;

	if (this->SlimDialog == 1) {
		sprintf_s(obuf2, sizeof(obuf2), "Fan %d ", fanctrl);
		if (fanctrl & 0x80) {
			if (!(SlimDialog && StayOnTop))
				strcat_s(obuf2, sizeof(obuf2), "(= BIOS)");
			strcat_s(title2, sizeof(title2), " Default Fan");
		}
		else {
			if (!(SlimDialog && StayOnTop))
				sprintf_s(obuf2 + strlen(obuf2), sizeof(obuf2) - strlen(obuf2), " Non Bios");
			sprintf_s(title2 + strlen(title2), sizeof(title2) - strlen(title2), " Fan %d (%s)",	fanctrl & 0x3F,	this->CurrentModeFromDialog() == 2 ? "Smart" : "Fixed");
		}
	}
	else {
		sprintf_s(obuf2, sizeof(obuf2), "0x%02x (", fanctrl);
		if (fanctrl & 0x80) {
			strcat_s(obuf2, sizeof(obuf2), "BIOS Controlled)");
			strcat_s(title2, sizeof(title2), " Default Fan");
		}
		else {
			sprintf_s(obuf2 + strlen(obuf2), sizeof(obuf2) - strlen(obuf2), "Fan Level %d, Non Bios)", fanctrl & 0x3F);
			sprintf_s(title2 + strlen(title2), sizeof(title2) - strlen(title2), " Fan %d (%s)",	fanctrl & 0x3F,	this->CurrentModeFromDialog() == 2 ? "Smart" : "Fixed");
		}
	}

	::SetDlgItemText(this->hwndDialog, 8100, obuf2);

	strcpy_s(this->Title2, sizeof(this->Title2), title2);

	// display fan speeds
	this->lastfan1speed = this->fan1speed;
	this->fan1speed = (this->State.Fan1SpeedHi << 8) | this->State.Fan1SpeedLo;
	if (this->fan1speed > 0x1fff)
		fan1speed = lastfan1speed;

	this->lastfan2speed = this->fan2speed;
	this->fan2speed = (this->State.Fan2SpeedHi << 8) | this->State.Fan2SpeedLo;
	if (this->fan2speed > 0x1fff)
		fan2speed = lastfan2speed;

	sprintf_s(obuf2, sizeof(obuf2), "%d/%d RPM", this->fan1speed, this->fan2speed);

	::SetDlgItemText(this->hwndDialog, 8102, obuf2);

	// display temperature list
	if (Fahrenheit)
		sprintf_s(obuf2, sizeof(obuf2), "%d° F", this->MaxTemp * 9 / 5 + 32);
	else
		sprintf_s(obuf2, sizeof(obuf2), "%d° C", this->MaxTemp);

	::SetDlgItemText(this->hwndDialog, 8103, obuf2);

	strcpy_s(templist2, sizeof(templist2), "");

	for (i = 0; i < 12; i++) {
		int temp = this->State.Sensors[i];

		if (temp != 0 && temp < 128) {
			if (Fahrenheit)
				sprintf_s(obuf2, sizeof(obuf2), "%d° F", temp * 9 / 5 + 32);
			else
				sprintf_s(obuf2, sizeof(obuf2), "%d° C", temp);

			if (SlimDialog && StayOnTop)
				sprintf_s(templist2 + strlen(templist2), sizeof(templist2) - strlen(templist2), "%d %s %s", i + 1, this->State.SensorName[i], obuf2);
			else
				sprintf_s(templist2 + strlen(templist2), sizeof(templist2) - strlen(templist2), "%d %s %s (0x%02x)", i + 1, this->State.SensorName[i], obuf2, this->State.SensorAddr[i]);

			strcat_s(templist2, sizeof(templist2), "\r\n");
		}
		else {
			if (this->ShowAll == 1) {
				sprintf_s(obuf2, sizeof(obuf2), "n/a");

				size_t strlen_templist = strlen_s(templist2, sizeof(templist2));

				if (SlimDialog && StayOnTop)
					sprintf_s(templist2 + strlen_templist, sizeof(templist2) - strlen_templist, "%d %s %s", i + 1, this->State.SensorName[i], obuf2);
				else
					sprintf_s(templist2 + strlen_templist, sizeof(templist2) - strlen_templist, "%d %s %s (0x%02x)", i + 1, this->State.SensorName[i], obuf2, this->State.SensorAddr[i]);

				strcat_s(templist2, sizeof(templist2), "\r\n");
			}
		}
	}

	::SetDlgItemText(this->hwndDialog, 8101, templist2);

	this->icontemp = this->State.Sensors[iMaxTemp];

	// compact single line status (combined)
	strcpy_s(templist, sizeof(templist), "");

	if (Fahrenheit) {
		for (i = 0; i < 12; i++) {
			if (this->State.Sensors[i] < 128) {
				if (this->State.Sensors[i] != 0)
					sprintf_s(templist + strlen(templist), sizeof(templist) - strlen(templist), "%d;", this->State.Sensors[i] * 9 / 5 + 32);
				else
					sprintf_s(templist + strlen(templist), sizeof(templist) - strlen(templist), "%d;", 0);
			}
			else {
				strcat_s(templist, sizeof(templist), "0;");
			}
		}
	}
	else {
		for (i = 0; i < 12; i++) {
			if (this->State.Sensors[i] != 128) {
				sprintf_s(templist + strlen(templist), sizeof(templist) - strlen(templist), "%d; ", this->State.Sensors[i]);
			}
			else {
				strcat_s(templist, sizeof(templist), "0; ");
			}
		}
	}

	templist[strlen(templist) - 1] = '\0';

	if (Fahrenheit)
		sprintf_s(CurrentStatus, sizeof(CurrentStatus), "Fan: 0x%02x / Switch: %d° F (%s)", State.FanCtrl, MaxTemp * 9 / 5 + 32, templist);
	else
		sprintf_s(CurrentStatus, sizeof(CurrentStatus), "Fan: 0x%02x / Switch: %d° C (%s)", State.FanCtrl, MaxTemp,	templist);

	// display fan speed

	if (fan1speed > 0x1fff)
		fan1speed = lastfan1speed;
	if (fan2speed > 0x1fff)
		fan2speed = lastfan2speed;
	sprintf_s(obuf2, sizeof(obuf2), "%d/%d", this->fan1speed, this->fan2speed);

	sprintf_s(CurrentStatuscsv, sizeof(CurrentStatuscsv), "%s %s; %d; %d; ", templist, obuf2, State.FanCtrl, MaxTemp);

	::SetDlgItemText(this->hwndDialog, 8112, this->CurrentStatus);

	//
	// handle fan control according to mode
	//

	this->CurrentModeFromDialog();
	this->ShowAllFromDialog();

	switch (this->CurrentMode) {

	case 1: // BIOS
		if (this->PreviousMode != this->CurrentMode) {
			sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Change Mode from ");

			if (this->PreviousMode == 1)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "BIOS->");
			if (this->PreviousMode == 2)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Smart->");
			if (this->PreviousMode == 3)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Manual->");

			if (this->CurrentMode == 1)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "BIOS, setting fan speed");
			if (this->CurrentMode == 2)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Smart, recalculate fan speed");
			if (this->CurrentMode == 3)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Manual, setting fan speed");

			this->Trace(obuf);
		}

		if (this->State.FanCtrl != 0x080)
			ok = this->SetFan("BIOS", 0x80);
		break;

	case 2: // Smart
		this->SmartControl();
		break;

	case 3: // Manual
		if (this->PreviousMode != this->CurrentMode) {
			sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Change Mode from ");

			if (this->PreviousMode == 1)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "BIOS->");
			if (this->PreviousMode == 2)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Smart->");
			if (this->PreviousMode == 3)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Manual->");

			if (this->CurrentMode == 1)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "BIOS, setting fan speed");
			if (this->CurrentMode == 2)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Smart, recalculate fan speed");
			if (this->CurrentMode == 3)
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Manual, setting fan speed");

			this->Trace(obuf);
		}

		::GetDlgItemText(this->hwndDialog, 8310, manlevel, sizeof(manlevel));

		if (isdigit(manlevel[0]) && atoi(manlevel) >= 0 && atoi(manlevel) <= 255) {
			if (this->State.FanCtrl != atoi(manlevel))
				ok = this->SetFan("Manual", atoi(manlevel));
			else
				ok = true;
		}

		break;
	}

	this->PreviousMode = this->CurrentMode;

	if (this->CurrentMode == 3 && this->MaxTemp > this->ManModeExitInternal)
		this->CurrentMode = 2;

	return ok;
}

//-------------------------------------------------------------------------
//  smart fan control depending on temperature
//-------------------------------------------------------------------------
void
FANCONTROL::SmartControl(void) {
	int i,
		newfanctrl = -1,
		levelIndex = -1,
		fanctrl = this->State.FanCtrl;
	char obuf[256] = "";

	if (this->PreviousMode == 1) {
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Change Mode from BIOS->");
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Smart, recalculate fan speed");

		this->Trace(obuf);
	}

	if (this->PreviousMode == 3) {
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Change Mode from Manual->");
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Smart, recalculate fan speed");

		this->Trace(obuf);
	}

//i         Temp Fan Hup Hdown 
//0 Level = 50   0   0   0 
//1 Level = 60   1   0   5 <--- means, when going down switch to this level at 55
//2 Level = 70   2   0   0 
//3 Level = 80   4   5   0 <--- means, when going up, switch this level at 85
//4 Level = 90   7   0   0 
//5 Level = 95   64  0   0 
//6 Level = 105 128  0   0 

	newfanctrl = -1;

	if ((fanctrl > 7 && (fanctrl != 64 || !Lev64Norm)) || this->PreviousMode == 3 || this->PreviousMode == 1) {
		newfanctrl = 0;
		levelIndex = 0;
		fanctrl = 0;
	}

	// Check for fan speed ramp upwards
	for (i = 0; this->SmartLevels[i].temp != -1; i++) {
		if (this->MaxTemp >= this->SmartLevels[i].temp && this->SmartLevels[i].fan >= fanctrl) {
			newfanctrl = this->SmartLevels[i].fan;
			levelIndex = i;
		}
	}

	// Check for fan speed ramp downwards
	if (newfanctrl == -1) {
		for (i = 0; this->SmartLevels[i].temp != -1; i++) {
			if (this->MaxTemp <= this->SmartLevels[i].temp && this->SmartLevels[i].fan < fanctrl) {
				newfanctrl = this->SmartLevels[i].fan;
				levelIndex = i;
				break;
			}
		}
	}

	// fan speed ramp up or down?
	if (newfanctrl != -1 && newfanctrl != this->State.FanCtrl) {
		//if (newfanctrl == 0x80) {  
		    // switch to BIOS-auto mode
		//	this->ModeToDialog(1);    
		//}

		// do not change if hyst zone, determine which hyst zone if we are in based on previous temp
		// DO NOT HAVE HYSTERESIS OVERLAP WITH FAN TEMPS IN CONFIG!
		SMARTENTRY newLevel = this->SmartLevels[levelIndex];
		if (this->LastSmartLevel < 0) { // ignore hyst on first time setting fan
			this->LastSmartLevel = levelIndex;
			this->SetFan("Smart", newfanctrl);
			return;
		}

		if (this->MaxTemp < this->SmartLevels[this->LastSmartLevel].temp) {
			if (this->MaxTemp > newLevel.temp - newLevel.hystDown)
				return; // cooling
		}
		else {
			if (this->MaxTemp < newLevel.temp + newLevel.hystUp)
				return; // rising 
		}

		this->LastSmartLevel = levelIndex; 
		this->SetFan("Smart", newfanctrl);
	}

	return;
}

//-------------------------------------------------------------------------
//  set fan state via EC
//-------------------------------------------------------------------------
int
FANCONTROL::SetFan(const char* source, int fanctrl, BOOL final) {
	int ok = 0;
	int fan1_ok = 0;
	int fan2_ok = 0;
	char obuf[256] = "", obuf2[256], datebuf[128];

	if (this->FanBeepFreq && this->FanBeepDura)
		::Beep(this->FanBeepFreq, this->FanBeepDura);

	this->CurrentDateTimeLocalized(datebuf, sizeof(datebuf));

	sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "%s: Set fan control to 0x%02x, ", source, fanctrl);
	if (this->IndSmartLevel == 1 && this->SmartLevels2[0].temp2 != 0 && source == "Smart")
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Mode 2, ");
	if (this->IndSmartLevel == 0 && this->SmartLevels2[0].temp2 != 0 && source == "Smart")
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Mode 1, ");
	sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Result: ");

	if (this->ActiveMode && !this->FinalSeen) {
		int ok_ecaccess = false;
		for (int i = 0; i < 10; i++) {
			if (ok_ecaccess = this->EcAccess.Lock(100))
				break;
			else
				::Sleep(100);
		}
		if (!ok_ecaccess) {
			this->Trace("Could not acquire mutex to set fan state");
			return 0;
		}

		for (int i = 0; i < 5; i++) {
			// set new fan level
			ok = this->WriteByteToEC(TP_ECOFFSET_FAN_SWITCH, TP_ECVALUE_SELFAN1);
			ok = this->WriteByteToEC(TP_ECOFFSET_FAN, fanctrl);

			::Sleep(100);

			ok = this->WriteByteToEC(TP_ECOFFSET_FAN_SWITCH, TP_ECVALUE_SELFAN2);
			ok = this->WriteByteToEC(TP_ECOFFSET_FAN, fanctrl);

			::Sleep(100);

			// verify completion of fan2
			fan2_ok = this->ReadByteFromEC(TP_ECOFFSET_FAN, &this->State.FanCtrl);

			::Sleep(100);

			// verify completion of fan1
			ok = this->WriteByteToEC(TP_ECOFFSET_FAN_SWITCH, TP_ECVALUE_SELFAN1);
			//ok = this->WriteByteToEC(TP_ECOFFSET_FAN, fanctrl);

			::Sleep(100);

			fan1_ok = this->ReadByteFromEC(TP_ECOFFSET_FAN, &this->State.FanCtrl);

			if (fan1_ok && fan2_ok) {
				sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "[i=%d] ", i);
				break;
			}

			::Sleep(300);
		}

		this->EcAccess.Unlock();

		if (this->State.FanCtrl == fanctrl) {
			sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "OK");
			ok = true;
			if (final)
				this->FinalSeen = true;    // prevent further changes when setting final mode
		}
		else {
			sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "FAILED!!");

			/*			::Beep(880, 300);
						::Sleep(200);
						::Beep(880, 300);
						::Sleep(200);
						::Beep(880, 300);
			*/

			ok = false;
		}
	}
	else {
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "IGNORED!(passive mode");
	}

	// display result
	sprintf_s(obuf2, sizeof(obuf2), "%s   (%s)", obuf, datebuf);

	::SetDlgItemText(this->hwndDialog, 8113, obuf2);

	this->Trace(this->CurrentStatus);
	this->Trace(obuf);

	if (!final)
		::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);

	return ok;
}

int
FANCONTROL::SetHdw(const char* source, int hdwctrl, int HdwOffset, int AnyWayBit) {
	int ok = 0;
	char obuf[256] = "", obuf2[256], datebuf[128];
	char newhdwctrl;

	int ok_ecaccess = false;
	for (int i = 0; i < 10; i++) {
		if (ok_ecaccess = this->EcAccess.Lock(100))
			break;
		else
			::Sleep(100);
	}
	if (!ok_ecaccess) {
		this->Trace("Could not acquire mutex to write EC register");
		return 0;
	}

	this->CurrentDateTimeLocalized(datebuf, sizeof(datebuf));

	for (int i = 0; i < 5; i++) {
		ok = this->ReadByteFromEC(HdwOffset, &newhdwctrl);
		if (newhdwctrl & hdwctrl) {
			ok = this->WriteByteToEC(HdwOffset, (newhdwctrl - hdwctrl) | AnyWayBit);
			hdwctrl = newhdwctrl - hdwctrl;
		}
		else {
			ok = this->WriteByteToEC(HdwOffset, (newhdwctrl + hdwctrl) | AnyWayBit);
			hdwctrl = newhdwctrl + hdwctrl;
		}

		ok = this->ReadByteFromEC(HdwOffset, &newhdwctrl);

		if (hdwctrl == newhdwctrl)
			break;

		::Sleep(300);
	}

	sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "%s: Set EC register 0x%02x to %d, ", source, HdwOffset, hdwctrl);
	sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "Result: ");

	if (hdwctrl == newhdwctrl) {
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "OK");
		ok = true;
	}
	else {
		sprintf_s(obuf + strlen(obuf), sizeof(obuf) - strlen(obuf), "COULD NOT SET HARDWARE STATE!!!!");
		ok = false;
	}


	// display result
	sprintf_s(obuf2, sizeof(obuf2), "%s   (%s)", obuf, datebuf);

	::SetDlgItemText(this->hwndDialog, 8113, obuf2);

	this->Trace(obuf);

	this->EcAccess.Unlock();

	return ok;
}

//-------------------------------------------------------------------------
//  read fan and temperatures from embedded controller
//-------------------------------------------------------------------------
int
FANCONTROL::ReadEcStatus(FCSTATE* pfcstate) {
	int ok = 0;
	FCSTATE sample1, sample2;

	// reading from the EC seems to yield erratic results at times (probably
	// due to collision with other drivers reading from the port).  So try
	// up to three times to read two samples which look ok and have matching
	// value

	int ok_ecaccess = false;
	for (int i = 0; i < 10; i++) {
		if (ok_ecaccess = this->EcAccess.Lock(100))
			break;
		else
			::Sleep(100);
	}

	if (!ok_ecaccess) {
		this->Trace("Could not acquire mutex to read EC status");
		return false;
	}

	for (int i = 0; i < 3; i++) {
		ok = this->ReadEcRaw(&sample1);
		if (ok) {
			ok = this->ReadEcRaw(&sample2);
			if (ok) {
				// match for identical fanctrl settings and temperature differences
				// of no more than 1°C
				BOOL match = sample1.FanCtrl == sample2.FanCtrl;
				for (int j = 0; j < 12 && match; j++) {
					if (sample1.Sensors[j] > 128 || sample2.Sensors[j] > 128 ||
						abs(sample1.Sensors[j] - sample2.Sensors[j]) > 1) {
						match = false;
					}
				}

				if (match) {	// looks good
					ok = true;
					break;
				}
				else {		// try again after short delay
					ok = false;
					::Sleep(200);
				}
			}
		}
	}

	this->EcAccess.Unlock();

	if (ok) {
		memcpy(pfcstate, &sample1, sizeof(*pfcstate));
	}

	return ok;
}

//-------------------------------------------------------------------------
//  read fan and temperatures from embedded controller
//-------------------------------------------------------------------------
int
FANCONTROL::ReadEcRaw(FCSTATE* pfcstate) {
	int i, idxtemp, ok = true;

	pfcstate->FanCtrl = -1;

	memset(pfcstate->Sensors, 0, sizeof(pfcstate->Sensors));

	// Status Register
	if (ReadByteFromEC(TP_ECOFFSET_FAN, &pfcstate->FanCtrl)) {
		;
	}
	else {
		this->Trace("failed to read status register from EC");
		ok = false;
	}

	if (!ok) return false;

	// Fan 2 speed registers hi/lo
	if (WriteByteToEC(TP_ECOFFSET_FAN_SWITCH, TP_ECVALUE_SELFAN2)) {
		if (ReadByteFromEC(TP_ECOFFSET_FANSPEED, &pfcstate->Fan2SpeedLo)) {
			if (ReadByteFromEC(TP_ECOFFSET_FANSPEED + 1, &pfcstate->Fan2SpeedHi)) {
				;
			}
			else {
				this->Trace("failed to read FanSpeedHighByte 2 from EC");
				ok = false;
			}
		}
		else {
			this->Trace("failed to read FanSpeedLowByte 2 from EC");
			ok = false;
		}
	}
	else {
		this->Trace("failed to select Fan 2 in EC");
		ok = false;
	}

	if (!ok) return false;

	// Fan 1 speed registers hi/lo
	if (WriteByteToEC(TP_ECOFFSET_FAN_SWITCH, TP_ECVALUE_SELFAN1)) {
		if (ReadByteFromEC(TP_ECOFFSET_FANSPEED, &pfcstate->Fan1SpeedLo)) {
			if (ReadByteFromEC(TP_ECOFFSET_FANSPEED + 1, &pfcstate->Fan1SpeedHi)) {
				;
			}
			else {
				this->Trace("failed to read FanSpeedHighByte 1 from EC");
				ok = false;
			}
		}
		else {
			this->Trace("failed to read FanSpeedLowByte 1 from EC");
			ok = false;
		}
	}
	else {
		this->Trace("failed to select Fan 1 in EC");
		ok = false;
	}

	if (!ok) return false;

	if (!this->UseTWR) {
		idxtemp = 0;

		for (i = 0; i < 8 && ok; i++) {    // temp sensors 0x78 - 0x7f
			pfcstate->SensorAddr[idxtemp] = TP_ECOFFSET_TEMP0 + i;

			pfcstate->SensorName[idxtemp] = this->gSensorNames[idxtemp];

			if (ReadByteFromEC(TP_ECOFFSET_TEMP0 + i, &pfcstate->Sensors[idxtemp])) {
				if (this->ShowBiasedTemps)
					pfcstate->Sensors[idxtemp] = pfcstate->Sensors[idxtemp] - this->SensorOffset[idxtemp].offs;
			}
			else {
				this->Trace("failed to read a TEMP0 byte from EC");
				ok = false;
			}

			idxtemp++;
		}

		for (i = 0; i < 4 && ok; i++) {    // temp sensors 0xC0 - 0xC4
			pfcstate->SensorAddr[idxtemp] = TP_ECOFFSET_TEMP1 + i;

			pfcstate->SensorName[idxtemp] = "n/a";

			if (!this->NoExtSensor) {
				pfcstate->SensorName[idxtemp] = this->gSensorNames[idxtemp];

				if (ReadByteFromEC(TP_ECOFFSET_TEMP1 + i, &pfcstate->Sensors[idxtemp])) {
					if (this->ShowBiasedTemps)
						pfcstate->Sensors[idxtemp] = pfcstate->Sensors[idxtemp] - this->SensorOffset[idxtemp].offs;
				}
				else {
					this->Trace("failed to read a TEMP1 byte from EC");
					ok = false;
				}
			}

			idxtemp++;
		}
	}
	else {
		char data = -1;
		char dataOut[16];
		int iOK = false;
		int iTimeout = 100;
		int iTimeoutBuf = 1000;
		int iTime = 0;
		int iTick = 10;
		int iNumTry = 0;

	retry:
		iNumTry++;

		if (iNumTry >= 3) {
			this->Trace("failed to read temps , EC is not ready for TWR");
			return false;
		}

		for (iTime = 0; iTime < iTimeoutBuf; iTime += iTick) {    // wait for ec ready
			data = (char)ReadPort(0x1604) & 0xff;                // or timeout iTimeoutBuf = 1000
			if (!data)                                            // ec is ready: ctrlprt = 0
				break;
			if (data & 0x50)                                    // some unrequested outputis waiting
				ReadPort(0x161f);                                // clear data output
			::Sleep(iTick);
		}

		WritePort(0x1610, 0x20);                            // tell them we want to read
		data = (char)ReadPort(0x1604) & 0xff;
		if (!(data & 0x20))                                    // ec is not ready
			goto retry;

		for (int i = 1; i < 15; i++) {
			WritePort(0x1610 + i, 0x00);
		}

		WritePort(0x161f, 0x00);

		for (iTime = 0; iTime < iTimeoutBuf; iTime++) {            // wait for full buffers to clear
			data = (char)ReadPort(0x1604) & 0xff;                // or timeout iTimeoutBuf = 1000
			if (data == 0x50)
				break;
		}

		if (data != 0x50)
			goto retry;

		for (int i = 0; i < 16; i++) {
			dataOut[i] = (char)ReadPort(0x1610 + i) & 0xff;
		}

		pfcstate->SensorAddr[0] = 0x78;
		pfcstate->SensorName[0] = this->gSensorNames[0];
		pfcstate->Sensors[0] = dataOut[0];

		pfcstate->SensorAddr[1] = 0x79;
		pfcstate->SensorName[1] = this->gSensorNames[1];
		pfcstate->Sensors[1] = dataOut[1];

		pfcstate->SensorAddr[2] = 0x7a;
		pfcstate->SensorName[2] = this->gSensorNames[2];
		pfcstate->Sensors[2] = dataOut[2];

		pfcstate->SensorAddr[3] = 0x7b;
		pfcstate->SensorName[3] = this->gSensorNames[3];
		pfcstate->Sensors[3] = dataOut[3];

		pfcstate->SensorAddr[4] = 0x7c;
		pfcstate->SensorName[4] = this->gSensorNames[4];
		pfcstate->Sensors[4] = dataOut[4];

		pfcstate->SensorAddr[5] = 0x7d;
		pfcstate->SensorName[5] = this->gSensorNames[5];
		pfcstate->Sensors[5] = dataOut[6];

		pfcstate->SensorAddr[6] = 0x7e;
		pfcstate->SensorName[6] = this->gSensorNames[6];
		pfcstate->Sensors[6] = dataOut[8];

		pfcstate->SensorAddr[7] = 0x7f;
		pfcstate->SensorName[7] = this->gSensorNames[7];
		pfcstate->Sensors[7] = dataOut[9];

		pfcstate->SensorAddr[8] = 0xc0;
		pfcstate->SensorName[8] = this->gSensorNames[8];
		pfcstate->Sensors[8] = dataOut[10];

		pfcstate->SensorAddr[9] = 0xc1;
		pfcstate->SensorName[9] = this->gSensorNames[9];
		pfcstate->Sensors[9] = dataOut[11];

		pfcstate->SensorAddr[10] = 0xc2;
		pfcstate->SensorName[10] = this->gSensorNames[10];
		pfcstate->Sensors[10] = dataOut[12];

		pfcstate->SensorAddr[11] = 0xc3;
		pfcstate->SensorName[11] = this->gSensorNames[11];
		pfcstate->Sensors[11] = dataOut[13];
	}

	return ok;
}
