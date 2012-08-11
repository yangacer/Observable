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
// TODO replace cout with a static ostream reference
#define OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN_ \
  { \
    std::cerr << "@log:" << '"' << get_notifer_info_() << "\" -> \"" ; \
  }

#define OBSERVER_TRACKING_SUBJECT_INVOKE_END_ \
  { \
    std::cerr << " [label=\"" << \
    DEMANGLE_BY_TYPE(decltype(*this)) << "\"];\n"; \
  }

#define OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED \
  { \
    std::cerr << DEMANGLE_BY_TYPE(decltype(*this)) << "::" <<\
    __FUNCTION__ << '"'; \
  }

#define OBSERVER_TRACKING_OBSERVER_FN_INVOKED \
  { \
    std::cerr <<  __FUNCTION__ << '"'; \
  }

#else // OBSERVER_ENABLE_TRACKING

#define OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_ 
#define OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN 
#define OBSERVER_TRACKING_SUBJECT_INVOKE_END_
#define OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED
#define OBSERVER_TRACKING_OBSERVER_FN_INVOKED 

#endif // OBSERVER_ENABLE_TRACKING

#endif
