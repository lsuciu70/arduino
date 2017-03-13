void setup() {
  Serial.begin(115200);
  float x = 1.0f;
  Serial.print(F("println unformatted version: x = "));
  Serial.println(x);
  Serial.println("------------------------");
  const uint8_t X_CHAR_LEN = 5;
  char x_char[X_CHAR_LEN] = {'\0'};
  sprintf(x_char, "%.2f", x);
  Serial.print("sprintf version: x = ");
  Serial.println(x_char);
  Serial.print("sprintf ascii chars of: x = {");
  for(uint8_t i = 0; i < X_CHAR_LEN; ++i)
  {
    if(i)
      Serial.print(F(", "));
    Serial.print((int)x_char[i]);
  }
  Serial.println("}");

}

void loop() {
}
