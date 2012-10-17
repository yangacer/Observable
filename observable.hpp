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

namespace detail {

template<
  typename FuncPtr, bool IsMemFuncPtr, 
  typename InstOrArg>
struct get_handle;

} // namespace detail

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

  /** Attach observer to an observable object. Specialize
   * for no parameter free function.
   * @param fptr Function pointer.
   * @return Handle for detaching a registered observer.
   */
  template<typename FuncPtr>
  handle_t attach(FuncPtr fptr)
  {
   
    detail::get_handle<
      FuncPtr, 
      std::is_member_function_pointer<FuncPtr>::value,
      void*
        > get_handle;

    handle_t addr = get_handle(fptr, 0);

    obs_.emplace_back(
        addr,
        std::bind(fptr)
      );

    return addr;
  }

 /** Attach observer to an observable object. Support function and 
  *  functor which consist of at least one parameter.
   * @param fptr Function pointer.
   * @param inst Instance or argument.
   * @param prototype Argument list.
   * @return Handle for detaching a registered observer.
   */
  template<typename FuncPtr, typename Inst, typename ...Proto>
  handle_t attach(FuncPtr fptr, Inst&& inst, Proto&&... proto)
  {
    detail::get_handle<
      FuncPtr, 
      std::is_member_function_pointer<FuncPtr>::value,
      Inst
        > get_handle;

    handle_t handle = get_handle(
      fptr, std::forward<Inst>(inst));

    obs_.emplace_back(
      handle,
      std::bind(
        fptr, 
        std::forward<Inst>(inst), 
        std::forward<Proto>(proto)...
        )
      );
    return handle;
  }

  template<typename FuncPtr>
  void detach(FuncPtr fptr)
  {
    detail::get_handle<
      FuncPtr, 
      std::is_member_function_pointer<FuncPtr>::value,
      void*
        > get_handle;

    detach(get_handle(fptr, 0));
  }

  template<typename FuncPtr, typename Inst>
  void detach(FuncPtr fptr, Inst &&inst)
  {
    detail::get_handle<
      FuncPtr, 
      std::is_member_function_pointer<FuncPtr>::value,
      Inst
        > get_handle;

    detach(get_handle(fptr, std::forward<Inst>(inst)));
  }

  void detach_all()
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

namespace detail {
 
template<typename FuncPtr, typename InstOrArg>
struct get_handle<FuncPtr, true, InstOrArg>
{
  handle_t operator()(FuncPtr fptr, InstOrArg&& inst)
  {
#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
    uint8_t const ptr_size(sizeof(FuncPtr)+sizeof(void*));
    char ptr_val[ptr_size+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(FuncPtr));
    std::memcpy(ptr_val + sizeof(FuncPtr), &*inst, sizeof(void*));
    handle_t addr(ptr_val, ptr_size);
    
#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic pop
#endif
    return addr;
  }
};

template<typename FuncPtr, typename InstOrArg>
struct get_handle<FuncPtr, false, InstOrArg>
{
  handle_t operator()(FuncPtr fptr, InstOrArg&& inst)
  {
#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
    char ptr_val[sizeof(FuncPtr)+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(FuncPtr));
    handle_t addr(ptr_val, sizeof(FuncPtr));
#ifdef OBSERVER_IS_GNUC_
#pragma GCC diagnostic pop
#endif
    return addr;
  }
};


} // namespace detail

} // namespace observer

#endif // header guard
