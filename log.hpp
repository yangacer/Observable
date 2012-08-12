#ifndef OBSERVER_LOG_HPP_
#define OBSERVER_LOG_HPP_

#ifdef OBSERVER_ENABLE_TRACKING

#include <string>
#include <typeinfo>
#include <ostream>
#include <chrono>
#include <iomanip>
#include "compat.hpp"

struct logger
{
  logger()
  {
    start();
  }

  std::ostream &set(std::ostream &os)
  {
    os_ = &os; 
    return *os_;
  }

  std::ostream &get()
  {
    return *os_;
  }

  std::ostream &start()
  {
    start_ = std::chrono::high_resolution_clock::now();
    return *os_;
  }

  std::ostream &add_timestamp()
  {
    using namespace std::chrono;
    auto dur = 
      (duration_cast<milliseconds>(system_clock::now() - start_).count());

    (*os_) << dur/1000 << "."  << std::setw(3) << std::setfill('0') << dur%1000 ;

    return *os_;

  }

  static logger &singleton()
  {
    static logger inst;
    return inst;
  }
private:
  std::ostream *os_;
  std::chrono::time_point<std::chrono::system_clock> start_;
};


#define OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_ \
  virtual std::string get_notifer_info_() const \
  { \
    return DEMANGLE_BY_TYPE(decltype(*this)); \
  }

#define OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN_ \
    logger::singleton().get() << "@obs: ["; \
    logger::singleton().add_timestamp() << "] " << \
      '"' << get_notifer_info_() << "\" -- " ;

#define OBSERVER_TRACKING_SUBJECT_INVOKE_END_ \
    logger::singleton().get() << " [" ; \
    logger::singleton().add_timestamp() << \
      "];\n"; 

#define OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED \
  { \
    logger::singleton().get() << '"' << DEMANGLE_BY_TYPE(decltype(*this)) << "::" <<\
    __FUNCTION__ << '"'; \
  }

#define OBSERVER_TRACKING_OBSERVER_FN_INVOKED \
  { \
    logger::singleton().get() <<  __FUNCTION__ << '"'; \
  }

#else // OBSERVER_ENABLE_TRACKING

#define OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_ 
#define OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN_
#define OBSERVER_TRACKING_SUBJECT_INVOKE_END_
#define OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED
#define OBSERVER_TRACKING_OBSERVER_FN_INVOKED 

#endif // OBSERVER_ENABLE_TRACKING

#endif
