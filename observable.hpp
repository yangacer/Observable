#ifndef OBSERVABLE_HPP_
#define OBSERVABLE_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/function_equal.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/vector.hpp>
#include "bind_weak_ptr.hpp" /* boost ticket #810 */
#include <map>
#include <vector>

#ifdef TRACE_NOTIFICATION_
#include <cstdio>
#endif

namespace observer {

/** observable base class
 * @tparam CbFunc function signature.
 * @tparam Tag Useful when interfaces have distinct semantics
 * but function signatures are common.
 */
template<typename CbFunc, typename Tag = void>
struct observable 
{
  typedef CbFunc callback_signature;
  typedef boost::function<CbFunc> callback_class;
  typedef std::map<void*, callback_class> collection_type;
  typedef std::vector<typename collection_type::key_type> garbage_collection_t; 

  /** Attach observer to an observable object.
   * @param address Key to callback object that can be
   * address of member function's object instance or pointer
   * to a free function.
   * @param cb Callback object.
   * @remark The address is only for indexing storage of callback
   * objects.
   */
  void attach(void *address, callback_class cb)
  { obs_.insert(std::make_pair(address, cb)); }

  /** Detach observer from an observable object.
   * @param address Key to callback object that can be
   * address of member function's object instance or pointer
   * to a free function.
   */
  void detach(void *address)
  {  obs_.erase(address); }

  /** Wipe(detach) dead observers.
   * @detail This method only detaches dead observers discovered
   * during notify(). If dead observers were never notified, then
   * this method is unable to detach them.
   */
  void wipe_dead_observers()
  {
    for(typename garbage_collection_t::iterator i = trash_.begin();
        i != trash_.end(); ++i)
      obs_.erase(*i);
    trash_.clear();
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

#define NOTIFY_IMPL_BEGIN \
  typedef typename collection_type::const_iterator iter_t; \
  PRINT_NOTIFY; \
  for(iter_t Iter = obs_.begin(); Iter != obs_.end(); ++Iter){ \
    try{ \
      (Iter->second) /* place argument list here */
#define NOTIFY_IMPL_END \
    }catch(boost::disposed_exception const &e){ \
      trash_.push_back(Iter->first); \
      continue; \
    }\
  }

  // XXX Limitation: const reference semantic can not be deduced
  // from template parameter without c++0x
  void notify() const
  {
    NOTIFY_IMPL_BEGIN();
    NOTIFY_IMPL_END;
  }

  template<typename A>
  void notify(A a) const
  { 
    NOTIFY_IMPL_BEGIN(a);
    NOTIFY_IMPL_END;
  }

  template<typename A1, typename A2>
  void notify(A1 a1, A2 a2) const
  {
    NOTIFY_IMPL_BEGIN(a1,a2);
    NOTIFY_IMPL_END;
  }

  template<typename A1, typename A2, typename A3>
  void notify(A1 a1, A2 a2, A3 a3) const
  {
    NOTIFY_IMPL_BEGIN(a1,a2,a3);
    NOTIFY_IMPL_END;
  }
  
  template<typename A1, typename A2, typename A3, typename A4>
  void notify(A1 a1, A2 a2, A3 a3, A4 a4) const
  {
    NOTIFY_IMPL_BEGIN(a1,a2,a3,a4);
    NOTIFY_IMPL_END;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  void notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
  {
    NOTIFY_IMPL_BEGIN(a1,a2,a3,a4,a5);
    NOTIFY_IMPL_END;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
  {
    NOTIFY_IMPL_BEGIN(a1,a2,a3,a4,a5,a6);
    NOTIFY_IMPL_END;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  void notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
  {
    NOTIFY_IMPL_BEGIN(a1,a2,a3,a4,a5,a6,a7);
    NOTIFY_IMPL_END;
  }
  
  // Unsafe notify(): throw boost::dispose_exception
#define UNSAFE_NOTIFY_IMPL \
  typedef typename collection_type::const_iterator iter_t; \
  for(iter_t Iter = obs_.begin(); Iter != obs_.end(); ++Iter) \
    (Iter->second)
  
  void unsafe_notify() const
  {
    UNSAFE_NOTIFY_IMPL();
  }

  template<typename A>
  void unsafe_notify(A a) const
  { 
    UNSAFE_NOTIFY_IMPL(a);
  }

  template<typename A1, typename A2>
  void unsafe_notify(A1 a1, A2 a2) const
  {
    UNSAFE_NOTIFY_IMPL(a1,a2);
  }

  template<typename A1, typename A2, typename A3>
  void unsafe_notify(A1 a1, A2 a2, A3 a3) const
  {
    UNSAFE_NOTIFY_IMPL(a1,a2,a3);
  }
  
  template<typename A1, typename A2, typename A3, typename A4>
  void unsafe_notify(A1 a1, A2 a2, A3 a3, A4 a4) const
  {
    UNSAFE_NOTIFY_IMPL(a1,a2,a3,a4);
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  void unsafe_notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
  {
    UNSAFE_NOTIFY_IMPL(a1,a2,a3,a4,a5);
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void unsafe_notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
  {
    UNSAFE_NOTIFY_IMPL(a1,a2,a3,a4,a5,a6);
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  void unsafe_notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
  {
    UNSAFE_NOTIFY_IMPL(a1,a2,a3,a4,a5,a6,a7);
  }

protected:
  collection_type obs_;

private:
  mutable garbage_collection_t trash_;

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
