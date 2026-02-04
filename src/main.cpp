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
#include <supla/network/html/custom_text_parameter.h>
#include <supla/network/html/custom_checkbox_parameter.h>
#include <supla/network/html/custom_parameter.h>
#include <Servo.h>

// ================= KONFIGURACJA PINÓW =================
const int SERVO_PIN = D1;        // Serwo
const int BUTTON_PIN = D2;       // Przycisk
const int STATUS_LED_PIN = D4;   // Dioda
const int CONFIG_BUTTON_PIN = D3; // Config

// ================= GLOBALNY WSKAŹNIK KONFIGURACJI =================
// To jest nasz klucz do sukcesu - zapiszemy tu dostęp do pamięci
Supla::LittleFsConfig *suplaConfig = nullptr;

// ================= NAPIS BÓLU (CSS) =================
const char* HTML_BOLU = "<h1 style='color:green; text-align:center; font-size:40px; margin: 10px;'>Bólu</h1>";

// ================= KLASA SERWA =================
class ServoRelay : public Supla::Control::Relay {
  private:
    Servo myServo;
    int pin;

    // Pobieramy kąt z pamięci (bezpiecznie)
    int getAngle(bool open) {
      int valOpen = 90;  // Domyślne wartości
      int valClosed = 0;
      bool inverted = false;

      // Czytamy z pamięci TYLKO jeśli wskaźnik istnieje
      if (suplaConfig) {
        char buf[16];
        // Pobierz kąt OTWARTY
        if (suplaConfig->getString("angle_open", buf, 16)) {
           if (strlen(buf) > 0) valOpen = atoi(buf);
        }
        // Pobierz kąt ZAMKNIĘTY
        if (suplaConfig->getString("angle_closed", buf, 16)) {
           if (strlen(buf) > 0) valClosed = atoi(buf);
        }
        // Pobierz odwrócenie logiki
        int32_t logicVal = 0;
        if (suplaConfig->getInt32("logic_invert", &logicVal)) {
           inverted = (logicVal == 1);
        }
      }

      if (open) {
        return inverted ? valClosed : valOpen;
      } else {
        return inverted ? valOpen : valClosed;
      }
    }

  public:
    ServoRelay(int pinGpio) : Relay(pinGpio, false), pin(pinGpio) {
      // Konstruktor - nic nie robimy z serwem, żeby nie szarpało przy starcie
    }

    void turnOn(_supla_int_t duration) override {
      int angle = getAngle(true);
      Serial.print("--- OTWIERAM (Kat: "); Serial.print(angle); Serial.println(") ---");
      myServo.attach(pin);
      myServo.write(angle);
      channel.setNewValue(true);
    }

    void turnOff(_supla_int_t duration) override {
      int angle = getAngle(false);
      Serial.print("--- ZAMYKAM (Kat: "); Serial.print(angle); Serial.println(") ---");
      myServo.attach(pin);
      myServo.write(angle);
      channel.setNewValue(false);
    }
};

Supla::ESPWifi wifi;
Supla::EspWebServer suplaServer;

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- START ---");
  
  // 1. Inicjalizacja Pamięci (I zapisanie wskaźnika!)
  suplaConfig = new Supla::LittleFsConfig();
  
  // 2. Dioda
  new Supla::Device::StatusLed(STATUS_LED_PIN, true);

  // 3. GUI (Strona WWW)
  new Supla::Html::CustomParameter("tekst_bolu", HTML_BOLU);

  // Pola konfiguracji (ID muszą pasować do tych w getString powyżej)
  new Supla::Html::CustomTextParameter("angle_open", "Kat OTWARTY (0-180)", 3);
  new Supla::Html::CustomTextParameter("angle_closed", "Kat ZAMKNIETY (0-180)", 3);
  new Supla::Html::CustomCheckboxParameter("logic_invert", "Odwroc logike");

  new Supla::Html::WifiParameters();
  new Supla::Html::ProtocolParameters();
  new Supla::Html::StatusLedParameters();
  new Supla::Html::DeviceInfo(&SuplaDevice);

  // 4. Przycisk Config
  auto cfgButton = new Supla::Control::Button(CONFIG_BUTTON_PIN, true, true);
  cfgButton->addAction(Supla::ENTER_CONFIG_MODE, &SuplaDevice, Supla::ON_HOLD);

  // 5. Serwo
  auto myServoValve = new ServoRelay(SERVO_PIN);

  // 6. Przycisk ścienny (D2)
  auto wallButton = new Supla::Control::Button(BUTTON_PIN, true, true);
  // ON_CHANGE = Reakcja na każdą zmianę stanu (dla włącznika ściennego bistabilnego)
  wallButton->addAction(Supla::TOGGLE, myServoValve, Supla::ON_CHANGE);

  // 7. Nazwa
  SuplaDevice.setName("Supla Serwo Bolu");

  // 8. Start
  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}