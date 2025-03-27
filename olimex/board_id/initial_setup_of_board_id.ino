#include <Preferences.h>

Preferences preferences;
const char* nvs_namespace = "board_config";
const char* id_key = "board_id";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Wait for Serial to be ready
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("ESP32 Board ID Burner");
  Serial.println("---------------------");
  Serial.println("Enter a unique ID for this board (10-255):");
  
  // Wait for user input
  while (!Serial.available()) {
    delay(100);
  }
  
  // Read the ID from serial
  int board_id = Serial.parseInt();
  
  // Validate the ID is in range
  if (board_id < 10 || board_id > 255) {
    Serial.println("Error: ID must be between 10 and 255");
    return;
  }
  
  // Open preferences with namespace "board_config"
  preferences.begin(nvs_namespace, false);
  
  // Check if ID already exists
  int existing_id = preferences.getInt(id_key, 0);
  if (existing_id > 0) {
    Serial.print("Warning: Board already has ID: ");
    Serial.println(existing_id);
    Serial.println("Do you want to overwrite? (y/n)");
    
    while (!Serial.available()) {
      delay(100);
    }
    
    char response = Serial.read();
    if (response != 'y' && response != 'Y') {
      Serial.println("Cancelled. Keeping existing ID.");
      preferences.end();
      return;
    }
  }
  
  // Store the new ID
  preferences.putInt(id_key, board_id);
  preferences.end();
  
  Serial.print("Success: Board ID set to ");
  Serial.println(board_id);
}

void loop() {
  // Nothing to do here
}