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

// --- KONFIGURACJA PINÓW ---
const int SERVO_PIN = D1;
const int STATUS_LED_PIN = D4;
const int CONFIG_BUTTON_PIN = D3;

// --- USTAWIENIA SERWA ---
const int KAT_OTWARTY = 90;
const int KAT_ZAMKNIETY = 0;

// --- TWOJA KLASA SERWA ---
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
      channel.setNewValue(true);
    }

    void turnOff(_supla_int_t duration) override {
      Serial.println("SUPLA: Zamykam zawór");
      myServo.write(KAT_ZAMKNIETY);
      channel.setNewValue(false);
    }
};

// Komponenty globalne
Supla::ESPWifi wifi;
Supla::Html::DeviceInfo deviceInfo(&SuplaDevice);
Supla::Html::WifiParameters wifiParams;
Supla::Html::ProtocolParameters protocolParams;
Supla::Html::StatusLedParameters statusLedParams;

void setup() {
  Serial.begin(115200);

  // 1. Konfiguracja pamięci (LittleFS)
  // POPRAWKA: Używamy Supla::LittleFsConfig bez "Storage::"
  new Supla::LittleFsConfig();

  // 2. Dioda statusu
  new Supla::Device::StatusLed(STATUS_LED_PIN, true);

  // 3. Przycisk Konfiguracji
  auto cfgButton = new Supla::Control::Button(CONFIG_BUTTON_PIN, true, true);
  
  // POPRAWKA: Dodaliśmy znak "&" przed SuplaDevice (przekazujemy wskaźnik)
  cfgButton->addAction(Supla::ENTER_CONFIG_MODE, &SuplaDevice, Supla::ON_HOLD);

  // 4. Twoje Serwo
  new ServoRelay(SERVO_PIN);

  // 5. Start biblioteki
  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}