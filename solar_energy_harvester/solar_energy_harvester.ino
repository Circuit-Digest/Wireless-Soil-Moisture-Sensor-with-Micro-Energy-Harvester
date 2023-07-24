#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <RH_RF69.h>
#define Lora_En 3
#define Sense_En 7
RH_RF69 rf69;
const int AirValue = 620;    //you need to replace this value with Value_1
const int WaterValue = 310;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent = 0;
// watchdog interrupt
ISR(WDT_vect) {
  wdt_disable();  // disable watchdog
}  // end of WDT_vect
void setup() {
  Serial.begin(9600);
  pinMode(Lora_En, OUTPUT);
  pinMode(Sense_En, OUTPUT);
}


void loop() {
  digitalWrite(Sense_En, HIGH);
  // enble ADC
  ADCSRA = 0;
  soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  digitalWrite(Sense_En, LOW);
  digitalWrite(Lora_En, HIGH);
  rf69.init();
  rf69.setFrequency(865.985);
  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
  rf69.setEncryptionKey(key);
  rf69.send(soilMoistureValue, 2);
  rf69.waitPacketSent();
  digitalWrite(Lora_En, LOW);
  //sleep for aprox 1 hr( WDT 8S * 450 ~ 3600S)
  for (int i = 0; i < 450; i++) {

    // disable ADC
    ADCSRA = 0;
    // clear various "reset" flags
    MCUSR = 0;
    // allow changes, disable reset
    WDTCSR = bit(WDCE) | bit(WDE);
    // set interrupt mode and an interval
    WDTCSR = bit(WDIE) | bit(WDP3) | bit(WDP0);  // set WDIE, and 8 seconds delay
    wdt_reset();                                 // pat the dog
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    noInterrupts();  // timed sequence follows
    sleep_enable();
    // turn off brown-out enable in software
    MCUCR = bit(BODS) | bit(BODSE);
    MCUCR = bit(BODS);
    interrupts();
    sleep_cpu();
  }
  sleep_disable();
}