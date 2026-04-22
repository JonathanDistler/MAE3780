//gives directions on how to turn, can implement the turn functions previously
//NOTE: need to change to binary pin numbers and not PC0 and PC1, will get points off
void setup() {
  Serial.begin(9600);
  DDRC &= ~((1 << PC0) | (1 << PC1));
}

void loop() {
  while (1) {
    uint8_t pins = PINC & ((1 << PC0) | (1 << PC1));

    //logic to find which side of the car the black color is on.
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
