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
  {}

  ~simple_observer()
  { std::cout<<"simple_observer is deleted\n"; }

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
    vector<meta_observable, input_observable> >::base
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
  handle_t handle;

  simple_observer so;
  mySubject s;
  mySubject *d = new derivedSubject;
  
  handle = s.meta_observable::attach(&simple_observer::on_meta, &so, _1);
  s.meta_observable::attach(&free_function, _1);
  d->meta_observable::attach(&simple_observer::on_meta, &so, _1);

  s.meta_observable::notify("test");
  d->meta_observable::notify("yoyo");
  delete d;
  cout<<"Num of observers attached to simple_observer: "<<
    s.meta_observable::get_observers().size()<<"\n";
  
  cout<<"Do detach.\n";
  s.meta_observable::detach(handle);

  cout<<"Num of observers attached to simple_observer: "<<
    s.meta_observable::get_observers().size()<<"\n";

  return 0;
}
