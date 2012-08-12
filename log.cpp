#include "log.hpp"
#include <iostream>

logger::logger()
  : os_(&std::cout)
{}

void
logger::set(std::ostream &os)
{
  os_ = &os;
}

std::ostream&
logger::get()
{ return *os_; }

logger &
logger::singleton()
{
  static logger inst;
  return inst;
}
