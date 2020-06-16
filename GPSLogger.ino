#include "LoRaWAN.h"
#include "CayenneLPP.h"
CayenneLPP ThisLPP(64);
const char *devEui = "";
const char *appEui = "";
const char *appKey = "";
const int Green = 4;
const int Red = 5;
const int Blue = 10;
long TimeBetweenTransmissions = 10 * 1000;
long TimeStamp = -1 * TimeBetweenTransmissions;

void setup( void )
{
  // US Region
  LoRaWAN.begin(US915);
  // Helium SubBand
  LoRaWAN.setSubBand(2);
  // Disable Adaptive Data Rate
  LoRaWAN.setADR(false);
  // Set Data Rate 1 - Max Payload 53 Bytes
  LoRaWAN.setDataRate(1);
  // Device IDs and Key
  LoRaWAN.joinOTAA(appEui, appKey, devEui);
  // Setup LEDs
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);
  // GPS reset pin
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);
  // UART to communicate with GPS
  Serial.begin(19200);
}

void loop( void )
{
  String UARTText;
  float Lat, Lon, Alt;
  int CommaArray[16];
  do {
    UARTText = "";
    // Wait for any UART text
    while (Serial.available() > 0) {
      digitalWrite(Green, 1);
      // Serial is too slow
      delay(6);
      // Build the string
      UARTText += char(Serial.read());
      digitalWrite(Green, 0);
    }
    // Reset GPS if missed 3 transmissions unless just started then wait 5 minutes
    if (millis() > TimeStamp +  TimeBetweenTransmissions * ((TimeStamp > 0) ? 3 : 31)) {
      // New timestamp
      TimeStamp = millis();
      digitalWrite(15, 0);
      digitalWrite(15, 1);
    }
    // Global Positioning System Fix Data explanation can be found at http://aprs.gids.nl/nmea/#gga
  } while (!UARTText.startsWith("$GPGGA"));
  // Find commas in string
  for (int i = 1; i < 16; i++) CommaArray[i] = UARTText.indexOf(',', CommaArray[i - 1] + 1);
  // Board is not busy, already joined, been more than 10 seconds since transmission, and we have GPS fix
  if (!LoRaWAN.busy() && LoRaWAN.joined() && (millis() > TimeStamp + TimeBetweenTransmissions) && UARTText.substring(CommaArray[7] + 1, CommaArray[8]).toFloat() > 0.0) {
    // New timestamp
    TimeStamp = millis();
    // Start send
    digitalWrite(Blue, 1);
    // Degrees Minutes Seconds to Decimal Degrees conversion found at https://www.latlong.net/degrees-minutes-seconds-to-decimal-degrees
    // First 2 digits are degrees
    Lat = (UARTText.substring(CommaArray[3] + 1, CommaArray[3] + 1 + 2).toFloat()
           // Divide minutes by 60 to add to degrees
           + UARTText.substring(CommaArray[3] + 1 + 2, CommaArray[4]).toFloat() / 60.0)
          // Multiply by -1 if South
          * ((UARTText.substring(CommaArray[4], CommaArray[5]) == ",N" ? 1.0 : -1.0));
    // First 3 digits are degrees
    Lon = (UARTText.substring(CommaArray[5] + 1, CommaArray[5] + 1 + 3).toFloat()
           // Divide minutes by 60 to add to degrees
           + UARTText.substring(CommaArray[5] + 1 + 3, CommaArray[6]).toFloat() / 60.0)
          // Multiply by -1 if West
          * ((UARTText.substring(CommaArray[6], CommaArray[7]) == ",E" ? 1.0 : -1.0));
    // Covert meters to feet
    Alt = UARTText.substring(CommaArray[10] + 1, CommaArray[11]).toFloat() * 3.28084;
    ThisLPP.reset();
    //ThisLPP.addGPS(0, Lat, Long, Alt)
    ThisLPP.addGPS(0, Lat , Lon , Alt );
    LoRaWAN.sendPacket(ThisLPP.getBuffer(), ThisLPP.getSize());
    // End send
    digitalWrite(Blue, 0);
  } //else digitalWrite(Blue, 1 - digitalRead(Blue));
  // Blink Red LED
  //digitalWrite(Red, 1 - digitalRead(Red));
  //delay(500);
}