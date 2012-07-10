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
#include <map>
#include <exception>

#ifdef TRACE_NOTIFICATION_
#include <cstdio>
#endif

namespace observer {
  typedef std::string handle_t;

  struct redundant_observer : std::exception {};
  
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
  typedef std::map<handle_t, callback_class> collection_type;
  
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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
    char ptr_val[sizeof(FuncPtr)+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(FuncPtr));
    handle_t addr(ptr_val, sizeof(FuncPtr));

    auto rt = obs_.insert(
      std::make_pair(
        addr,
        std::bind(fptr, std::forward<Proto>(proto)...)
        )
      );

    if(!rt.second)
      throw redundant_observer(); 

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    return addr;
  }

  template<typename MemFuncPtr, typename Inst, typename ...Proto>
  handle_t attach_mem_fn(MemFuncPtr fptr, Inst&& inst, Proto&&... proto)
  {
    static_assert( 
      std::is_member_function_pointer<MemFuncPtr>::value,
      "Use attach() to attach free functions");

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
    uint8_t const ptr_size(sizeof(MemFuncPtr)+sizeof(void*));
    char ptr_val[ptr_size+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(MemFuncPtr));
    std::memcpy(ptr_val + sizeof(MemFuncPtr), &*inst, sizeof(void*));
    handle_t addr(ptr_val, ptr_size);

    auto rt = obs_.insert(
      std::make_pair(
        addr,
        std::bind(
          fptr, 
          std::forward<Inst>(inst), 
          std::forward<Proto>(proto)...
          )
        )
      );
    
    if(!rt.second)
      throw redundant_observer(); 

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    return addr;
  }

  /** Detach observer from an observable object.
   * @param hdl Handle was returned by attach(...).
   */
  void detach(handle_t hdl)
  {  obs_.erase(hdl); }

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
      (iter->second)();
  }
  
  template<typename ...Args>
  void notify(Args&&... param) const
  {
    PRINT_NOTIFY; 
    for(auto iter = obs_.begin(); iter != obs_.end(); ++iter)
        (iter->second)(std::forward<Args>(param)...);
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

} // namespace observer

#endif // header guard
