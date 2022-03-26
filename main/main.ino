#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

#define VOLUME_PIN A0

class Mp3Notify;

SoftwareSerial secondarySerial(10, 11); // RX, TX
typedef DFMiniMp3<SoftwareSerial, Mp3Notify, Mp3ChipMH2024K16SS> DfMp3;
DfMp3 dfmp3(secondarySerial);

uint16_t volume;
uint16_t totalFilesCount;

byte buttonColPins[] = {4, 3, 2};
int numberOfCols = sizeof(buttonColPins) / sizeof(buttonColPins[0]);

byte buttonRowPins[] = {5, 6, 7};
int numberOfRows = sizeof(buttonRowPins) / sizeof(buttonRowPins[0]);

int currentPlayedTrackIndex = -1;

class Mp3Notify
{
public:
	static void PrintlnSourceAction(DfMp3_PlaySources source, const char *action)
	{
		if (source & DfMp3_PlaySources_Sd)
		{
			Serial.print("[DFPlayer] SD Card, ");
		}
		if (source & DfMp3_PlaySources_Usb)
		{
			Serial.print("[DFPlayer] USB Disk, ");
		}
		if (source & DfMp3_PlaySources_Flash)
		{
			Serial.print("[DFPlayer] Flash, ");
		}
		Serial.println(action);
	}
	static void OnError(DfMp3 &mp3, uint16_t errorCode)
	{
		Serial.print("[DFPlayer] Com Error ");
		Serial.println(errorCode);
	}
	static void OnPlayFinished(DfMp3 &mp3, DfMp3_PlaySources source, uint16_t track)
	{
		Serial.print("[DFPlayer] Play finished for #");
		Serial.println(track);
	}
	static void OnPlaySourceOnline(DfMp3 &mp3, DfMp3_PlaySources source)
	{
		PrintlnSourceAction(source, "online");
	}
	static void OnPlaySourceInserted(DfMp3 &mp3, DfMp3_PlaySources source)
	{
		PrintlnSourceAction(source, "inserted");
	}
	static void OnPlaySourceRemoved(DfMp3 &mp3, DfMp3_PlaySources source)
	{
		PrintlnSourceAction(source, "removed");
	}
};

void setup()
{
	Serial.begin(115200);

	Serial.println("initializing...");

	dfmp3.begin();

	volume = dfmp3.getVolume();
	Serial.print("Current volume ");
	Serial.println(volume);

	totalFilesCount = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
	Serial.print("Total files count: ");
	Serial.println(totalFilesCount);

	pinMode(VOLUME_PIN, INPUT);
	setupButtonMatrix();
}

void setupButtonMatrix()
{
	for (int i = 0; i < numberOfCols; i++)
	{
		pinMode(buttonColPins[i], OUTPUT);
		digitalWrite(buttonColPins[i], HIGH);
	}
	for (int i = 0; i < numberOfRows; i++)
	{
		pinMode(buttonRowPins[i], INPUT_PULLUP);
	}
}

int getPressedButton()
{
	int pressedBtnIndex = -1;
	for (int i = 0; i < numberOfCols; i++)
	{
		digitalWrite(buttonColPins[i], LOW);
		for (int j = 0; j < numberOfRows; j++)
		{
			if (LOW == digitalRead(buttonRowPins[j]))
			{
				pressedBtnIndex = i + (j * numberOfCols);
				break;
			}
		}
		digitalWrite(buttonColPins[i], HIGH);
		if (-1 != pressedBtnIndex)
		{
			break;
		}
	}
	return pressedBtnIndex;
}

int getVolume()
{
	int vol = analogRead(VOLUME_PIN);
	return map(vol, 0, 1022, 0, 30); // 1022and not 1023 to avoid the fluctuations when reaching 1023
}

void loop()
{

	int pressedButton = getPressedButton();

	if (-1 != pressedButton && pressedButton != currentPlayedTrackIndex)
	{
		Serial.print("Button pressed: ");
		Serial.println(pressedButton);
		currentPlayedTrackIndex = pressedButton;
		dfmp3.playMp3FolderTrack(currentPlayedTrackIndex + 1);
	}

	int vol = getVolume();
	if(abs(volume - vol) > 2 || (0 == vol && 0 != volume)) { // only if significant change, or 0 - change the volume
		volume = vol;
		Serial.print("Volume changed to: ");
		Serial.println(volume);
		dfmp3.setVolume(volume);
	}
	
}
