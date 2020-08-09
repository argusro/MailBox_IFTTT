/*=============================================================================================================

   MailBox notification with ESP-01 and IFTTT

   This code will make a single webhook call, and then enters in deep sleep.
   Every time that the ESP-01 is reset will repeat this process.
   Electrical connections are simple, supply the ESP with 3.3V and a switch between
   reset pin and ground.
   OTA (Over The Air) firmware upgrade process was also implemented, just place
   the new bin file in a web server and a file including the firmware version.
   If differ from the one stored on the local virtual EEPROM the new firmware
   will be pulled and installed.
   Use ESP-01 with memory > 1MB

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   Firmware: 1.0 - 08/2020
   Autor: ArgusR <@argusro>

  ===========================================================================================================*/
// lIBRARIES FOR:
#include <ESP8266WiFi.h>            // wifi with ESP8266
#include <ArduinoOTA.h>             // OTA firmware update
#include <EEPROM.h>                 // non-volatile  memory storage
#include <ESP8266httpUpdate.h>      // firmware update
#include <ESP8266HTTPClient.h>      // http call

//#define DEBUG                       // uncomment this line to debug
#define USE_OTA                     // comment this line to not use OTA, but it's useful to have this after you deploy the system on your mailbox   

const char* fwImageURL = "http://your_webserver.com/path_to/firmware.bin";                    // update with your link to the new firmware bin file.
const char* fwVersionURL = "http://your_webserver.com/path_to/firmware.version";              // update with your link to a text file with new version (just a single line with a number).
																							  // if you change the number stored on this file it will trigger the firmware download.

const char* webhootURL = "http://maker.ifttt.com/trigger/IFTTT_EVENT/with/key/IFTTT_KEY"; // change IFTTT_EVENT and IFTTT_KEY to your values

uint addr = 0;                      // data structure to be read from EEPROM
struct {
  String fw_version = "";
} data;

String first_fw = "0.1";            // firmware version to be stored first time this code is deployed

char ssid[] = "SSID";               // change to your SSID, keep the quotes
char pass[] = "password";           // change to your wifi password, keep the quotes

WiFiClient  client;

void setup() {
#ifdef USE_OTA
  EEPROM.begin(16);
  EEPROM.get(addr, data);                           // read version number stored on EEPROM

  if (!isDigit(data.fw_version.charAt(0))) {        // check if memory is clean
#ifdef DEBUG
    Serial.println("No firmware information found, saving first firmware value");
#endif
    EEPROM.put(addr, first_fw);                     // write first firmware version if that is the case
    EEPROM.commit();
  }
#endif

#ifdef DEBUG
  Serial.begin(115200); //Initialize serial         // start serial if DEBUG is uncomment on the top
  Serial.println("\n\n\n");
#endif

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {              // start wifi connection procedure
#ifdef DEBUG
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(String(ssid));
#endif
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
#ifdef DEBUG
      Serial.print(".");
#endif
      delay(5000);
      if (millis() > 60000) ESP.restart();        // if doesn't connect in 60 seconds, restart ESP
    }
#ifdef DEBUG
    Serial.println("\nConnected.");
#endif
  }

  maker_event();                                  // call IFTTT event

#ifdef USE_OTA
  check_OTA();                                    // call firmware version check
#endif

  ESP.deepSleep(0);                               // go to sleep indefinitely
}

void loop() {                                     // since this is a deepsleep code, no need to use loop function
}

void maker_event() {                              // webhook post event
  HTTPClient http;
  if (http.begin(client, webhootURL)) {			  // not passing any value back, but you could return firmware version if you like
    int httpCode = http.GET();
#ifdef DEBUG
    Serial.println(httpCode);
#endif
  }
  http.end();
}

void check_OTA() {                                                              // check firmware version
  float current_version;
  current_version = data.fw_version.toFloat();                                  // convert fw_version string to a float
#ifdef DEBUG
  Serial.println("Current firmware:" + String(current_version));
#endif
  HTTPClient http;                                                              // define http client
  if (http.begin(client, fwVersionURL)) {                                       // start http client
    int httpCode = http.GET();                                                  
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {  // check http status
      String newFWVersion = http.getString();                                   // read value stored on the file
      float newVersion = newFWVersion.toFloat();                                // convert string to float
      if ( newVersion != current_version ) {                                    // compare remote file version with local version, if different:
#ifdef DEBUG
        Serial.println("New firmware:" + String(newVersion));
#endif
        data.fw_version = newFWVersion;                                         // store the new value on EEPROM
        EEPROM.put(addr, data);
        EEPROM.commit();
        OTA_start();                                                            // start OTA process
        delay(100);
        ESPhttpUpdate.update( fwImageURL );                                     // update firmware with remote bin file
      }
    }
  } // end if http.begin
} // end check_OTA()

void OTA_start() {                                                              // OTA code extracted from basic example. Uncomment prints if you wish
  ArduinoOTA.onStart([]() {                                                     // more details about the process.
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    //Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
#ifdef DEBUG
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}
