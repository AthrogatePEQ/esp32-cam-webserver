#include "storage.h"

// These are defined in the main .ino file
extern void flashLED(int flashtime);

extern char *myRotation;           // Rotation
extern int lampVal;                 // The current Lamp value
extern int8_t detection_enabled;    // Face detection enable
extern int8_t recognition_enabled;  // Face recognition enable


/*
 * Useful utility when debugging... 
 */

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing SPIFFS directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("- failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  Serial.println();
}

void dumpPrefs(fs::FS &fs){
  if (fs.exists(PREFERENCES_FILE)) {
    Serial.printf("Showing contents of %s\n", PREFERENCES_FILE);
    // Dump contents for debug
    File file = fs.open(PREFERENCES_FILE, FILE_READ);
    Serial.print("<BEGIN>");
    while (file.available()) {
      byte a = file.read();
      //if (isPrintable(a)) Serial.print(char(a));
      //else Serial.print('.');
      Serial.print(char(a));
    }
    Serial.println("<END>");
    file.close();
  } else {
    Serial.printf("%s not found, nothing to dump.\n", PREFERENCES_FILE);
  }
}

void loadPrefs(fs::FS &fs){
  if (fs.exists(PREFERENCES_FILE)) {
    dumpPrefs(SPIFFS);
    Serial.printf("Reading file %s\n", PREFERENCES_FILE);
    // This is the real operation.
    File file = fs.open(PREFERENCES_FILE, FILE_READ);
    if (!file) {
      Serial.println("Failed to open preferences file");
      return;
    }
    size_t size = file.size();
    if (size > 800) {
      Serial.println("Preferences file size is too large, maybe corrupt");
      return;
    }
    // Allocate the memory for deserialisation
    StaticJsonDocument<1023> doc;
    // Parse the prefs file
    DeserializationError err=deserializeJson(doc, file);
    if(err) {
      Serial.print(F("deserializeJson() failed with code: "));
      Serial.println(err.c_str());
      return;
    }
    // Sensor reference
    sensor_t * s = esp_camera_sensor_get();
    // process all the settings we save
    lampVal = doc["lamp"];
    s->set_framesize(s, (framesize_t)doc["framesize"]);
    s->set_quality(s, doc["quality"]);
    s->set_contrast(s, doc["contrast"]);
    s->set_brightness(s, doc["brightness"]);
    s->set_saturation(s, doc["saturation"]);
    s->set_special_effect(s, doc["special_effect"]);
    s->set_hmirror(s, doc["hmirror"]);
    s->set_vflip(s, doc["vflip"]);
    // close the file
    file.close();
  } else {
    Serial.printf("%s not found, using system defaults.\n", PREFERENCES_FILE);
  }
}

void savePrefs(fs::FS &fs){
  if (fs.exists(PREFERENCES_FILE)) {
    Serial.printf("%s updated\n", PREFERENCES_FILE);
  } else {
    Serial.printf("%s created\n", PREFERENCES_FILE);
  }
  File file = fs.open(PREFERENCES_FILE, FILE_WRITE);
  StaticJsonDocument<1023> doc;
  sensor_t * s = esp_camera_sensor_get();
  doc["lamp"] = lampVal;
  doc["framesize"] = s->status.framesize;
  doc["quality"] = s->status.quality;
  doc["contrast"] = s->status.contrast;
  doc["brightness"] = s->status.brightness;
  doc["saturation"] = s->status.saturation;
  doc["special_effect"] = s->status.special_effect;
  doc["hmirror"] = s->status.hmirror;
  doc["vflip"] = s->status.vflip;
  serializeJson(doc, file);
  file.close();
}

void removePrefs(fs::FS &fs) {
  if (fs.exists(PREFERENCES_FILE)) {
    Serial.printf("Removing %s\r\n", PREFERENCES_FILE);
    if (!fs.remove(PREFERENCES_FILE)) {
      Serial.println("Error removing preferences");
    }
  }
}

void filesystemStart(){
  Serial.println();
    while ( !SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED) ) {
    // if we sit in this loop something is wrong; 
    // if no existing spiffs partition exists one should be automagically created.
    Serial.println("SPIFFS Mount failed, this can happen on first-run initialisation.");
    Serial.println("If it happens repeatedly check if a SPIFFS partition is present for your board?");
    for (int i=0; i<10; i++) {
      flashLED(100); // Show SPIFFS failure
      delay(100);
    }
    delay(1000);
    Serial.println("Retrying..");
  }
  Serial.println("Internal filesystem contents");
  listDir(SPIFFS, "/", 0);
}
