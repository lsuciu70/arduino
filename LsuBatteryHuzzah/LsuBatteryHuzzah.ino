void setup() {
  Serial.begin(115200);
}

void loop() {
  if(millis() % 10000 == 0)
  {
	  int level = analogRead(A0);
	  Serial.println(A0);
	  Serial.print(level);
	  Serial.print(" -> ");
	  // divider: 1M / 10M -> 2 / (10 + 1) = 0.091
	  // high: 4.2 [V] * .091 = 0.382 [V]
	  // readout: 0.382 * 1023 = 390
	  // low:  3.14 [V] * .091 = 0.285 [V]
	  // readout: 0.285 * 1023 = 292
	  // convert battery level to percent
	  // maps
	  //   350 -> 100 %
	  //   260 -> 0 %
	  level = map(level, 261, 350, 0, 100);
	  Serial.print(level);
	  Serial.print(" (");
	  Serial.print(millis());
	  Serial.println(")");
	  delay(1000);
  }
}
