unsigned long tm, last_tm;

void setup()
{
  Serial.begin(9600);
  last_tm = 0;
}

void loop()
{
  if(((tm = millis()) % 1000) == 0 && tm != last_tm)
    Serial.println((last_tm = tm) / 1000);
}

