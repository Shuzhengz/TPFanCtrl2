
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

#include "fancontrol.h"



#define TP_ECOFFSET_FAN		 (char)0x2F	// 1 byte (binary xyzz zzz)
#define TP_ECOFFSET_FANSPEED (char)0x84 // 16 bit word, lo/hi byte
#define TP_ECOFFSET_TEMP0    (char)0x78	// 8 temp sensor bytes from here
#define TP_ECOFFSET_TEMP1    (char)0xC0 // 4 temp sensor bytes from here




// Thanks for the following to "Thinker" on 
// http://www.thinkwiki.org/wiki/Talk:ACPI_fan_control_script
const char *gSensorNames[]= {

	// 78-7F (state index 0-7)
	"CPU", // main processor
	"APS", // harddisk protection gyroscope
	"PCM", // under PCMCIA slot (front left)
	"GPU", // graphical processor
	"BAT", // inside T43 battery
	"X7D", // usually n/a
	"BAT", // inside T43 battery
	"X7F", // usually n/a

	// C0-C4 (state index 8-11)
	"BUS", // unknown
	"PCI", // mini-pci, WLAN, southbridge area
	"PWR", // power supply (get's hot while charging battery)
	"XC3", // usually n/a

	// future
	"",
	"",
	"",
	"",
	""
};

		


//-------------------------------------------------------------------------
//  switch fan according to settings
//-------------------------------------------------------------------------
int 
FANCONTROL::HandleData(void)
{
	char obuf[256]= "", obuf2[128]="", 
		 templist[256]= "", templist2[512], 
		 manlevel[16]= "", title2[128]= "";
	int i, maxtemp, ok= 0;



	//
	// determine highest temp.
	// 

	// build a list of sensors to ignore, separated by "|", e.g. "|XC1|BAT|CPU|"
	char what[16], list[128];
	sprintf(list, "|%s|", this->IgnoreSensors);
	for (i= 0; list[i]!='\0'; i++) {
		if (list[i]==',')	
			list[i]= '|';
	}

	maxtemp= 0;
	
	for (i= 0; i<12; i++) {
		sprintf(what, "|%s|", this->State.SensorName[i]); // name (e.g. "|CPU|") to match against list above

		if (this->State.Sensors[i]!=0x80 && this-State.Sensors[i]!=0x00 && strstr(list, what)==0) {
			maxtemp= __max(this->State.Sensors[i], maxtemp);
		}
	}

	this->MaxTemp= maxtemp;


	//
	// update dialog elements
	//

	// title string (for minimized window)
	sprintf(title2, "%d°C", this->MaxTemp);



	// display fan statte
	int fanctrl= this->State.FanCtrl;
	sprintf(obuf2, "0x%02x (", fanctrl);
	if (fanctrl & 0x80)	{
		strcat(obuf2, "BIOS Controlled)");
		strcat(title2, " Default Fan");
	}
	else {
		sprintf(obuf2+strlen(obuf2), "Fan Level %d, Non Bios)", fanctrl & 0x3F);
		sprintf(title2+strlen(title2), " Fan %d (%s)",
						 fanctrl & 0x3F,
						 this->CurrentModeFromDialog()==2 ? "Smart" : "Fixed");
	}
	::SetDlgItemText(this->hwndDialog, 8100, obuf2);

	strcpy(this->Title2, title2);
	

	// display fan speed (experimental, not visible)
	int fanspeed= (this->State.FanSpeedHi << 8) | this->State.FanSpeedLo;
	if (fanspeed && (short)fanspeed!=(short)0xffff)
		sprintf(obuf2, "%d RPM", fanspeed);
	else
		strcpy(obuf2, "off");
	::SetDlgItemText(this->hwndDialog, 8102, obuf2);



	// display temperature list
	sprintf(obuf2, "%d°C", this->MaxTemp);
	::SetDlgItemText(this->hwndDialog, 8103, obuf2);


	strcpy(templist2, "");
	for (i= 0; i<12; i++) {
		int temp= this->State.Sensors[i];

		if (temp!=128) {
			sprintf(obuf2, "%d°C", temp);
			sprintf(templist2+strlen(templist2), "%s %s (0x%02x)", 
							this->State.SensorName[i],
							obuf2, 
							this->State.SensorAddr[i]);
			strcat(templist2, "\r\n");
		}
		else {
			// strcat(templist2, "n/a\r\n");
		}

	}
	::SetDlgItemText(this->hwndDialog, 8101, templist2);


	// compact single line status (combined)
	strcpy(templist, "");
	for (i= 0; i<12; i++) {
		if (this->State.Sensors[i]!=128) {
			sprintf(templist+strlen(templist), "%d ", this->State.Sensors[i]);
		}
		else {
			strcat(templist, "n/a ");
		}

	}
	templist[strlen(templist)-1]= '\0';
	sprintf(this->CurrentStatus, "Fan: 0x%02x / Highest: %d°C (%s)", this->State.FanCtrl, this->MaxTemp, templist);
	::SetDlgItemText(this->hwndDialog, 8112, this->CurrentStatus);



	//
	// handle fan control according to mode
	//

	this->CurrentModeFromDialog();

	switch (this->CurrentMode) {

		case 1: // Bios Auto
				if (this->State.FanCtrl!=0x080) 
					ok= this->SetFan("Mode switch", 0x80);
				break;


		case 2: // Smart
				this->SmartControl();
				break;


		case 3: // fixed manual
				::GetDlgItemText(this->hwndDialog, 8310, manlevel, sizeof(manlevel));

				if (isdigit(manlevel[0]) && atoi(manlevel)>=0 && atoi(manlevel)<=255) {

					if (this->State.FanCtrl!=atoi(manlevel)) {
						ok= this->SetFan("Manual", atoi(manlevel));
					}
					else 
						ok= true;
				}
				break;
	}

	this->PreviousMode= this->CurrentMode;

	return ok;
}




//-------------------------------------------------------------------------
//  smart fan control depending on temperature
//-------------------------------------------------------------------------
int 
FANCONTROL::SmartControl(void)
{
	int ok= 0, i,
		newfanctrl= -1,	
		fanctrl= this->State.FanCtrl;	


	// if currently auto force initial fan speed according to rules
	if ((fanctrl & 0x80)!=0 && this->PreviousMode==1) {  
		fanctrl= -1;
		newfanctrl= 0;
	}

	// check for fan-up
	for (i= 0; this->SmartLevels[i].temp!=-1; i++) {
		if (this->MaxTemp>=this->SmartLevels[i].temp && this->SmartLevels[i].fan>fanctrl) 
			newfanctrl= this->SmartLevels[i].fan; 
	}

	// not uptriggered check for downtrigger (previous level hit)
	if (newfanctrl==-1) {	
		for (i= 0; this->SmartLevels[i].temp!=-1; i++) {
			if (this->MaxTemp<=this->SmartLevels[i].temp && this->SmartLevels[i].fan<fanctrl) {
				newfanctrl= this->SmartLevels[i].fan; 
				break;
			}
		}
	}

	// fan speed needs change?
	if (newfanctrl!=-1 && newfanctrl!=fanctrl) {

		if (newfanctrl==0x80) { // switch to BIOS-auto mode
		//	this->ModeToDialog(1); // bios
		}
			
		ok= this->SetFan("Smart", newfanctrl);			
	}

	return ok;
}



//-------------------------------------------------------------------------
//  set fan state via EC
//-------------------------------------------------------------------------
int 
FANCONTROL::SetFan(const char *source, int fanctrl, BOOL final)
{
	int ok= 0;
	char obuf[256]= "", obuf2[256], datebuf[128];

	if (this->FanBeepFreq && this->FanBeepDura)
		::Beep(this->FanBeepFreq, this->FanBeepDura);

	this->CurrentDateTimeLocalized(datebuf, sizeof(datebuf));

	
	sprintf(obuf+strlen(obuf), "%s: Set fan control to 0x%02x, ", source, fanctrl);
	sprintf(obuf+strlen(obuf), "Result: ");

	if (this->ActiveMode && !this->FinalSeen) {

		this->EcAccess.Lock();

		// set new fan level
		ok= this->WriteByteToEC(TP_ECOFFSET_FAN, fanctrl);

		// verify completion
		ok= this->ReadByteFromEC(TP_ECOFFSET_FAN, &this->State.FanCtrl);

		if (ok && final) 
			this->FinalSeen= true;	// prevent further changes when setting final mode

		this->EcAccess.Unlock();

		if (this->State.FanCtrl==fanctrl) {
			sprintf(obuf+strlen(obuf), "OK");
			ok= true;
		}
		else {
			sprintf(obuf+strlen(obuf), "COULD NOT SET FAN STATE!!!!");

			::Beep(880, 300);
			::Sleep(200);
			::Beep(880, 300);
			::Sleep(200);
			::Beep(880, 300);
			ok= false;
		}
	}
	else {
		sprintf(obuf+strlen(obuf), "CMD IGNORED (PASSIVE MODE)");
	}

	// display result
	sprintf(obuf2, "%s   (%s)", obuf, datebuf);
	::SetDlgItemText(this->hwndDialog, 8113, obuf2);

	this->Trace(this->CurrentStatus);
	this->Trace(obuf);

	if (!final)
		::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);

	return ok;
}



//-------------------------------------------------------------------------
//  read fan and temperatures from embedded controller
//-------------------------------------------------------------------------
int 
FANCONTROL::ReadEcStatus(FCSTATE *pfcstate)
{
	DWORD i, ok, rc= 0;

	FCSTATE sample= { 0, }, 
			sample2= { 0, };

	setzero(pfcstate, sizeof(*pfcstate));

	
	// reading from the EC seems to yield erratic results at times (probably
	// due to collision with other drivers reading from the port).  So try
	// up to three times to read two samples which look oka and have matching
	// value

	this->EcAccess.Lock();

	for (i= 0; i<3; i++) {
		ok= this->ReadEcRaw(&sample);
		if (ok) {
			ok= this->ReadEcRaw(&sample2);
			if (ok) {

				// match for identical fanctrl settings and temperature differences
				// of no more than 1°C
				BOOL match= sample.FanCtrl==sample2.FanCtrl;
				for (int j= 0; j<12 && match; j++) {
					if (sample.Sensors[j]>128 || sample2.Sensors[j]>128 ||
						abs(sample.Sensors[j]-sample2.Sensors[j])>1) {
						match= false;
					}
				}
				
				if (match) {	// looks good
					ok= true;
					break;
				}
				else {		// try again after short delay
					ok= false;
					::Sleep(200);
				}
			}
		}
	} 

	this->EcAccess.Unlock();

	if (ok) {
		memcpy(pfcstate, &sample, sizeof(*pfcstate));
	}

	return ok;
}



//-------------------------------------------------------------------------
//  read fan and temperatures from embedded controller
//-------------------------------------------------------------------------
int 
FANCONTROL::ReadEcRaw(FCSTATE *pfcstate)
{
	int i, idxtemp, ok;

	pfcstate->FanSpeedLo= 0;
	pfcstate->FanSpeedHi= 0;
	pfcstate->FanCtrl= -1;
	memset(pfcstate->Sensors, 0, sizeof(pfcstate->Sensors));

	
	ok= ReadByteFromEC(TP_ECOFFSET_FAN, &pfcstate->FanCtrl);

	if (ok)
		ok= ReadByteFromEC(TP_ECOFFSET_FANSPEED, &pfcstate->FanSpeedLo);

	if (ok)
		ok= ReadByteFromEC(TP_ECOFFSET_FANSPEED+1, &pfcstate->FanSpeedHi);

	idxtemp= 0;
	for (i= 0; i<8 && ok; i++) {	// temp sensors 0x78 - 0x7f
		ok= ReadByteFromEC(TP_ECOFFSET_TEMP0+i, &pfcstate->Sensors[idxtemp]);
		pfcstate->SensorAddr[idxtemp]= TP_ECOFFSET_TEMP0+i;
		pfcstate->SensorName[idxtemp]= gSensorNames[idxtemp];
		idxtemp++;
	}

	for (i= 0; i<4 && ok; i++) {	// temp sensors 0xC0 - 0xC4
		ok= ReadByteFromEC(TP_ECOFFSET_TEMP1+i, &pfcstate->Sensors[idxtemp]);
		pfcstate->SensorAddr[idxtemp]= TP_ECOFFSET_TEMP1+i;
		pfcstate->SensorName[idxtemp]= gSensorNames[idxtemp];
		idxtemp++;
	}

	return ok;
}


