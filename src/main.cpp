// Queue + Display + Arduino
#include <cppQueue.h>
#include <Arduino.h>
#include <U8g2lib.h>

// Interrupts 
#include <avr/io.h>
#include <avr/interrupt.h>

// Hardware Interrupts
volatile uint16_t latestReadingA0 = 0;
volatile uint16_t latestReadingA1 = 0;
volatile uint16_t latestReadingA2 = 0;
volatile bool newReadingAvailableA0 = false;
volatile bool newReadingAvailableA1 = false;
volatile bool newReadingAvailableA2 = false;

// Selects which channel to read from in the next ADC conversion
volatile uint8_t currentChannel = 0; // 0 = A0, 1 = A1, 2 = A2

// Analog Readings
uint16_t valA0;
uint16_t valA1;
uint16_t valA2;
uint16_t filteredA2;

// Button Control
uint8_t Y_Pointer = 4;
uint8_t Y_Scale_Values[10] = {1, 2, 4, 8, 10, 12, 14, 16, 18, 20};
const unsigned long DEBOUNCE_MS = 500;
const int Y_MAX_INDEX = 9;

uint8_t pin2state;  // Pin D2 = DDRD 2
uint8_t pin3state;  // Pin D3 = DDRD 3
uint8_t pin4state;  // Pin D4 = DDRD 4
uint8_t pin5state;  // Pin D5 = DDRD 5
int prev_time;    

// Display Pins
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
bool pin_State = LOW;

// QUEUE + DATA
cppQueue q(sizeof(float), GRAPH_WIDTH, FIFO);

void setupTimer1() {
  cli();

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // Example: prescaler 64, OCR1A = 249 -> 1ms period (1kHz sample rate)
  // Adjust OCR1A for your desired sample interval: OCR1A = 249 by default
  // period(s) = (OCR1A + 1) * prescaler / 16,000,000;
  OCR1A = 249;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);

  // Enable Compare Match A interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();
}

void setupADC() {
  // AVcc reference, start on channel A0
  ADMUX = (1 << REFS0); // channel bits = 0000 -> A0

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
  // Select the channel to sample this tick, then start conversion
  ADMUX = (ADMUX & 0xF0) | (currentChannel & 0x0F); // keep REFS bits, set MUX bits
  ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
  if (currentChannel == 0) {
    latestReadingA0 = ADC;
    newReadingAvailableA0 = true;
    currentChannel = 1; // next tick: A1
  } else if (currentChannel == 1) {
    latestReadingA1 = ADC;
    newReadingAvailableA1 = true;
    currentChannel = 2; // next tick: A2
  } else {
    latestReadingA2 = ADC;
    newReadingAvailableA2 = true;
    currentChannel = 0; // next tick: back to A0
  }
}

void gen_signal(){
  unsigned long curr_time = millis(); 
  
  if (curr_time - previous_time >= TIME_HIGH){  
    PORTD ^= (1 << PORTD7);  // toggle pin 7
    previous_time = curr_time;
  }
}

void queue_Control(uint16_t rawA0, uint16_t rawA1, bool a0IsNew, bool a1IsNew) {
  const float ALPHA = 0.9f;
  const float THRESHOLD = 0.25f;

  static float filteredA0 = 0.0f;
  static float filteredA1 = 0.0f;

  if (a0IsNew) {
    float readingA0 = rawA0 * 4.81f / 1023.0f;
    filteredA0 = ALPHA * readingA0 + (1 - ALPHA) * filteredA0;
  }
  if (a1IsNew) {
    float readingA1 = rawA1 * 4.81f / 1023.0f;
    filteredA1 = ALPHA * readingA1 + (1 - ALPHA) * filteredA1;
  }

  if (q.isFull()) {
    float discard;
    q.pop(&discard);
  }

  float value = 0.0f;
  if (filteredA0 >= THRESHOLD) {
    value = filteredA0;
  } else if (filteredA1 >= THRESHOLD) {
    value = -filteredA1;
  }

  q.push(&value);
}

void grid(){
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
}

uint16_t filterA2(uint16_t rawA2) {
  const float ALPHA_A2 = 0.6f; 

  static float filteredA2 = 0.0f;

  filteredA2 = ALPHA_A2 * static_cast<float>(rawA2) + (1 - ALPHA_A2) * filteredA2;

  return filteredA2;
}

void graph() {
  uint8_t count = q.getCount();
  uint8_t midpoint = map(filteredA2, 1023, 0, 0, 63);

  if (count < 2) return;

  float temps[GRAPH_WIDTH];
  for (uint8_t i = 0; i < count; i++) {
    q.peekIdx(&temps[i], i);
  }

  for (uint8_t i = 0; i < count - 1; i++) {
    uint8_t x1 = GRAPH_WIDTH - count + i;
    uint8_t x2 = x1 + 1;

    // temps[i] can now be positive (A0) or negative (A1) —
    // midpoint - value handles both directions accurately
    int y1 = midpoint - static_cast<int>(temps[i] * Y_SCALE);
    int y2 = midpoint - static_cast<int>(temps[i + 1] * Y_SCALE);

    y1 = constrain(y1, 0, GRAPH_HEIGHT - 1);
    y2 = constrain(y2, 0, GRAPH_HEIGHT - 2);

    display.drawLine(x1, y1, x2, y2);
  }
}

void render() {
  display.firstPage();
  do {

    grid();

    graph();

  } while (display.nextPage());
}

void Scale_Control() {
  if (pin2state == 0 && pin3state == 0) return; 

  unsigned long curr_time = millis();
  
  if (curr_time - prev_time <= DEBOUNCE_MS) return;

  if (pin2state == 1 && Y_Pointer < Y_MAX_INDEX) {
    Y_Pointer++;
  } else if (pin3state == 1 && Y_Pointer > 0) {
    Y_Pointer--;
  } else {
    return;
  }

  prev_time = curr_time;
  Y_SCALE = Y_Scale_Values[Y_Pointer];
}

void pinStates() {
  pin2state = (PIND >> 2) & 0x01;
  pin3state = (PIND >> 3) & 0x01;
  pin4state = (PIND >> 4) & 0x01;
  pin5state = (PIND >> 5) & 0x01;
}

void setup() {
  Serial.begin(115200);

  DDRD |= (1 << DDD7);   // set PD7 (pin 7) as OUTPUT
  DDRD &= ~((1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5));

  setupADC();
  setupTimer1();

  previous_time = millis(); // Start time of the program for PWM signal generation
  prev_time = millis();

  // Start Screen
  display.begin();
  delay(100);              
  display.setContrast(200);

}

void loop() { 

  // Generate PWM Signal w/o Blocking Pins 
  gen_signal();
  pinStates();
  Scale_Control();
  
  bool a0IsNew = false;
  bool a1IsNew = false;

  if (newReadingAvailableA0) {
    noInterrupts();
    valA0 = latestReadingA0;
    newReadingAvailableA0 = false;
    interrupts();
    a0IsNew = true;
  }

  if (newReadingAvailableA1) {
    noInterrupts();
    valA1 = latestReadingA1;
    newReadingAvailableA1 = false;
    interrupts();
    a1IsNew = true;
  }

  if (a0IsNew || a1IsNew) {
    queue_Control(valA0, valA1, a0IsNew, a1IsNew);
  }
  
  if (newReadingAvailableA2) {
    
    noInterrupts();
    valA2 = latestReadingA2;
    newReadingAvailableA2 = false;
    interrupts();

    filteredA2 = filterA2(valA2);
  }

  // Build pixel data once, outside the page loop
  render();
  Serial.print("Filtered A2: ");
  Serial.println(filteredA2);
}
