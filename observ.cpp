#include "observable.hpp"
#include <memory>
#include <functional>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>

using namespace observer;

struct meta{};
typedef observable<void(std::string const&), meta>
meta_observable;

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
    cout<<"simple_observer: "<<obj_name<<":"<<state<<"\n";
  }

  std::string state;
};

void free_function(std::string const &obj_name)
{
  std::cout<<"free_function: "<<obj_name<<"\n";
}

struct mySubject
: make_observable< 
    vector<
      meta_observable, 
      input_observable
    > 
  >::base
{
  virtual ~mySubject(){ }  
};

struct derivedSubject
: mySubject
{
  ~derivedSubject()
  { printf("derived deleted\n"); }
};

int main()
{
  using namespace std;
  using namespace std::placeholders; 
  handle_t handle, handle2;

  simple_observer so;
  mySubject s;
  mySubject *d = new derivedSubject;
  
  so.state = "obs_outter";

  handle = s.meta_observable::attach_mem_fn(&simple_observer::on_meta, &so, _1);
  s.meta_observable::attach(&free_function, _1);
  d->meta_observable::attach_mem_fn(&simple_observer::on_meta, &so, _1);

  {
    shared_ptr<simple_observer> sp(new simple_observer);
    sp->state = "obs_inner";
    handle2 = s.meta_observable::attach_mem_fn(&simple_observer::on_meta, sp, _1);
    cout<<"Num of observers attached to mySubject: "<<
      s.meta_observable::get_observers().size()<<"\n";
  }

  s.meta_observable::notify("test");
  d->meta_observable::notify("yoyo");
  delete d;
  
  cout<<"Num of observers attached to mySubject: "<<
    s.meta_observable::get_observers().size()<<"\n";
  
  cout<<"Do detach.\n";
  s.meta_observable::detach(handle);
  s.meta_observable::detach(handle2);

  cout<<"Num of observers attached to mySubject: "<<
    s.meta_observable::get_observers().size()<<"\n";

  return 0;
}
