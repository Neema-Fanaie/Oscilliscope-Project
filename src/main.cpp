#include <cppQueue.h> // Queue Data Structure
#include <Arduino.h>
#include <U8g2lib.h>

// Interrupts 
#include <avr/io.h>
#include <avr/interrupt.h>

// Hardware Interrupts
volatile uint16_t latestReading = 0;
volatile bool newReadingAvailable = false;
uint16_t reading_tot = 0;
float reading;


const int PWM_Pin = 7;
const int Read_Pin = A0;
const int Width_Pin = A1;

// SPI PINS
// #define SCK 13
// #define MOSI 11
#define PIN_RES 8
#define PIN_DC 9
#define PIN_CS 10

int GRAPH_WIDTH = 128;
int GRAPH_HEIGHT = 65;   // adjust to your display height
int Y_SCALE = 10;        // adjust scaling to fit your temp range
int X_SCALE = 1;

// Screen Details 
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;

// SCREEN
U8G2_SH1106_128X64_NONAME_2_4W_HW_SPI display(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

// PWM 
const int TIME_HIGH = 1200;
const int TIME_LOW = 1200; 
long previous_time; 
long current_time;
bool pin_State = LOW;

// QUEUE + DATA
cppQueue q(sizeof(float), GRAPH_WIDTH, FIFO);

uint8_t pin2state;
uint8_t pin3state;

void setupTimer1() {
    cli();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Example: prescaler 64, OCR1A = 249 -> 1ms period (1kHz sample rate)
    // Adjust OCR1A for your desired sample interval:
    // period(s) = (OCR1A + 1) * prescaler / 16,000,000    OCR1A = 249;
    OCR1A = 249;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // Enable Compare Match A interrupt
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

void setupADC() {
    // AVcc reference, channel A0
    ADMUX = (1 << REFS0);

    // Enable ADC
    // Enable ADC interrupt
    // Prescaler = 16 (1 MHz ADC clock)
    ADCSRA =
        (1 << ADEN) |
        (1 << ADIE) |
        (1 << ADPS2);
}

// Timer1 fires every 1 ms
ISR(TIMER1_COMPA_vect) {
    // Start an ADC conversion
    ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
    latestReading = ADC;
    newReadingAvailable = true;
}

void gen_signal(){
  current_time = millis();
  
  if (current_time - previous_time >= TIME_HIGH){  // works for both since TIME_HIGH == TIME_LOW
    PORTD ^= (1 << PORTD7);  // toggle pin 7
    previous_time = current_time;
  }
}

void queue_Control(float reading){
  if (q.isFull()){
    int my_val;
    
    q.pop(&my_val);
    q.push(&reading);
  
  } else {
    
    q.push(&reading);
  
  }
}

void render() {
  display.firstPage();
  do {

    // --- grid drawing ---
    for (uint8_t i = 0; i <= 8; i++) {
      for (uint8_t j = 0; j <= 4; j++) {
        uint8_t x = i * 16;
        uint8_t y = j * 16;

        if (x == 0 && y == 0){
          display.drawLine(x, y, x+1, y);
          display.drawLine(x, y, x, y+1);

          display.drawLine(x + 6, y, x + 9, y);
          display.drawLine(x, y + 6, x, y + 9);

        } else if (x == 0 ){
        
          display.drawLine(x, y-1, x+1, y-1);
          display.drawLine(x, y-2, x, y);

          display.drawLine(x + 6, y - 1 , x + 9, y -1);
          display.drawLine(x, y + 5 , x, y + 8);
        
        } else if (y == 0){

          display.drawLine(x-2, y, x, y);
          display.drawLine(x-1, y, x-1, y+1);         

          display.drawLine(x + 5, y, x + 8, y);
          display.drawLine(x - 1 , y + 6 , x - 1, y + 9); 
        
        } else {
          display.drawLine(x - 2, y - 1 , x, y -1);
          display.drawLine(x - 1, y -2, x - 1 , y);

          display.drawLine(x + 6, y - 1 , x + 9, y -1);
          display.drawLine(x - 1 , y + 6 , x - 1, y + 9); 
        }
      }
    }

    // --- graph drawing ---
    int count = q.getCount();
    if (count >= 2) {
      float temps[GRAPH_WIDTH];
      for (int i = 0; i < count; i++) {
        q.peekIdx(&temps[i], i);
      }
      for (int i = 0; i < count - 1; i++) {
        int x1 = GRAPH_WIDTH - count + i;
        int y1 = GRAPH_HEIGHT - static_cast<int>(temps[i] * Y_SCALE * 10) / 10 - 2;
        int x2 = GRAPH_WIDTH - count + i + 1;
        int y2 = GRAPH_HEIGHT - static_cast<int>(temps[i + 1] * Y_SCALE * 10) / 10 - 2;
        display.drawLine(x1, y1, x2, y2);
      }
    }

  } while (display.nextPage());
}

void pinStates(){
  pin2state = (PIND >> 2) & 0x01;
  pin3state = (PIND >> 3) & 0x01;
}

void setup() {
  Serial.begin(115200);

  DDRD |= (1 << DDD7);   // set PD7 (pin 7) as OUTPUT
  DDRD &= ~((1 << DDD2) | (1 << DDD3)); // set PD2 (pin 2) and PD3 (pin 3) as INPUT

  setupADC();
  setupTimer1();

  // Initial time
  current_time = millis();

  // Start Screen
  display.begin();
  delay(100);              
  display.setContrast(200);

}

void loop() { 
  // Generate PWM Signal w/o Blocking Pins 
  gen_signal();

  if (newReadingAvailable)
  {
      noInterrupts();
      uint16_t val = latestReading;
      newReadingAvailable = false;
      interrupts();
      reading = val * 4.81/ 1023.0;
      Serial.println(reading);
  }

  queue_Control(reading);
  
  // Build pixel data once, outside the page loop
  render();
}
