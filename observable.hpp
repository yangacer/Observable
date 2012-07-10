#ifndef OBSERVABLE_HPP_
#define OBSERVABLE_HPP_

//#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
//#include <boost/function_equal.hpp>
#include <memory>
#include <cstring>
#include <functional>
#include <type_traits>
#include <string>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/vector.hpp>
//#include "bind_weak_ptr.hpp" /* boost ticket #810 */
#include <map>
#include <vector>

#ifdef TRACE_NOTIFICATION_
#include <cstdio>
#endif

namespace observer {
  typedef std::string address_t;

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
  typedef std::map<address_t, callback_class> collection_type;
  
  /** Attach observer to an observable object.
   * @param address Key to callback object that can be
   * address of member function's object instance or pointer
   * to a free function.
   * @param cb Callback object.
   * @remark The address is only for indexing storage of callback
   * objects.
   */
  //void attach(void *address, callback_class cb)
  //{ obs_.insert(std::make_pair(address, cb)); }

  template<typename FuncPtr, typename ...Proto>
  address_t attach(FuncPtr fptr, Proto&&... prototype)
  {
    char ptr_val[sizeof(FuncPtr)+1];
    std::memcpy(ptr_val, (void*)fptr, sizeof(FuncPtr));
    address_t addr(ptr_val, sizeof(FuncPtr));
    
    printf("%d\n", addr.size());

    obs_.insert(
      std::make_pair(
        addr,
        std::bind(fptr, std::forward<Proto>(prototype)...)
        )
      );
    return addr;
  }

  /** Detach observer from an observable object.
   * @param address Key to callback object that can be
   * address of member function's object instance or pointer
   * to a free function.
   */
  void detach(address_t address)
  {  obs_.erase(address); }

  collection_type const&
  get_observers() const
  { return obs_; }

#ifdef TRACE_NOTIFICATION_
  #define PRINT_NOTIFY \
  printf("%s: %s is notified\n", typeid(Tag).name(), typeid(CbFunc).name())
#else
  #define PRINT_NOTIFY 
#endif

  // XXX Limitation: const reference semantic can not be deduced
  // from template parameter without c++0x
  void notify() const
  {
    PRINT_NOTIFY; 
    for(auto Iter = obs_.begin(); Iter != obs_.end(); ++Iter)
      (Iter->second)();
  }
  
  template<typename ...Args>
  void notify(Args&&... param) const
  {
    PRINT_NOTIFY; 
    for(auto Iter = obs_.begin(); Iter != obs_.end(); ++Iter)
        (Iter->second)(std::forward<Args>(param)...);
  }

protected:
  ~observable(){}

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
