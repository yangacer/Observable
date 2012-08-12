#include "log.hpp"
#include <iostream>

logger::logger()
  : os_(&std::cout)
{
  start();
}

void
logger::set(std::ostream &os)
{
  os_ = &os;
}

std::ostream&
logger::get()
{ return *os_; }

void 
logger::start()
{
  start_ = std::chrono::high_resolution_clock::now();
}

void
logger::add_timestamp()
{
  using namespace std::chrono;

  (*os_) << 
    (duration_cast<milliseconds>(high_resolution_clock::now() - start_).count()) <<
    " ms";
}

logger &
logger::singleton()
{
  static logger inst;
  return inst;
}
