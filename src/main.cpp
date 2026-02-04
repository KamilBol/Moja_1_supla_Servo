#include <Arduino.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/network/esp_web_server.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/custom_parameter.h>
#include <Servo.h>

// ================= KONFIGURACJA =================
const int SERVO_PIN = D1;
const int STATUS_LED_PIN = D4;
const int CONFIG_BUTTON_PIN = D3;

// NAPIS "Bólu" - Prosty HTML
const char* HTML_BOLU = "<h1 style='color:green; text-align:center; font-size:40px; margin: 10px;'>Bólu</h1>";

// ================= KLASA SERWA =================
class ServoRelay : public Supla::Control::Relay {
  private:
    Servo myServo;
    int pin;
  public:
    ServoRelay(int pinGpio) : Relay(pinGpio, false), pin(pinGpio) {
      myServo.attach(pin);
      myServo.write(0);
    }
    void turnOn(_supla_int_t duration) override {
      Serial.println("--- Otwieram ---");
      myServo.attach(pin);
      myServo.write(90);
      channel.setNewValue(true);
    }
    void turnOff(_supla_int_t duration) override {
      Serial.println("--- Zamykam ---");
      myServo.write(0);
      channel.setNewValue(false);
    }
};

Supla::ESPWifi wifi;
Supla::EspWebServer suplaServer;

void setup() {
  Serial.begin(115200);
  
  // 1. Pamięć
  new Supla::LittleFsConfig();
  new Supla::Device::StatusLed(STATUS_LED_PIN, true);

  // 2. GUI (Strona WWW)
  new Supla::Html::CustomParameter("tekst_bolu", HTML_BOLU);
  
  new Supla::Html::WifiParameters();
  new Supla::Html::ProtocolParameters();
  new Supla::Html::StatusLedParameters();
  new Supla::Html::DeviceInfo(&SuplaDevice);

  // 3. Przycisk Config (D3)
  auto cfgButton = new Supla::Control::Button(CONFIG_BUTTON_PIN, true, true);
  cfgButton->addAction(Supla::ENTER_CONFIG_MODE, &SuplaDevice, Supla::ON_HOLD);

  // 4. Serwo
  new ServoRelay(SERVO_PIN);

  // 5. NAZWA STARTOWA
  SuplaDevice.setName("Supla Bolu");

  // 6. Start
  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}