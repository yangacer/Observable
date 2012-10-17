#include "observable.hpp"
#include <iostream>

#define NOTIFIED_ std::cout << __FUNCTION__ << ":" << __LINE__ << " is notified\n";

typedef observer::observable<void(int)> int_;
typedef observer::observable<void()> void_;

class subject
: public observer::make_observable<
  observer::vector< 
    int_, void_
  >
  >::base
{
public:
  void run()
  { 
    int_::notify(2); 
    void_::notify();
  }
};

struct observer_class 
{
  void int_callback(int i)
  { NOTIFIED_; }

  void void_callback()
  { NOTIFIED_; }
};

void int_callback(int i)
{ NOTIFIED_; }

void void_callback()
{ NOTIFIED_; }

int main()
{

  subject s;
  observer_class oc;
  s.int_::attach(&observer_class::int_callback, &oc, std::placeholders::_1);
  s.int_::attach(&int_callback, std::placeholders::_1);
  s.void_::attach(&observer_class::void_callback, &oc);
  s.void_::attach(&void_callback);
  s.run();

  return 0;
}
