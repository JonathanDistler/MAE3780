//gives directions on how to turn, can implement the turn functions previously
void setup() {
  Serial.begin(9600);
  DDRC &= ~((1 << PC0) | (1 << PC1));
}

void loop() {
  while (1) {
    uint8_t pins = PINC & ((1 << PC0) | (1 << PC1));

    if (!(pins & (1 << PC0))) {
      Serial.println("turn right");   // A0 sees black
    }
    else if (!(pins & (1 << PC1))) {
      Serial.println("turn left");  // A1 sees black
    }
    else {
      Serial.println("white");       // both HIGH
    }
  }
}