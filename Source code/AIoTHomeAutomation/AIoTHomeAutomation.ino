#define BLYNK_TEMPLATE_ID "TMPL3tutmzrCe"
#define BLYNK_TEMPLATE_NAME "AIoT Home automation"
#define BLYNK_AUTH_TOKEN "EP_dkCgIKJ3IdFYr7fFx-o2zOoYq5KcU"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiManager.h>

// Your WiFi credentials
char ssid[] = "";
char pass[] = "";

// Define the relay pins
const int relay1 = 27;
const int relay2 = 4;
const int relay3 = 25;
const int relay4 = 5;
const int relay5 = 19;

#define TRIGGER_PIN 15

// Define the push switch and buzzer pins
const int pushSwitch = 13;
const int buzzer = 14;

// Blynk virtual pins for the relays
#define VPIN_RELAY_1 V1
#define VPIN_RELAY_2 V2
#define VPIN_RELAY_3 V3
#define VPIN_RELAY_4 V4
#define VPIN_RELAY_5 V5

unsigned long buttonPressTime = 0;
unsigned long lastCheckTime = 0;
bool buttonPressed = false;
bool wifiResetting = false;
bool wasOffline = false;

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

  WiFiManager wm;

  bool res;
  res = wm.autoConnect("AIoT Home AP", "password");
  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    Serial.println("connected...yeey :)");
  }
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());

  // Set relay pins as output
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);

  // Set push switch and buzzer pins
  pinMode(pushSwitch, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  // Set all relays to off initially
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(relay5, LOW);

  // Turn off the buzzer
  digitalWrite(buzzer, LOW);

  // Trigger the buzzer for 5 seconds at startup
  triggerBuzzer(5000);
}

void loop() {
  Blynk.run();
  checkPushButton();
  checkTriggerPin();
  checkWiFiStatusPeriodically();

  if (digitalRead(TRIGGER_PIN) == LOW && !wifiResetting) {
    wifiResetting = true;
    Serial.println("WiFi Resetting");
    // Reset WiFi
    WiFiManager().resetSettings();
    WiFiManager wm;
    wm.setConfigPortalTimeout(120);

    if (!wm.startConfigPortal("AIoT Home On Demand")) {
      Serial.println("Failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
      Serial.println("System Offline");
    }
    Serial.println("Connected... :)");
    Serial.println("System Online");
    wifiResetting = false;
  }
}

// Blynk write functions for controlling the relays
BLYNK_WRITE(VPIN_RELAY_1) {
  int value = param.asInt();
  digitalWrite(relay1, value);
}

BLYNK_WRITE(VPIN_RELAY_2) {
  int value = param.asInt();
  digitalWrite(relay2, value);
}

BLYNK_WRITE(VPIN_RELAY_3) {
  int value = param.asInt();
  digitalWrite(relay3, value);
}

BLYNK_WRITE(VPIN_RELAY_4) {
  int value = param.asInt();
  digitalWrite(relay4, value);
}

BLYNK_WRITE(VPIN_RELAY_5) {
  int value = param.asInt();
  digitalWrite(relay5, value);
}

// Function to sync relay states from Blynk server when connected
BLYNK_CONNECTED() {
  delay(1000); // Wait for 1 second to ensure connection is stable
  Blynk.syncVirtual(VPIN_RELAY_1);
  Blynk.syncVirtual(VPIN_RELAY_2);
  Blynk.syncVirtual(VPIN_RELAY_3);
  Blynk.syncVirtual(VPIN_RELAY_4);
  Blynk.syncVirtual(VPIN_RELAY_5);
}

// Function to check the state of the push button
void checkPushButton() {
  if (digitalRead(pushSwitch) == LOW) {
    if (!buttonPressed) {
      buttonPressTime = millis();
      buttonPressed = true;
      digitalWrite(buzzer, HIGH); // Turn on the buzzer
    } else if (millis() - buttonPressTime >= 3000) {
      // Reset the system after button is pressed for 3 seconds
      resetSystem();
    }
  } else {
    buttonPressed = false;
    digitalWrite(buzzer, LOW); // Turn off the buzzer
  }
}

// Function to check the state of the TRIGGER_PIN
void checkTriggerPin() {
  if (digitalRead(TRIGGER_PIN) == LOW) {
    digitalWrite(buzzer, HIGH); // Turn on the buzzer
    delay(500); // Keep the buzzer on for 500 milliseconds
    digitalWrite(buzzer, LOW); // Turn off the buzzer
  }
}

// Function to check the WiFi status periodically
void checkWiFiStatusPeriodically() {
  unsigned long currentTime = millis();
  if (currentTime - lastCheckTime >= 3000) {
    lastCheckTime = currentTime;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("AIoT Home Offline");
      if (!wasOffline) {
        wasOffline = true;
        triggerBuzzer(3000); // Trigger buzzer for 3 seconds if system goes offline
      }
    } else {
      Serial.println("AIoT Home Online");
      if (wasOffline) {
        wasOffline = false;
        triggerBuzzer(4000); // Trigger buzzer for 4 seconds if system comes online
      }
    }
  }
}

// Function to trigger the buzzer for a specified duration
void triggerBuzzer(unsigned long duration) {
  digitalWrite(buzzer, HIGH);
  delay(duration);
  digitalWrite(buzzer, LOW);
}

// Function to reset the system
void resetSystem() {
  Serial.println("System reset");
  triggerBuzzer(5000); // Trigger buzzer for 5 seconds at system reset
  ESP.restart();
}
