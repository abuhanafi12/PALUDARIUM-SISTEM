#include <esp_now.h>
#include <WiFi.h>

#define X_AXIS_PIN 32
#define Y_AXIS_PIN 33
#define SWITCH_PIN 25



// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t receiverMacAddress[] = {0x24,0xDC,0xC3,0x9C,0x4D,0xF8}; //24:DC:C3:9C:4D:F8

typedef struct PacketData
{
  byte xAxisValue;
  byte yAxisValue;
  byte switchPressed;
} PacketData;

PacketData data;

esp_now_peer_info_t peerInfo;

// Fungsi utilitas untuk memetakan dan menyesuaikan nilai Joystick
int mapAndAdjustJoystickDeadBandValues(int value, bool reverse)
{
  if (value >= 2200)
  {
    value = map(value, 2200, 4095, 127, 254);
  }
  else if (value <= 1800)
  {
    value = map(value, 1800, 0, 127, 0);  
  }
  else
  {
    value = 127;
  }

  if (reverse)
  {
    value = 254 - value;
  }
  return value;
}

// Global Scope: DEFINISI FUNGSI CALLBACK YANG DIPERBAIKI (TANDA TANGAN BARU)
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t ");
  Serial.println(status);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Message sent" : "Message failed");
}

void setup() 
{
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
  {
    Serial.println("Succes: Initialized ESP-NOW");
  }

  // Register the send callback function
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  
  //peerInfo.ifidx=WIFI_IF_AP;
  // FIX: Melengkapi panggilan fungsi memcpy dengan angka 6, tanda kurung tutup, dan semicolon
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6); 
  peerInfo.channel = 0;   
  peerInfo.encrypt = false;
  
  // Add peer          
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  else
  {
    Serial.println("Succes: Added peer");
  } 

  pinMode(SWITCH_PIN, INPUT_PULLUP);   
}
 
void loop() 
{
  data.xAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(X_AXIS_PIN), false);
  data.yAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(Y_AXIS_PIN), true);  
  data.switchPressed = false; 

  if (digitalRead(SWITCH_PIN) == LOW)
  {
    data.switchPressed = true;
  }
  
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
  if (result == ESP_OK) 
  {
    Serial.println("Sent with success");
  }
  else 
  {
    Serial.println("Error sending the data");
  }     
  
  if (data.switchPressed == true)
  {
    delay(500);
  }
  else
  {
    delay(50);
  }
}