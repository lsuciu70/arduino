/* termometru */
#include <OneWire.h>
#include <DallasTemperature.h>

// one second, 1000 milliseconds
#define SECOND 1000

// Arduino pin 7
#define ONE_WIRE_BUS 7

enum
{
  RES_09_BIT = 9,
  RES_10_BIT = 10,
  RES_11_BIT = 11,
  RES_12_BIT = 12
};

const int RES = RES_11_BIT;

int timeToWaitForConversion(int res)
{
  switch (res)
  {
    case RES_09_BIT: return 94;
    case RES_10_BIT: return 188;
    case RES_11_BIT: return 375;
  }
  return 750;
}

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensor(&oneWire);

// Temperature senzor unique I2C address.
const uint8_t SENZOR [] = {0x28, 0xFF, 0x21, 0x14, 0xA6, 0x15, 0x3, 0x9D};

float temperature;

void printTempC_Serial();

void everyOneSecond(void);

void everyFiveSeconds(void);

void readTempC(void);

class exec_list
{
  private:
    struct node
    {
      long int when;
      void (*funt)(void);
      struct node * next;
    };
    node * head;
    void destroy(node *c)
    {
      if (c)
        destroy_r(c, c->next);
    }
    void destroy_r(node *c, node *n)
    {
      if (c)
        delete c;
      if (n)
        destroy_r(n, n->next);
    }
  public:
    exec_list()
    {
      head = new node();
      head->when = 0;
      head->funt = 0;
      head->next = 0;
    }

    virtual ~exec_list()
    {
      destroy(head);
    }

    void execute(long int current_time)
    {
      node * c = head;
      while (c->next)
      {
        node * n = c->next;
        if (n->when <= current_time)
        {
          // execute
          if (n->funt)
            n->funt();
          // remove
          c->next = n->next;
          delete n;
          n = 0;
        }
        // advance
        c = c->next;
      }
    }

    void add(void (*funt)(), long int when)
    {
      node * n = new node();
      n->when = when;
      n->funt = funt;
      n->next = 0;
      // start at head
      node * last = head;
      // search the last one
      while (last->next)
        last = last->next;
      // link after last
      last->next = n;
    }
};

exec_list scheduler;

// the setup routine runs once when starts
void setup()
{
  Serial.begin(9600);

  // Start up the library
  tempSensor.begin();
  tempSensor.setResolution(RES);
  tempSensor.setWaitForConversion(false);

  // trigger one first reading
  tempSensor.requestTemperatures();
  delay(timeToWaitForConversion(RES) + 5);
  temperature = 0;

  scheduler.add(everyFiveSeconds, 0);
  scheduler.add(everyOneSecond, 0);
  scheduler.add(readTempC, 0);
}

// the loop routine runs over and over again forever
void loop()
{
  scheduler.execute(millis());
}

// prints the temperature on serial
void printTempC_Serial()
{
  // Send the command to get temperatures
//  tempSensor.requestTemperatures();
//  Serial.println(tempSensor.getTempC(SENZOR));
  Serial.println(temperature);
}

void everyOneSecond(void)
{
  long int m = millis();
  Serial.println(m / SECOND);
  scheduler.add(everyOneSecond, m - (m % SECOND) + SECOND);
}

void everyFiveSeconds(void)
{
  long int m = millis();
  printTempC_Serial();
  scheduler.add(everyFiveSeconds, m - (m % SECOND) + 5 * SECOND);
}

void startConversion()
{
  tempSensor.requestTemperatures();
}

void readTempC(void)
{
  long int m = millis();
  m = m - (m % SECOND);
  temperature = tempSensor.getTempC(SENZOR);
  int next_read = m + (next_read * SECOND); // 2 seconds
  // schedule next read ater 2 seconds
  // - schedule conversion after 2 seconds minus the time to wait for it
  scheduler.add(startConversion, next_read - (timeToWaitForConversion(RES) + 5));
  // schedule read after 2 seconds
  scheduler.add(readTempC, next_read);
}

