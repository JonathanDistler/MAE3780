void setup() {
  Serial.begin(9600);
  DDRC &= ~((1 << PC0) | (1 << PC1));
}

void loop() {
  while (1) {
    uint8_t pins = PINC & ((1 << PC0) | (1 << PC1));

    if (pins == ((1 << PC0) | (1 << PC1)))
      Serial.println("white");   // BOTH HIGH
    else
      Serial.println("black");   // ANY LOW
  }
}