#include "Arduino.h"
#include "LsuScheduler.h"

void LsuScheduler::destroy(node *c)
{
  if (c)
    destroy_r(c, c->next);
}

void LsuScheduler::destroy_r(node *c, node *n)
{
  if (c)
    delete c;
  if (n)
    destroy_r(n, n->next);
}

LsuScheduler::LsuScheduler()
{
  head = new node();
  head->when = 0;
  head->funt = 0;
  head->next = 0;
}

LsuScheduler::~LsuScheduler()
{ 
  destroy(head);
}

void LsuScheduler::execute(long int current_time)
{
  node * c = head;
  while (c->next)
  {
    node * n = c->next;
    if (n->when - current_time <= 0)
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

void LsuScheduler::add(void (*funt)(), long int when)
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

