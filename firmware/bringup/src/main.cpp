// Carrier board bring-up test. Repeats every 5 s and reports over USB serial:
//  - idle levels on CAN RX (GPIO8) and RS485 RX (GPIO2)
//  - CAN: TWAI in no-ACK self-test mode. The controller reads its own frame back through the
//    transceiver (TX GPIO7 -> TJA1051 -> RX GPIO8), so a pass proves the transceiver path.
//  - RS485: driver enabled (DE GPIO4), bytes sent on UART1 TX (GPIO1). ~RE is tied low, so the
//    receiver always listens and the bytes should echo back on RX (GPIO2).
// No external wiring needed. Both transceivers run from the carrier's 5 V rail, which is only
// powered from the NMEA 2000 terminal (12 V), not from USB.
#include <Arduino.h>
#include "driver/twai.h"

static const int PIN_CAN_TX = 7;
static const int PIN_CAN_RX = 8;
static const int PIN_485_TX = 1;
static const int PIN_485_RX = 2;
static const int PIN_485_DE = 4;
static const int PIN_LED = 21;  // XIAO user LED, active low

static bool testCan() {
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)PIN_CAN_TX, (gpio_num_t)PIN_CAN_RX, TWAI_MODE_NO_ACK);
  twai_timing_config_t t = TWAI_TIMING_CONFIG_250KBITS();
  twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  if (twai_driver_install(&g, &t, &f) != ESP_OK) { Serial.println("CAN: driver install failed"); return false; }
  twai_start();

  int ok = 0;
  const int N = 10;
  for (int i = 0; i < N; i++) {
    twai_message_t tx = {};
    tx.identifier = 0x123;
    tx.data_length_code = 8;
    tx.self = 1;  // self-reception request
    for (int b = 0; b < 8; b++) tx.data[b] = (uint8_t)(i * 8 + b);
    if (twai_transmit(&tx, pdMS_TO_TICKS(50)) != ESP_OK) continue;
    twai_message_t rx;
    if (twai_receive(&rx, pdMS_TO_TICKS(50)) == ESP_OK && rx.identifier == tx.identifier &&
        memcmp(rx.data, tx.data, 8) == 0) ok++;
  }
  twai_status_info_t s;
  twai_get_status_info(&s);
  Serial.printf("CAN: %d/%d frames looped back; state %d (0 stopped,1 running,2 bus-off,3 recovering) TEC %lu REC %lu bus_err %lu\n",
                ok, N, (int)s.state, (unsigned long)s.tx_error_counter, (unsigned long)s.rx_error_counter,
                (unsigned long)s.bus_error_count);
  twai_stop();
  twai_driver_uninstall();
  return ok == N;
}

static bool test485() {
  Serial1.begin(38400, SERIAL_8N1, PIN_485_RX, PIN_485_TX);
  while (Serial1.available()) Serial1.read();
  const char msg[] = "!BRINGUP,0123456789*00\r\n";
  const size_t n = sizeof(msg) - 1;
  digitalWrite(PIN_485_DE, HIGH);
  delayMicroseconds(50);
  Serial1.write((const uint8_t *)msg, n);
  Serial1.flush();  // wait until the last bit has left
  digitalWrite(PIN_485_DE, LOW);
  delay(20);
  char got[64];
  size_t m = 0;
  while (Serial1.available() && m < sizeof(got) - 1) got[m++] = (char)Serial1.read();
  got[m] = 0;
  bool pass = (m == n) && memcmp(got, msg, n) == 0;
  Serial.printf("RS485: sent %u bytes, echoed %u%s\n", (unsigned)n, (unsigned)m, pass ? " (match)" : (m ? " (MISMATCH)" : ""));
  Serial1.end();
  return pass;
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_485_DE, OUTPUT);
  digitalWrite(PIN_485_DE, LOW);
  Serial.begin(115200);
  delay(1500);
  Serial.println("\ncarrier bring-up test");
}

void loop() {
  pinMode(PIN_CAN_RX, INPUT);
  pinMode(PIN_485_RX, INPUT);
  delay(5);
  Serial.printf("--- %lu s: idle levels CAN RX(GPIO8)=%d  RS485 RX(GPIO2)=%d (1 = recessive/idle)\n",
                millis() / 1000, digitalRead(PIN_CAN_RX), digitalRead(PIN_485_RX));
  bool can = testCan();
  bool rs = test485();
  Serial.printf("RESULT: CAN %s, RS485 %s\n", can ? "PASS" : "FAIL", rs ? "PASS" : "FAIL");
  // LED: one blink = both pass, three = something failed
  int blinks = (can && rs) ? 1 : 3;
  for (int i = 0; i < blinks; i++) { digitalWrite(PIN_LED, LOW); delay(100); digitalWrite(PIN_LED, HIGH); delay(200); }
  delay(5000);
}
