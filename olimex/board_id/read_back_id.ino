#include <Preferences.h>

Preferences preferences;
const char* nvs_namespace = "board_config";
const char* id_key = "board_id";
int board_id;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Open preferences
  preferences.begin(nvs_namespace, true); // true = read-only mode
  
  // Get the board ID
  board_id = preferences.getInt(id_key, 0);
  preferences.end();
  
  // Display the ID
  Serial.print("Board ID: ");
  if (board_id >= 10 && board_id <= 255) {
    Serial.println(board_id);
  } else {
    Serial.println("Not set or invalid (must be 10-255)");
  }
  
  // Your main initialization code goes here
  // ...
}

void loop() {
  // Your main program logic goes here
  // You can access the board_id variable anywhere in your code
  
  // Example: Print ID every 5 seconds
  Serial.print("This is board #");
  Serial.println(board_id);
  delay(5000);
}