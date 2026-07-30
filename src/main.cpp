// Imports, Display and Arduino
#include <Arduino.h>
#include <U8g2lib.h>

// * ============== All Analog Variables and Related Setup ============== * //
// Hardware Interrupts
volatile uint16_t latestReadingA0 = 0;
volatile uint16_t latestReadingA1 = 0;
volatile uint16_t latestReadingA2 = 0;
volatile bool newReadingAvailableA0 = false;
volatile bool newReadingAvailableA1 = false;
volatile bool newReadingAvailableA2 = false;

// Selects which channel to read from in the next ADC conversion
volatile uint8_t currentChannel = 0; // 0 = A0, 1 = A1, 2 = A2


// * ================ All Button Control Related Variables ================ * //
uint8_t Y_Pointer = 2;
uint8_t Y_Scale_Values[15] = {1, 2, 4, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30};
uint16_t Y_SCALE = Y_Scale_Values[Y_Pointer];
const int Y_MAX_INDEX = 14;

uint8_t X_Pointer = 2;
uint16_t X_Scale_Values[15] = {1, 2, 3, 5, 7, 10, 15, 20, 30, 45, 65, 100, 150, 220, 300};
uint16_t X_SCALE = X_Scale_Values[X_Pointer];
const int X_MAX_INDEX = 14;

volatile uint16_t X_DECIMATION = 1;   // current averaging factor, set from X_SCALE
volatile float    accumSum = 0.0f;    // running sum of raw combined samples
volatile uint16_t accumCount = 0;     // how many raw samples summed so far

const uint16_t cooldown_Time = 500; // Control the coolDownTime time for button presses
const uint16_t debounce_Time = 50; // Control the debounce time for button presses

uint8_t pin2state;  // Pin D2 = DDRD 2
uint8_t pin3state;  // Pin D3 = DDRD 3
uint8_t pin4state;  // Pin D4 = DDRD 4
uint8_t pin5state;  // Pin D5 = DDRD 5
unsigned long prev_time_Y;    
unsigned long prev_time_X;

// Debounce tracking - one set per pin
uint8_t pin2lastReading = LOW;
uint8_t pin3lastReading = LOW;
uint8_t pin4lastReading = LOW;
uint8_t pin5lastReading = LOW;
unsigned long pin2highSince = 0;
unsigned long pin3highSince = 0;
unsigned long pin4highSince = 0;
unsigned long pin5highSince = 0;

// * ========================= Screen Variables ========================= * //
// SPI Pin Assigments
const uint8_t PIN_RES = 8;
const uint8_t PIN_DC = 9;
const uint8_t PIN_CS = 10;

// Screen Details 
constexpr uint8_t  GRAPH_WIDTH = 128;
constexpr uint8_t  GRAPH_HEIGHT = 64;   

// Screen Initialisation from U8g2lib.h
U8G2_SH1106_128X64_NONAME_2_4W_HW_SPI display(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

//TODO: Irrelevant to the main logic, Remove when finished project
const int TIME_HIGH = 1200;
const int TIME_LOW = 1200; 
long previous_time; 
bool pin_State = LOW;

// * ========================= Queue Variables  & Functions ========================= * //
float ringBuf[GRAPH_WIDTH] = {0};
volatile uint16_t ringHead = 0;   // next write position
volatile uint16_t ringCount = 0;  // number of valid entries

void ringPush(float v) {
  ringBuf[ringHead] = v;
  ringHead = (ringHead + 1) % GRAPH_WIDTH;
  if (ringCount < GRAPH_WIDTH) ringCount++;
}

// idx 0 = oldest, idx (count-1) = newest
float ringPeek(uint16_t idx) {
  uint16_t start = (ringHead + GRAPH_WIDTH - ringCount) % GRAPH_WIDTH;
  return ringBuf[(start + idx) % GRAPH_WIDTH];
}

void queue_Control(uint16_t rawA0, uint16_t rawA1) {
  const float THRESHOLD = 0.05f;
  float value;

  float readingA0 = rawA0 * 5.000f / 1023.0f;
  float readingA1 = rawA1 * 5.000f / 1023.0f;

  if (readingA0 >= THRESHOLD) {
    value = readingA0;
  } else if (readingA1 >= THRESHOLD) {
    value = -readingA1;
  } else {
    value = 0.0f;
  }

  accumSum += value;
  accumCount++;

  if (accumCount >= X_DECIMATION) {
    ringPush(accumSum / accumCount);   // average, not raw — this is the anti-alias step
    accumSum = 0.0f;
    accumCount = 0;
  }
}

// * ========================= ADC Functions for Interrupts ========================= * //
void setupTimer1() {
  cli();//stop interrupts

  // Initiate Registers at 0
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 249; // starting freq, e.g. 5Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // prescaler 256 (CS12=1, CS11=0, CS10=0)
  TIMSK1 |= (1 << OCIE1A);


  sei();//allow interrupts
}

void setupADC() {
  ADMUX = (1 << REFS0); // AVcc reference, start on channel A0 (MUX bits = 0000)
  ADCSRA =
    (1 << ADEN) |   // enable ADC
    (1 << ADIE) |   // enable ADC interrupt
    (1 << ADPS2);   // prescaler 16 -> 1 MHz ADC clock
}

ISR(TIMER1_COMPA_vect) {
  currentChannel = 0;
  ADMUX = (ADMUX & 0xF0) | currentChannel; // select A0
  ADCSRA |= (1 << ADSC);                    // start conversion
}

ISR(ADC_vect) {
  uint16_t result = ADC;

  switch (currentChannel) {
    case 0:
      latestReadingA0 = result;
      currentChannel = 1;
      ADMUX = (ADMUX & 0xF0) | currentChannel; // select A1
      ADCSRA |= (1 << ADSC);                    // start next conversion
      break;

    case 1:
      latestReadingA1 = result;
      currentChannel = 2;
      ADMUX = (ADMUX & 0xF0) | currentChannel; // select A2
      ADCSRA |= (1 << ADSC);                    // start next conversion
      break;

    case 2:
      latestReadingA2 = result;
      queue_Control(latestReadingA0, latestReadingA1); // full round done
      // don't restart ADSC here — wait for next Timer1 tick
      break;
  }
}

// ========================== Filter A2 Readings for Midpoint Line ========================= //
uint16_t filterA2(uint16_t rawA2) {
  const float ALPHA_A2 = 0.2f; 

  static float filteredA2 = 0.0f;

  filteredA2 = ALPHA_A2 * static_cast<float>(rawA2) + (1 - ALPHA_A2) * filteredA2;

  return filteredA2;
}

//* ========================= Button Control Functions ========================= * //
void pinStates() {
  pin2state = (PIND >> 2) & 0x01;
  pin3state = (PIND >> 3) & 0x01;
  pin4state = (PIND >> 4) & 0x01;
  pin5state = (PIND >> 5) & 0x01;
}

bool debounce(uint8_t rawReading, uint8_t &lastReading, unsigned long &highSince) {
  unsigned long now = millis();

  if (rawReading == HIGH) {
    if (lastReading == LOW) {
      highSince = now; // just went HIGH, start timing
    }
    lastReading = HIGH;
    return (now - highSince) >= debounce_Time;
  } else {
    lastReading = LOW;
    return false;
  }
}

void Y_Scale_Control() {
  bool pin2debounced = debounce(pin2state, pin2lastReading, pin2highSince);
  bool pin3debounced = debounce(pin3state, pin3lastReading, pin3highSince);

  if (!pin2debounced && !pin3debounced) return;

  unsigned long curr_time = millis();
  if (curr_time - prev_time_Y <= cooldown_Time) return; // Cooldown check

  if (pin2debounced && Y_Pointer < Y_MAX_INDEX) {
    Y_Pointer++;
  } else if (pin3debounced && Y_Pointer > 0) {
    Y_Pointer--;
  } else {
    return;
  }

  prev_time_Y = curr_time;
  Y_SCALE = Y_Scale_Values[Y_Pointer];
}

void X_Scale_Control() {
  bool pin4debounced = debounce(pin4state, pin4lastReading, pin4highSince);
  bool pin5debounced = debounce(pin5state, pin5lastReading, pin5highSince);

  if (!pin4debounced && !pin5debounced) return;

  unsigned long curr_time = millis();
  if (curr_time - prev_time_X <= cooldown_Time) return;

  if (pin4debounced && X_Pointer < X_MAX_INDEX) {
    X_Pointer++;
  } else if (pin5debounced && X_Pointer > 0) {
    X_Pointer--;
  } else {
    return;
  }

  prev_time_X = curr_time;
  X_SCALE = X_Scale_Values[X_Pointer];

  noInterrupts();
  X_DECIMATION = X_SCALE;
  accumSum = 0.0f;     // reset so you don't get one weird partial-average frame right after a change
  accumCount = 0;
  interrupts();
}

//* ========================= Graphing Functions ========================= * //
// ? Creates a background grid for the graph to be drawn on, maybe remove later

// Draws the Graph using filtered readings from A0 and A1
void graph() {

  // Read from A2 - Determine the midpoint for the graph
  noInterrupts();
  uint16_t a2 = latestReadingA2;
  interrupts();
  uint16_t midpoint = map(filterA2(a2), 1023, 0, 0, 63);

  float temps[GRAPH_WIDTH];
  uint16_t count = ringCount;
  
  if (count < 2) return; // Don't want too few readings

  for (uint16_t i = 0; i < count; i++) {
    temps[i] = ringPeek(i); // Read all values in Queue until now
  }
  
  // Graph it from midpoint, constained to fit in screen, like a real oscilloscope
  for (uint16_t i = 0; i < count - 1; i++) {
    uint16_t x1 = GRAPH_WIDTH - count + i;
    uint16_t x2 = x1 + 1;

    /* temps[i] can now be positive (A0) or negative (A1) —
    midpoint - value handles both directions accurately */
    int y1 = midpoint - static_cast<int>(temps[i] * Y_SCALE);
    int y2 = midpoint - static_cast<int>(temps[i + 1] * Y_SCALE);

    y1 = constrain(y1, 0, GRAPH_HEIGHT);
    y2 = constrain(y2, 0, GRAPH_HEIGHT - 1);

    display.drawLine(x1, y1, x2, y2);
  } display.drawLine(0, midpoint, GRAPH_WIDTH - 1, midpoint); // Draw the midpoint line
}

void graph1() {
  noInterrupts();
  uint16_t a2 = latestReadingA2;
  uint16_t count = ringCount;
  uint16_t head  = ringHead;
  interrupts();

  uint16_t midpoint = map(filterA2(a2), 1023, 0, 0, 63);
  if (count < 2) return;

  for (uint16_t i = 0; i < count - 1; i++) {
    uint16_t x1 = GRAPH_WIDTH - count + i;
    uint16_t x2 = x1 + 1;

    uint16_t idx1 = (head + GRAPH_WIDTH - count + i) % GRAPH_WIDTH;
    uint16_t idx2 = (head + GRAPH_WIDTH - count + i + 1) % GRAPH_WIDTH;

    int y1 = midpoint - static_cast<int>(ringBuf[idx1] * Y_SCALE);
    int y2 = midpoint - static_cast<int>(ringBuf[idx2] * Y_SCALE);

    y1 = constrain(y1, 0, GRAPH_HEIGHT);
    y2 = constrain(y2, 0, GRAPH_HEIGHT - 1);

    display.drawLine(x1, y1, x2, y2);
  }
  display.drawLine(0, midpoint, GRAPH_WIDTH - 1, midpoint);
}

// Render calls grid() and graph() to draw screen easily
void render() {
  display.firstPage();
  do {

    graph1();

  } while (display.nextPage());
}

// * ======================== Setup and Loop ========================= * //
void setup() {
  Serial.begin(9600);

  DDRD |= (1 << DDD7);   // set PD7 (pin 7) as OUTPUT
  DDRD &= ~((1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5));

  setupADC();
  setupTimer1();

  previous_time = prev_time_Y = prev_time_X = millis(); // Start time of the program for PWM signal generation

  // Start Screen
  display.begin();
  delay(100);              
  display.setContrast(200);

}

void loop() { 

  // Button Control for Y and X Scale, and Pin State Detection
  pinStates();
  Y_Scale_Control();
  X_Scale_Control();
  
  // Build pixel data once, outside the page loop
  render();
}
