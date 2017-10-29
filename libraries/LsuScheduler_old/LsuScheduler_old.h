#ifndef LsuScheduler_h
#define LsuScheduler_h

#include <Arduino.h>

class LsuScheduler
{
private:
  struct node
  {
    long int when;
    void (*funt)(void);
    struct node * next;
  };
  node * head;
  void destroy(node *);
  void destroy_r(node *, node *);
public:
  LsuScheduler();
  virtual ~LsuScheduler();
  void execute(unsigned long);
  void add(void (*)(), unsigned long);
};

#endif // LsuScheduler_h
