#ifndef OBSERVER_COMPAT_HPP_
#define OBSERVER_COMPAT_HPP_

// Compiler detection
#if defined(__GNUC__) && !defined(__clang__)
#define OBSERVER_IS_GNUC_
#else
#undef OBSERVER_IS_GNUC_
#endif 

// GNUC specific
#ifdef OBSERVER_IS_GNUC_
#include <cxxabi.h>

std::string translate_name(char const* name)
{
  int status(-4);
  std::shared_ptr<char> res(abi::__cxa_demangle(name, 0, 0, &status));
  return (res) ? res.get() : name;
}

template<typename T>
struct get_name
{
  std::string operator()()
  { 
    int status(-4);
    std::shared_ptr<char> res(abi::__cxa_demangle(typeid(T).name(), 0, 0, &status));
    return (res) ? res.get() : typeid(T).name();
  }
};

#define DEMANGLE_BY_TYPE(Type) get_name<Type>()() 
#define DEMANGLE_BY_NAME(Name) translate_name(Name)
#else // OBSERVER_IS_GUNC_
// TODO demangle in other compilers?
#define DEMANGLE_BY_TYPE(Type) typeid(Type).name()
#define DEMANGLE_BY_NAME(Name)
#endif 


#endif // header guard
