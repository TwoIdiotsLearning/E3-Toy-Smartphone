#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

class Mp3Notify;

SoftwareSerial secondarySerial(10, 11); // RX, TX
typedef DFMiniMp3<SoftwareSerial, Mp3Notify, Mp3ChipMH2024K16SS> DfMp3;
DfMp3 dfmp3(secondarySerial);

uint16_t volume;
uint16_t totalFilesCount;

byte buttonColPins[] = {2, 3, 4};
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
			Serial.print("SD Card, ");
		}
		if (source & DfMp3_PlaySources_Usb)
		{
			Serial.print("USB Disk, ");
		}
		if (source & DfMp3_PlaySources_Flash)
		{
			Serial.print("Flash, ");
		}
		Serial.println(action);
	}
	static void OnError(DfMp3 &mp3, uint16_t errorCode)
	{
		Serial.print("Com Error ");
		Serial.println(errorCode);
	}
	static void OnPlayFinished(DfMp3 &mp3, DfMp3_PlaySources source, uint16_t track)
	{
		Serial.print("Play finished for #");
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
			if (LOW == digitalRead(buttonRowPins[i]))
			{
				pressedBtnIndex = i + j;
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

void loop()
{
	int pressedButton = getPressedButton();
	if (-1 != pressedButton && pressedButton != currentPlayedTrackIndex)
	{
		currentPlayedTrackIndex = pressedButton;
		dfmp3.playMp3FolderTrack(currentPlayedTrackIndex);
	}
}
