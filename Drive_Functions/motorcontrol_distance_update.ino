// No need to include Arduino.h — it's automatic in .ino

// ---------------- CONSTANTS (SI UNITS) ----------------
const float wheel_diam = 0.0655;      // meters
const float rpm = 53.6912751678;      // motor RPM
const float shaft_distance = 0.12;    // meters


// ---------------- FUNCTION PROTOTYPES ----------------
float time_delay_linear(float distance_m);
float time_delay_turn(float angle_deg);

void drive_forward(float distance_m);
void drive_backward(float distance_m);
void drive_right(float degrees);
void drive_left(float degrees);
void stop_motors();


// ---------------- SETUP ----------------
void setup() {

  // LEFT MOTOR (PB0, PB1)
  DDRB |= (1 << 0) | (1 << 1);

  // RIGHT MOTOR (PD2, PD3)
  DDRD |= (1 << 2) | (1 << 3);

  // BUTTON PB2
  DDRB &= ~(1 << 2);
  PORTB |= (1 << 2);   // pull-up

  stop_motors();
}


// ---------------- MAIN LOOP ----------------
void loop() {

  static uint8_t prev = 1;

  uint8_t curr = (PINB & (1 << 2)) ? 1 : 0;

  // Detect button press (falling edge)
  if (prev == 1 && curr == 0) {

    drive_forward(0.3048);   // 1 ft
    drive_right(90);

    drive_forward(0.3048);
    drive_left(90);

    drive_forward(0.1524);   // 0.5 ft
    drive_backward(0.4572);  // 1.5 ft

    drive_left(90);
    drive_forward(0.3048);

    stop_motors();
  }

  prev = curr;
}


// ---------------- MOVEMENT FUNCTIONS ----------------
void drive_forward(float distance_m){

  // LEFT forward
  PORTB = (PORTB & 0b11111100) | 0b00000010;

  // RIGHT forward
  PORTD = (PORTD & 0b11110011) | 0b00001000;

  float t = time_delay_linear(distance_m);
  delay((unsigned long)t);
}

void drive_backward(float distance_m){

  // LEFT reverse
  PORTB = (PORTB & 0b11111100) | 0b00000001;

  // RIGHT reverse
  PORTD = (PORTD & 0b11110011) | 0b00000100;

  float t = time_delay_linear(distance_m);
  delay((unsigned long)t);
}

void drive_right(float degrees){

  // LEFT forward
  PORTB = (PORTB & 0b11111100) | 0b00000010;

  // RIGHT reverse
  PORTD = (PORTD & 0b11110011) | 0b00000100;

  float t = time_delay_turn(degrees);
  delay((unsigned long)t);
}

void drive_left(float degrees){

  // LEFT reverse
  PORTB = (PORTB & 0b11111100) | 0b00000001;

  // RIGHT forward
  PORTD = (PORTD & 0b11110011) | 0b00001000;

  float t = time_delay_turn(degrees);
  delay((unsigned long)t);
}

void stop_motors(){

  PORTB &= ~((1 << 0) | (1 << 1));
  PORTD &= ~((1 << 2) | (1 << 3));
}


// ---------------- TIME CALCULATIONS ----------------
float time_delay_linear(float distance_m){

  float wheel_circ = PI * wheel_diam;     // meters
  float v = wheel_circ * (rpm / 60.0);    // m/s
  float t = distance_m / v;               // seconds

  return t * 1000.0;                      // ms
}

float time_delay_turn(float angle_deg){

  float angle_rad = angle_deg * PI / 180.0;

  float wheel_circ = PI * wheel_diam;
  float v = wheel_circ * (rpm / 60.0);    // m/s

  float omega = (2.0 * v) / shaft_distance;  // rad/s

  float t = angle_rad / omega;            // seconds

  return t * 1000.0;                      // ms
}