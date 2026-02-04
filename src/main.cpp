#include <Arduino.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <Servo.h>

// --- KONFIGURACJA PINÓW (Wemos D1 Mini) ---
const int SERVO_PIN = D1;        // Serwo
const int STATUS_LED_PIN = D4;   // Dioda LED (Wemos ma na D4)
const int CONFIG_BUTTON_PIN = D3; // Przycisk FLASH (D3) - do wchodzenia w tryb konfiguracyjny

// --- USTAWIENIA SERWA ---
const int KAT_OTWARTY = 90;
const int KAT_ZAMKNIETY = 0;

// --- TWOJA KLASA SERWA (Działa jak przekaźnik) ---
class ServoRelay : public Supla::Control::Relay {
  private:
    Servo myServo;
    int pin;

  public:
    ServoRelay(int pinGpio) : Relay(pinGpio, false), pin(pinGpio) {
      myServo.attach(pin);
      myServo.write(KAT_ZAMKNIETY);
    }

    void turnOn(_supla_int_t duration) override {
      Serial.println("SUPLA: Otwieram zawór");
      myServo.attach(pin);
      myServo.write(KAT_OTWARTY);
      channel.setNewStatus(true);
    }

    void turnOff(_supla_int_t duration) override {
      Serial.println("SUPLA: Zamykam zawór");
      myServo.write(KAT_ZAMKNIETY);
      channel.setNewStatus(false);
    }
};

// --- KOMPONENTY SYSTEMU ---
Supla::Storage::LittleFsConfig config;
Supla::ESPWifi wifi;
Supla::Html::DeviceInfo deviceInfo(&SuplaDevice);
Supla::Html::WifiParameters wifiParams;

// TO JEST KLUCZOWE - To dodaje pola "Serwer" i "Email" na stronie konfiguracyjnej!
Supla::Html::ProtocolParameters protocolParams; 

Supla::Html::StatusLedParameters statusLedParams;

void setup() {
  Serial.begin(115200);

  // 1. Ustawienie niestandardowego IP strony konfiguracyjnej (Twoje 192.168.5.1)
  wifi.setAPAddress(IPAddress(192.168, 5, 1));

  // 2. Dioda statusu
  new Supla::Device::StatusLed(STATUS_LED_PIN, true);

  // 3. Przycisk Konfiguracji (D3/Flash)
  // Jak przytrzymasz go 5-10s, urządzenie wejdzie w tryb konfiguracji i wystawi sieć WiFi.
  // To jest Twoje "zabezpieczenie" - nikt nie wejdzie w ustawienia bez fizycznego dostępu.
  auto cfgButton = new Supla::Control::Button(CONFIG_BUTTON_PIN, true, true);
  cfgButton->configureAsConfigButton(&SuplaDevice);

  // 4. Twoje Serwo
  new ServoRelay(SERVO_PIN);

  // 5. Start biblioteki
  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}