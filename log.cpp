#include "log.hpp"
#include <iostream>
#include <iomanip>

logger::logger()
  : os_(&std::cout)
{
  start();
}

std::ostream&
logger::set(std::ostream &os)
{
  os_ = &os;
  return *os_;
}

std::ostream&
logger::get()
{ return *os_; }

std::ostream&
logger::start()
{
  start_ = std::chrono::high_resolution_clock::now();
  return *os_;
}

std::ostream&
logger::add_timestamp()
{
  using namespace std::chrono;
  auto dur = 
    (duration_cast<milliseconds>(system_clock::now() - start_).count());

  (*os_) << dur/1000 << "."  << std::setw(3) << std::setfill('0') << dur%1000 ;

  return *os_;
}

logger &
logger::singleton()
{
  static logger inst;
  return inst;
}
