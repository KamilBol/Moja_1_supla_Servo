#include <Arduino.h>
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/relay.h>
#include <supla/device/status_led.h>
#include <Servo.h>

// ================= KONFIGURACJA =================
const char* WIFI_SSID = "TUTAJ_WPISZ_NAZWE_WIFI";
const char* WIFI_PASS = "TUTAJ_WPISZ_HASLO_WIFI";
const char* SUPLA_SERVER = "svrX.supla.org"; // Twój adres serwera (np. svr1.supla.org)
const char* SUPLA_EMAIL = "twoj@email.com";  // Twój email do Supli

// PINY (Dla Wemos D1 Mini)
const int SERVO_PIN = D1;      // Tu podłączasz sygnał serwa
const int STATUS_LED_PIN = D4; // Wbudowana dioda LED

// KĄTY SERWA
const int KAT_OTWARTY = 90;
const int KAT_ZAMKNIETY = 0;

// ================= KLASA WŁASNA (MAGICZNA) =================
// To sprawia, że serwo udaje przekaźnik w Supli
class ServoRelay : public Supla::Control::Relay {
  private:
    Servo myServo;
    int pin;

  public:
    ServoRelay(int pinGpio) : Relay(pinGpio, false), pin(pinGpio) {
      // Konstruktor: ustawiamy serwo na start
      myServo.attach(pin);
      myServo.write(KAT_ZAMKNIETY);
    }

    // Nadpisujemy włączenie (zamiast prądu, ruszamy serwem)
    void turnOn(_supla_int_t duration) override {
      Serial.println("SUPLA: Włączono -> Otwieram Serwo");
      myServo.attach(pin); 
      myServo.write(KAT_OTWARTY);
      channel.setNewStatus(true); // Zmieniamy ikonę w apce na ON
    }

    // Nadpisujemy wyłączenie
    void turnOff(_supla_int_t duration) override {
      Serial.println("SUPLA: Wyłączono -> Zamykam Serwo");
      myServo.write(KAT_ZAMKNIETY);
      channel.setNewStatus(false); // Zmieniamy ikonę w apce na OFF
    }
};

// Obiekt WiFi
Supla::ESPWifi wifi(WIFI_SSID, WIFI_PASS);

void setup() {
  Serial.begin(115200);
  
  // Konfiguracja Supla
  // GUID i AUTH KEY biblioteka wygeneruje sama przy pierwszym uruchomieniu
  // (pod warunkiem włączonej rejestracji urządzeń w Cloudzie)
  SuplaDevice.begin(0, SUPLA_SERVER, SUPLA_EMAIL, nullptr);

  // Dioda statusu (miga jak szuka sieci)
  new Supla::Device::StatusLed(STATUS_LED_PIN, true);

  // Dodajemy nasze Serwo do systemu
  new ServoRelay(SERVO_PIN);
}

void loop() {
  SuplaDevice.iterate();
}