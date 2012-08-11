#ifndef OBSERVABLE_HPP_
#define OBSERVABLE_HPP_

#include <memory>
#include <cstring>
#include <functional>
#include <type_traits>
#include <string>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/vector.hpp>
#include <vector>
#include <exception>
#include <memory>

#if defined(__GNUC__) && !defined(__clang__)
#define OBSERVER_IS_GNUC_
#else
#undef OBSERVER_IS_GNUC_
#endif 

#ifdef OBSERVER_ENABLE_TRACKING_
#include <iostream>
#include <cstdio>

#ifdef OBSERVER_IS_GNUC_
#include <cxxabi.h>

template<typename T>
struct getname
{
  std::string operator()()
  { 
    int status(-4);
    std::shared_ptr<char> res(abi::__cxa_demangle(typeid(T).name(), 0, 0, &status));
    return (res) ? res.get() : typeid(T).name();
  }
};

#define DEMANGLE(Type) getname<Type>()() 

#else // OBSERVER_IS_GUNC_
// TODO demangle in other compilers?
#define DEMANGLE(Type) typeid(Type).name()
#endif 

#endif // OBSERVER_ENABLE_TRACKING_


namespace observer {
namespace detail {
  void print_every_4_bits(std::string const& encoded);
} // namespace detail
typedef std::string handle_t;

/** observable base class
 * @tparam CbFunc function signature.
 * @tparam Tag Useful when interfaces have distinct semantics
 * but function signatures are common.
 */
template<typename CbFunc, typename Tag = void>
struct observable 
{
  typedef CbFunc callback_signature;
  typedef std::function<CbFunc> callback_class;
  typedef std::vector<std::tuple<handle_t, callback_class> > collection_type;
  
  /** Attach observer to an observable object.
   * @param fptr Function pointer or member function pointer.
   * @param prototype Function prototype.
   * @return Handle for detaching a registered observer.
   * @remark If the fptr is a member function pointer, the second parameter 
   * will be a pointer to an object instance.
   */

  template<typename FuncPtr, typename ...Proto>
  handle_t attach(FuncPtr fptr, Proto&&... proto)
  {
    static_assert( 
      !std::is_member_function_pointer<FuncPtr>::value,
      "Use attach_mem_fn() to attach member functions");

#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
    char ptr_val[sizeof(FuncPtr)+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(FuncPtr));
    handle_t addr(ptr_val, sizeof(FuncPtr));

    obs_.emplace_back(
        addr,
        std::bind(fptr, std::forward<Proto>(proto)...)
      );

#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic pop
#endif

#ifdef OBSERVER_ENABLE_TRACKING_
    std::cout << "@" << DEMANGLE(decltype(*this)) << 
      "[" << DEMANGLE(Tag) << "]: " ;
    //detail::print_every_4_bits(addr);
    std::cout << " " << DEMANGLE(FuncPtr) << "\n" ;
#endif

    return addr;
  }

  template<typename MemFuncPtr, typename Inst, typename ...Proto>
  handle_t attach_mem_fn(MemFuncPtr fptr, Inst&& inst, Proto&&... proto)
  {
    static_assert( 
      std::is_member_function_pointer<MemFuncPtr>::value,
      "Use attach() to attach free functions");

#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
    uint8_t const ptr_size(sizeof(MemFuncPtr)+sizeof(void*));
    char ptr_val[ptr_size+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(MemFuncPtr));
    std::memcpy(ptr_val + sizeof(MemFuncPtr), &*inst, sizeof(void*));
    handle_t addr(ptr_val, ptr_size);

    obs_.emplace_back(
        addr,
        std::bind(
          fptr, 
          std::forward<Inst>(inst), 
          std::forward<Proto>(proto)...
          )
      );
    
#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic pop
#endif

#ifdef OBSERVER_ENABLE_TRACKING_
    std::cout << "@" << DEMANGLE(decltype(*this)) << 
      "[" << DEMANGLE(Tag) << "]: " ;
    //detail::print_every_4_bits(addr);
    std::cout << " " << DEMANGLE(MemFuncPtr) << "\n" ;
#endif

    return addr;
  }
   
  /** Detach observer from an observable object.
   * @param hdl Handle was returned by attach(...).
   */
  void detach(handle_t const &hdl)
  {
    auto i = obs_.begin();
    while(i < obs_.end()){
      // remove all matched
      if(hdl == std::get<0>(*i))
        i = obs_.erase(i);
      ++i;
    }
  }

  collection_type const&
  get_observers() const
  { return obs_; }

#ifdef TRACE_NOTIFICATION_
  #define PRINT_NOTIFY \
  printf("%s: %s is notified\n", typeid(Tag).name(), typeid(CbFunc).name())
#else
  #define PRINT_NOTIFY 
#endif

  void notify() const
  {
    PRINT_NOTIFY; 
    for(auto iter = obs_.begin(); iter != obs_.end(); ++iter)
      std::get<1>(*iter)();
  }
  
  template<typename ...Args>
  void notify(Args&&... param) const
  {
    PRINT_NOTIFY; 
    for(auto iter = obs_.begin(); iter != obs_.end(); ++iter)
      std::get<1>(*iter)(std::forward<Args>(param)...);
  }

protected:
  ~observable()
  {
#ifdef TRACE_NOTIFICATION_
    printf("observer %s is terminated\n", typeid(Tag).name());
#endif
  }
   
  collection_type obs_;

};

namespace mpl = boost::mpl;
using mpl::vector;

template<typename F>
struct helper
: F 
{};

// Pass boost::mpl::vector<Func1, Func2, ...> to this
// struct and get desired observable base class via 'type'.
// e.g.
// struct mySubject
// : make_observable<
//    observer::vector<
//      meta_observable, input_observable
//    > 
//   >::base
// {};
template<typename FuncVector>
struct make_observable
{
  typedef typename mpl::inherit_linearly<
    FuncVector,
    mpl::inherit<mpl::_1, helper<mpl::_2> >
  >::type base;

  typedef FuncVector function_vector;
};

namespace detail {
  void print_every_4_bits(std::string const& encoded)
  {
    for(auto i = encoded.begin(); i != encoded.end(); ++i)
      printf("%02x", *i);
  }
} // namespace detail

} // namespace observer

#endif // header guard
