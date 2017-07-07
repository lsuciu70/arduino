//#include <Scheduler.h>

void scheduledLoop();

void setup()
{
   Serial.begin(112500);
}

void loop()
{
  scheduledLoop()

  if(millis() % 2000)
    Serial.println(String("loop: ") + millis());
  delay(100);
}

void scheduledLoop()
{
  if(millis() % 1000)
    Serial.println(String("scheduledLoop: ") + millis());
  yield();
}

