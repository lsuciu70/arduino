const uint8_t PINS = 8;
const uint8_t pins[PINS] =  
{
	D1, D2, D3, D4, D5, D6, D7, D8/*, D9, D10*/
};

volatile uint16_t value;

void readValue()
{
  for (uint8_t i = 0; i < PINS; ++i)
  {
    if (HIGH == digitalRead(pins[i]))
      bitSet(value, i);
    else
      bitClear(value, i);
  }
}
void printValue()
{
  Serial.print("value: ");
  Serial.println((value | 0x100), BIN);
}

void handleInterrupt()
{
  readValue();
  printValue();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Started");
}

bool initialized = false;

void lateInit()
{
  if(initialized)
    return;
  initialized = true;
  pinMode(D9, OUTPUT);
  digitalWrite(D9, HIGH);
  for (uint8_t i = 0; i < PINS; ++i)
  {
    pinMode(pins[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(pins[i]), handleInterrupt, CHANGE);
  }
//  pinMode(D10, OUTPUT);
  Serial.println("Initialized");
  readValue();
  printValue();
}

void loop()
{
  lateInit();
}
