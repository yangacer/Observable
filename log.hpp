#ifndef OBSERVER_LOG_HPP_
#define OBSERVER_LOG_HPP_

#include <string>
#include <typeinfo>
#include <iostream>
#include "compat.hpp"

#ifdef OBSERVER_ENABLE_TRACKING

#define OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_ \
  virtual std::string get_notifer_info_() const \
  { \
    return DEMANGLE_BY_TYPE(decltype(*this)); \
  }

#define OBSERVER_TRACKING_INVOKE_(FnName) \
  { \
    std::cout << "@log:" << '"' << get_notifer_info_() << "\" -> \"" << \
      DEMANGLE_BY_NAME(FnName) << "\"\n"; \
  }
  
#else // OBSERVER_ENABLE_TRACKING

#define OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_ 

#define OBSERVER_TRACKING_INVOKE_(Observer) {}

#endif // OBSERVER_ENABLE_TRACKING

#endif
