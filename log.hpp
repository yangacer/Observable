#ifndef OBSERVER_LOG_HPP_
#define OBSERVER_LOG_HPP_

#include <string>
#include <typeinfo>
#include <ostream>
#include <chrono>
#include "compat.hpp"

struct logger
{
  logger();
  void set(std::ostream &os);
  std::ostream &get();
  void start();
  void add_timestamp();

  static logger &singleton();
private:
  std::ostream *os_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};


#ifdef OBSERVER_ENABLE_TRACKING

#define OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_ \
  virtual std::string get_notifer_info_() const \
  { \
    return DEMANGLE_BY_TYPE(decltype(*this)); \
  }

#define OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN_ \
    logger::singleton().get() << "@obs:" << '"' << get_notifer_info_() << "\" -- \"" ; \

#define OBSERVER_TRACKING_SUBJECT_INVOKE_END_ \
    logger::singleton().get() << " [label=\"";  \
    logger::singleton().add_timestamp(); \
    logger::singleton().get() << "\"];\n"; 

#define OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED \
  { \
    logger::singleton().get() << DEMANGLE_BY_TYPE(decltype(*this)) << "::" <<\
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
