#include <Keypad.h>
#include <ctype.h>
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>
#define playFileByIndexNumber(arg) playFromMP3Folder(arg)
const int CALLIN_IN_SEC = 10; // time from starting Arduino till telephon ring and play "Anonym call"
const int DIAL_TIMEOUT_MILLSEC = 5000; // if no more digit dialed after 5sec
long phoneBook[] {
  110, 112, 1234,
  2,
  32169,
  4,
  5,
  6666,
  7890,
  8,
  9991, 9992, 9993, 9994, 9995, 9996, 9997, 9998
}; // max phonenumber 999.999.999 since datatype long! max number of entries 32.000 !
/*
 Verzeichnis von F:\mp3

 Verzeichnis von F:\

05.12.2023  20:28    <DIR>          10
08.12.2023  14:38    <DIR>          11
08.12.2023  15:36    <DIR>          30
08.12.2023  14:36    <DIR>          32
08.12.2023  15:36    <DIR>          66
02.12.2023  21:47    <DIR>          mp3
               0 Datei(en),              0 Bytes

 Verzeichnis von F:\mp3

02.12.2023  21:47    <DIR>          .
02.12.2023  21:47    <DIR>          ..
14.08.2017  16:05           111.498 0001_Polizei_Ansage.mp3
14.08.2017  16:28           100.336 0002_Feuerwehr_Ansage.mp3
12.03.2011  14:20           541.420 0003_music_GC1KC9Y.mp3
04.12.2023  12:35            48.066 0004.wav
04.12.2023  12:35            52.476 0005.wav
04.12.2023  12:36           100.104 0006.wav
04.12.2023  12:36            68.352 0007.wav
04.12.2023  12:36            75.848 0008.wav
04.12.2023  12:36            82.464 0009.wav
04.12.2023  12:37           121.712 0010.wav
04.12.2023  12:37            86.874 0011.wav
04.12.2023  12:37           113.774 0012.wav
04.12.2023  12:38            63.060 0013.wav
04.12.2023  12:38            88.196 0014.wav
04.12.2023  12:38           107.160 0015.wav
05.06.2017  11:14           132.370 0016_nummer_1.wav
05.06.2017  11:15           143.396 0017_nummer_2.wav
05.06.2017  11:16           231.604 0018_nummer_3.wav
04.12.2023  12:33           104.072 0019_eins.wav
06.07.2017  15:25           128.522 0020_rosi.mp3
14.08.2017  16:03           125.649 0021_Auskunft.mp3
11.01.2020  17:46           194.938 0022_loser.wav
04.12.2023  12:34            91.724 0023.wav
04.12.2023  12:35           108.924 0024.wav
28.02.2006  13:00            10.026 1001_ringin.wav
06.06.2017  13:28            78.232 1002_kein_anschluss.wav
06.06.2017  13:24           185.520 1003_kein_anschluss_kurz.wa
              27 Datei(en),      3.296.317 Bytes

 Verzeichnis von F:\10

05.12.2023  20:28    <DIR>          .
05.12.2023  20:28    <DIR>          ..
06.06.2017  13:28            78.232 001_kein_anschluss.wav
06.06.2017  13:24           185.520 002_kein_anschluss_kurz.wav
06.06.2017  13:20           247.792 003_bonus.wav
11.01.2020  17:46           194.938 800_loser.wav
14.08.2017  16:03           125.649 999_Auskunft.mp3
               5 Datei(en),        832.131 Bytes

 Verzeichnis von F:\32

08.12.2023  14:36    <DIR>          .
08.12.2023  14:36    <DIR>          ..
06.07.2017  15:25           128.522 168_rosi.mp3
               1 Datei(en),        128.522 Bytes

 Verzeichnis von F:\11

08.12.2023  14:38    <DIR>          .
08.12.2023  14:38    <DIR>          ..
14.08.2017  16:03           125.649 833_Auskunft.mp3
               1 Datei(en),        125.649 Bytes

 Verzeichnis von F:\30

08.12.2023  15:36    <DIR>          .
08.12.2023  15:36    <DIR>          ..
06.06.2017  21:38           118.320 260_geht_evtl_nicht.wav
               1 Datei(en),        118.320 Bytes

 Verzeichnis von F:\66

08.12.2023  15:36    <DIR>          .
08.12.2023  15:36    <DIR>          ..
               0 Datei(en),              0 Bytes

     Anzahl der angezeigten Dateien:
              35 Datei(en),      4.500.939 Bytes
*/


enum foldermp3_sounds {
  FMP3_AUSKUNFT = 21, 
  FMP3_RINGIN = 1001,
  FMP3_KEIN_ANSCHLUSS,
  FMP3_KEIN_ANSCHLUSS_KURZ,
};


SoftwareSerial mySerial(8, 9); // RX, TX
DFPlayerMini_Fast mp3;

#define PIN_HOOK    12


int num_phonenumbers = sizeof(phoneBook) / sizeof(phoneBook[0]);

/* Definition für das Keypad
  3 grau
  4 weiss
  5 orange
  6 blau

  7  lila
  10 gruen
  11 gelb
*/
const byte ROWS = 4; // vier Zeilen
const byte COLS = 3; // 3 Spalten
// Symbole auf den Tasten
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {3, 4, 5, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 10, 11}; //connect to the column pinouts of the keypad


// Instanz von Keypad erzeugen, Keymap und Pins übergeben
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char customKey;

// Our phonebook is not allowed to have more than 32.000 entries
// if entry not found then return -1
int FindIndex( const long a[], size_t size, long value )
{
  size_t index = 0;

  while ( index < size && a[index] != value ) ++index;

  return ( index == size ? -1 : index );
};

// ring phone until receiver is lifted
void ring_phone()
{ while (digitalRead(PIN_HOOK) == HIGH) {
    if (digitalRead(PIN_HOOK) == HIGH) {
      mp3.playFileByIndexNumber(FMP3_RINGIN);
      if (digitalRead(PIN_HOOK) == HIGH) delay (1500); else break;
      mp3.playFileByIndexNumber(FMP3_RINGIN);
    }
    else break ;
    if (digitalRead(PIN_HOOK) == HIGH) delay (4000); else break;
    delay(200);
  };
}; // ring_phome

void disp_mp3_module_info()
{
  Serial.print("currentVolume: "); Serial.println(mp3.currentVolume() );
  Serial.print("currentEQ: "); Serial.println(mp3.currentEQ() );
  Serial.print("currentMode: "); Serial.println(mp3.currentMode() );
  Serial.print("currentVersion: ");Serial.println(mp3.currentVersion() );
  Serial.print("numUsbTracks: "); Serial.println(mp3.numUsbTracks() );
  Serial.print("numSdTracks: "); Serial.println(mp3.numSdTracks() );
  Serial.print("numFlashTracks: "); Serial.println(mp3.numFlashTracks() );
  Serial.print("currentUsbTrack: ");Serial.println(mp3.currentUsbTrack() );
  Serial.print("currentSdTrack: "); Serial.println(mp3.currentSdTrack() );
  Serial.print("currentFlashTrack: "); Serial.println(mp3.currentFlashTrack() );
  Serial.print("numTracksInFolder in folder 01: "); Serial.println(mp3.numTracksInFolder(01) );
  Serial.print("numFolders: "); Serial.println(mp3.numFolders() );
};

void setup() {
  pinMode(PIN_HOOK, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.print(F("\n"
                 "Sketch:   " __FILE__ "\n"
                 "Compiled: " __DATE__ " " __TIME__ "\n"
                 "GCC:      " __VERSION__ "\n"
                 "Projekt: WohnwagenTelefon  \n"));
  Serial.print (F("Anzahl Nummern im Telefonbuch: "));
  Serial.println(num_phonenumbers);

  mySerial.begin(9600);
  mp3.begin(mySerial, false);
  disp_mp3_module_info();
  Serial.println(F("Setting volume to max"));
  mp3.volume(30);

  //mp3.playLargeFolder(3, 9999);
  mp3.playLargeFolder(3, 4095+4096);

  /*
    Serial.println("Playing track 101");
    mp3.playFileByIndexNumber(101);
    delay(2000);
    mp3.playAdvertisement(1);
    delay(1000);
    mp3.stopAdvertisement();
  
  mp3._debug = false;
  mp3.playFolder(11, 33);
  delay(2000);
  mp3.playFolder(10, 1);
  mp3.playFolder(32, 168);
  Serial.println("SEND Stack");
  mp3.printStack(mp3.sendStack);
  Serial.println("RECEIVE Stack");
  mp3.printStack(mp3.recStack);
  mp3.printError();

  */
  
  //Serial.print(F("numTracksInFolder in folder 10: ")); Serial.println(mp3.numTracksInFolder(10) );
   //mp3.printError();
}; // setup

unsigned long int dialed_nr = 0;
unsigned long last_time_dialed = 0;
int index = -1;
long num_waiting4receiver_pickup = 0;
boolean call_still_needed = true; // we gonna call after waiting CALLIN_IN_SEC

void loop() {
  if (digitalRead(PIN_HOOK) == LOW) // receiver in hand
  {
    // check if dialing timed out
    if ( (millis() - last_time_dialed > DIAL_TIMEOUT_MILLSEC) && (dialed_nr != 0) )
    { // at least one digit dialed but not continued to dial for
      // another DIAL_TIMEOUT_MILLSEC
      Serial.print(F("Timeout while dialing ..."));
      mp3.playFileByIndexNumber(FMP3_KEIN_ANSCHLUSS_KURZ);
      dialed_nr = 0;
    };

    // read key if pressed, but do not block !
    customKey = customKeypad.getKey();
    if (customKey) {
      mp3.stop();
      Serial.print("Taste erkannt :");
      Serial.println(customKey);
      if (isdigit(customKey)) {

        dialed_nr = dialed_nr * 10 + (customKey - '0');
        index = FindIndex(phoneBook, num_phonenumbers, dialed_nr);
        if (  index  != -1 )
        {
          Serial.print(F("Nummer im Telefonbuch auf Platz"));
          Serial.println(index + 1);
          mp3.playFileByIndexNumber(index + 1);
          dialed_nr = 0;
        }
        else
        { if (dialed_nr > 10000 && dialed_nr < 100000)
          {
            int folder = dialed_nr / 1000;
            int sound_nr = dialed_nr % 1000;
            Serial.print(F("Spiele Folder: "));
            Serial.print(folder);
            if ( mp3.numTracksInFolder(folder) <1 )
            {
              Serial.print(" ist leer oder nicht vorhanden!");
              mp3.playFileByIndexNumber(FMP3_KEIN_ANSCHLUSS);
            } else
            {
              Serial.print(F(" Sound Nr: "));
              Serial.println(sound_nr);
              if ( folder < 16 )
                 mp3.playLargeFolder(folder, sound_nr);
              else
                 mp3.playFolder(folder, sound_nr);
            };
            
            dialed_nr = 0;
          }
          else
          {
            Serial.println(F("Nummer nicht gefunden"));
            if (dialed_nr >= 1000000)
            { 
              Serial.println(F("Kein Anschluss unter dieser Nummer."));
              mp3.playFileByIndexNumber(FMP3_KEIN_ANSCHLUSS);
              dialed_nr = 0;
            }
          }
          
          
        };
        last_time_dialed = millis();
      };
      Serial.print(F("Rufnummer :"));
      Serial.println (dialed_nr);
    };
  }
  else // hanged up
  {
    Serial.println( F("Handapparat auf Gabel..." ));
    dialed_nr = 0;
    delay(500);
    num_waiting4receiver_pickup ++;
    if (num_waiting4receiver_pickup % 5 == 0)
    {
      Serial.print  ( F("num_waiting4receiver_pickup: ") );
      Serial.println( num_waiting4receiver_pickup );
    };
    if ((num_waiting4receiver_pickup > 2 * CALLIN_IN_SEC) && call_still_needed)
    {
      Serial.println( F("Time to give them a call !") );
      ring_phone();
      mp3.playFileByIndexNumber(FMP3_AUSKUNFT);
      call_still_needed = false;
    }
  };

}
