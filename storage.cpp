#include "storage.h"

// This is defined in the main .ino file
extern void flashLED(int flashtime);

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

void loadPrefs(fs::FS &fs){
  if (fs.exists(PREFERENCES_FILE)) {
    Serial.printf("Reading file: %s\n", PREFERENCES_FILE);
    File filedump = fs.open(PREFERENCES_FILE, FILE_READ);
    Serial.println("<BEGIN>");
    while (filedump.available()) {
      byte a = filedump.read();
      if (isPrintable(a)) Serial.printf("%02X - %c\n",a,a);
      else Serial.printf("%02X -\n",a);
    }
    Serial.println("<END>");
    filedump.close();
    
    File file = fs.open(PREFERENCES_FILE, FILE_READ);
    sensor_t * s = esp_camera_sensor_get();
    int bytes = file.read((byte *)s, sizeof(*s));
    Serial.print("Read: ");
    Serial.print(bytes);
    Serial.println(" bytes");
    file.close();
  } else {
    Serial.printf("Preferences file: %s not found, using system defaults.\n", PREFERENCES_FILE);
  }
}

void savePrefs(fs::FS &fs){
  if (fs.exists(PREFERENCES_FILE)) {
    Serial.printf("Preferences file: %s already exists. Overwriting.\n", PREFERENCES_FILE);
  } else {
    Serial.printf("Creating: %s\n", PREFERENCES_FILE);
  }
  File file = fs.open(PREFERENCES_FILE, FILE_WRITE);
  sensor_t * s = esp_camera_sensor_get();
  /* Only save the first part of the structure; we dont need all the pointers to methods/functions
     See: https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h#L142 
     the +2 is bacause tha resulting sensor_t structure is padded by the compiler. */
  int h = sizeof(sensor_id_t) + sizeof(uint8_t) + sizeof(pixformat_t) + sizeof(camera_status_t) + 2;
  int bytes = file.write((byte *)s, h);
  Serial.print("Wrote: ");
  Serial.print(bytes);
  Serial.println(" bytes");
  if (bytes != h){
    Serial.println("- file write failed");
  }
  file.close();
}

void removePrefs(fs::FS &fs) {
  if (fs.exists(PREFERENCES_FILE)) {
    Serial.printf("Removing: %s\r\n", PREFERENCES_FILE);
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
