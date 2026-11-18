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

// DC loopback: drive CAN TX as a plain GPIO and read RX. Independent of bit timing and
// termination. The TJA1051's TXD dominant time-out releases a long dominant, so RX is
// sampled right after TX goes low.
static bool testCanDc() {
  // Reset both pins so no peripheral (TWAI from a previous round) is still routed to them.
  gpio_reset_pin((gpio_num_t)PIN_CAN_TX);
  gpio_reset_pin((gpio_num_t)PIN_CAN_RX);
  gpio_set_direction((gpio_num_t)PIN_CAN_RX, GPIO_MODE_INPUT);
  // Pull-down on RX: a high reading then proves the TJA1051's RXD is driving the line. With no
  // pull (or the pull-up gpio_reset_pin() enables), an open RX joint reads as a healthy idle
  // high. That hid an unsoldered D9 on the first board.
  gpio_set_pull_mode((gpio_num_t)PIN_CAN_RX, GPIO_PULLDOWN_ONLY);
  // TX released (input, no pull): the TJA1051's internal TXD pull-up should hold it recessive.
  gpio_set_direction((gpio_num_t)PIN_CAN_TX, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)PIN_CAN_TX, GPIO_FLOATING);
  delayMicroseconds(200);
  int txFloat = gpio_get_level((gpio_num_t)PIN_CAN_TX), rxFloat = gpio_get_level((gpio_num_t)PIN_CAN_RX);
  // Drive TX with readback of the pad itself.
  gpio_set_direction((gpio_num_t)PIN_CAN_TX, GPIO_MODE_INPUT_OUTPUT);
  gpio_set_level((gpio_num_t)PIN_CAN_TX, 1);
  delayMicroseconds(200);
  int txHi = gpio_get_level((gpio_num_t)PIN_CAN_TX), rxHi = gpio_get_level((gpio_num_t)PIN_CAN_RX);
  gpio_set_level((gpio_num_t)PIN_CAN_TX, 0);
  int rxLo[5];
  int txLo = 0;
  for (int i = 0; i < 5; i++) {  // samples at ~0, 5, 10, 20, 50 us
    static const int at[5] = {0, 5, 10, 20, 50};
    static int last = 0;
    if (i == 0) last = 0;
    delayMicroseconds(at[i] - last);
    last = at[i];
    rxLo[i] = gpio_get_level((gpio_num_t)PIN_CAN_RX);
    if (i == 2) txLo = gpio_get_level((gpio_num_t)PIN_CAN_TX);
  }
  gpio_set_level((gpio_num_t)PIN_CAN_TX, 1);
  delayMicroseconds(200);
  int rxAfter = gpio_get_level((gpio_num_t)PIN_CAN_RX);
  gpio_set_pull_mode((gpio_num_t)PIN_CAN_RX, GPIO_FLOATING);
  bool pass = txHi == 1 && txLo == 0 && rxHi == 1 && rxLo[3] == 0 && rxAfter == 1;
  Serial.printf("CAN DC: released TX=%d RX=%d | TX set 1: pad %d RX %d | TX set 0: pad %d RX %d%d%d%d%d (0-50us) | TX set 1: RX %d %s\n",
                txFloat, rxFloat, txHi, rxHi, txLo, rxLo[0], rxLo[1], rxLo[2], rxLo[3], rxLo[4], rxAfter,
                pass ? "(ok)" : "(FAIL)");
  return pass;
}

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
  // Leave TX recessive: after uninstall the pin can stay routed to the stopped controller.
  pinMode(PIN_CAN_TX, OUTPUT);
  digitalWrite(PIN_CAN_TX, HIGH);
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

#ifdef TXD_SQUARE
// Meter-friendly mode: 10 kHz 50 % square wave on CAN TX. Each low lasts 50 us, well under the
// TJA1051's 0.3 ms minimum TXD dominant time-out, so the transmitter keeps driving. A DC meter
// then reads averages: TXD ~VIO/2; CANH above and CANL below their 2.6 V recessive level if the
// transmitter drives. RX is sampled continuously and the share of low samples is printed.
void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_485_DE, OUTPUT);
  digitalWrite(PIN_485_DE, LOW);
  Serial.begin(115200);
  delay(1500);
  gpio_reset_pin((gpio_num_t)PIN_CAN_TX);
  gpio_reset_pin((gpio_num_t)PIN_CAN_RX);
  gpio_set_direction((gpio_num_t)PIN_CAN_RX, GPIO_MODE_INPUT);
  gpio_set_direction((gpio_num_t)PIN_CAN_TX, GPIO_MODE_INPUT_OUTPUT);  // output with pad readback
  gpio_set_level((gpio_num_t)PIN_CAN_TX, 1);
}

// Software square wave: 50 us low, 50 us high. RX and the TX pad are sampled in the middle of
// each half. Interrupts stretch the odd half-period; the meter average is barely affected.
// The RX pull is set explicitly before each 1 s measurement and cycles up -> down -> none.
// (gpio_reset_pin() enables the pull-up, so it must never be left implicit.) A connected,
// working RXD output overrides the weak internal pull and toggles with TX; an RX line that
// is open between U2 pin 4 and the XIAO just follows the pull.
void loop() {
  static const gpio_pull_mode_t pulls[3] = {GPIO_PULLUP_ONLY, GPIO_PULLDOWN_ONLY, GPIO_FLOATING};
  static const char *names[3] = {"UP", "DOWN", "none"};
  static int round = 0;
  const char *rxPullName = names[round % 3];
  gpio_set_pull_mode((gpio_num_t)PIN_CAN_RX, pulls[round % 3]);
  round++;
  delay(2);
  uint32_t periods = 0, txLowOk = 0, txHighOk = 0, rxLowWhenTxLow = 0, rxHighWhenTxHigh = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 1000) {
    gpio_set_level((gpio_num_t)PIN_CAN_TX, 0);
    delayMicroseconds(25);
    if (gpio_get_level((gpio_num_t)PIN_CAN_TX) == 0) txLowOk++;
    if (gpio_get_level((gpio_num_t)PIN_CAN_RX) == 0) rxLowWhenTxLow++;
    delayMicroseconds(25);
    gpio_set_level((gpio_num_t)PIN_CAN_TX, 1);
    delayMicroseconds(25);
    if (gpio_get_level((gpio_num_t)PIN_CAN_TX) == 1) txHighOk++;
    if (gpio_get_level((gpio_num_t)PIN_CAN_RX) == 1) rxHighWhenTxHigh++;
    delayMicroseconds(25);
    periods++;
  }
  Serial.printf("[RX pull %s] TXD square ~10 kHz, %lu periods: TX pad follows low %lu / high %lu; RX low while TX low %lu, RX high while TX high %lu\n",
                rxPullName, (unsigned long)periods, (unsigned long)txLowOk, (unsigned long)txHighOk,
                (unsigned long)rxLowWhenTxLow, (unsigned long)rxHighWhenTxHigh);
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
}
#else
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
  bool dc = testCanDc();
  bool can = testCan();
  bool rs = test485();
  Serial.printf("RESULT: CAN DC %s, CAN frames %s, RS485 %s\n", dc ? "PASS" : "FAIL", can ? "PASS" : "FAIL",
                rs ? "PASS" : "FAIL");
  can = can && dc;
  // LED: one blink = both pass, three = something failed
  int blinks = (can && rs) ? 1 : 3;
  for (int i = 0; i < blinks; i++) { digitalWrite(PIN_LED, LOW); delay(100); digitalWrite(PIN_LED, HIGH); delay(200); }
  delay(5000);
}
#endif
