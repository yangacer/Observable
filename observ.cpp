#define OBSERVER_ENABLE_TRACKING
#include "observable.hpp"
#include <memory>
#include <functional>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>
#include <sstream>

using namespace observer;

struct meta{};
typedef observable<void(std::string const&), meta>
meta_observable;

typedef observable<void()> void_observable;

typedef observable<void(std::string const&)>
named_observable;

struct input{};
typedef observable<void(char const*, size_t), input>
input_observable;

struct simple_observer
: std::enable_shared_from_this<simple_observer>
{
  using std::enable_shared_from_this<simple_observer>::shared_from_this;

  simple_observer()
  : state("state")
  {}
  
  simple_observer(simple_observer &&tmp)
  : state(tmp.state)
  {
    state = "state";
  }

  ~simple_observer()
  { 
    std::cout<<"simple_observer("<<
      state<<
      ") is deleted\n"; 
  }

  void 
  on_input(char const* data, size_t byte_off)
  {}

  void 
  on_meta(std::string const &obj_name)
  {
    using namespace std;
    OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
    cout<<"simple_observer: "<<obj_name<<":"<<state<<"\n";
    sleep(1);
  }
  
  void
  on_named(std::string const &obj_name)
  {
    OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
    std::cout << obj_name << "\n";
  }

  void
  on_void()
  {
    OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  }

  std::string state;
};

void free_function(std::string const &obj_name)
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;

  std::cout<<"free_function: "<<obj_name<<"\n";
}

struct mySubject
: make_observable< 
    vector<
      meta_observable, 
      input_observable,
      named_observable,
      void_observable
    > 
  >::base
{
  void name_it()
  {
    named_observable::notify("Named!!");
  }

  virtual ~mySubject(){ }  

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_

};

struct derivedSubject
: mySubject
{
  ~derivedSubject()
  { printf("derived deleted\n"); }

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_
};

int main()
{
  using namespace std;
  using namespace std::placeholders; 

  std::stringstream log;
  logger::singleton().set(log);

  handle_t handle, handle2;

  simple_observer so;
  mySubject s;
  mySubject *d = new derivedSubject;
  
  so.state = "obs_outter";

  handle = s.meta_observable::attach_mem_fn(&simple_observer::on_meta, &so, _1);
  s.meta_observable::attach(&free_function, _1);
  s.void_observable::attach_mem_fn(&simple_observer::on_void, &so);
  d->meta_observable::attach_mem_fn(&simple_observer::on_meta, &so, _1);

  {
    shared_ptr<simple_observer> sp(new simple_observer);
    sp->state = "obs_inner";
    handle2 = s.meta_observable::attach_mem_fn(&simple_observer::on_meta, sp, _1);
    cout<<"Num of observers attached to mySubject: "<<
      s.meta_observable::get_observers().size()<<"\n";
  }

  s.meta_observable::notify("test");
  s.void_observable::notify();
  d->meta_observable::notify("yoyo");
  delete d;
  

  cout<<"Num of observers attached to mySubject: "<<
    s.meta_observable::get_observers().size()<<"\n";
  
  cout<<"Do detach (obs_inner, free_function).\n";
  s.meta_observable::detach(handle);
  s.meta_observable::detach(&free_function);
  
  s.named_observable::attach_mem_fn(&simple_observer::on_named, &so, _1);
  s.name_it();

  cout<<"Num of observers attached to mySubject: "<<
    s.meta_observable::get_observers().size()<<"\n";

  cout << "LOG -------\n";
  cout << log.str();

  return 0;
}
