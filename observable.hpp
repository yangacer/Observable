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

#include "compat.hpp"
#include "log.hpp"


namespace observer {

typedef std::string handle_t;

/** observable base class
 * @tparam CbFunc function signature.
 * @tparam Tag Useful when interfaces have distinct semantics of a 
 * common function signature.
 */
template<typename CbFunc, typename Tag = void>
struct observable 
{
  typedef CbFunc callback_signature;
  typedef std::function<CbFunc> callback_class;

#ifdef MAKE_DOC
  typedef implementation_defined collection_type;
#else
  typedef std::vector<std::tuple<handle_t, callback_class> > collection_type;
#endif // MAKE_DOC

  /** Attach observer to an observable object.
   * @param fptr Function pointer.
   * @param prototype Argument list.
   * @return Handle for detaching a registered observer.
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

    return addr;
  }

  /** Attach observer to an observable object.
   * @param fptr Member function pointer.
   * @param inst Object instance
   * @param prototype Argument list.
   * @return Handle for detaching a registered observer.
   */
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

    return addr;
  }

  void detach()
  {
    obs_.clear();
  }
   
  /** Detach observer.
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
  
  /** Detach the front observer. */
  void detach_front()
  {
    if(!obs_.empty())
      obs_.erase(obs_.begin());
  }

  /** Detach the first observer matches a given handl.
   *  @param hdl Handle of observer.
   */
  void detach_first(handle_t const &hdl)
  {
    for(auto i = obs_.begin(); i != obs_.end(); ++i){
      if(hdl == std::get<0>(*i)){
        obs_.erase(i);
        break;
      }
    }
  }

  /** Get container of observers.
   *  @return collection_type.
   */
  collection_type const&
  get_observers() const
  { return obs_; }

  // For logging
  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_
  
  template<typename ...Args>
  void notify(Args&&... param) const
  {
    for(auto iter = obs_.begin(); iter != obs_.end(); ++iter){
      OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN_;
      std::get<1>(*iter)(std::forward<Args>(param)...);
      OBSERVER_TRACKING_SUBJECT_INVOKE_END_;
    }
  }
  
  template<typename ...Args>
  void notify_one(Args&&...param) const
  {
    if(!obs_.empty()){
      OBSERVER_TRACKING_SUBJECT_INVOKE_BEGIN_;
      std::get<1>(obs_.front())(std::forward<Args>(param)...);
      OBSERVER_TRACKING_SUBJECT_INVOKE_END_;
    }
  }

protected:
  ~observable()
  {}
  
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
