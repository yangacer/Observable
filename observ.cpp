#include "observable.hpp"
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
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
: boost::enable_shared_from_this<simple_observer>
{
  using boost::enable_shared_from_this<simple_observer>::shared_from_this;

  simple_observer()
  : state("state")
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
  using namespace boost;
  using namespace std;

  simple_observer so;
  mySubject s;
  mySubject *d = new derivedSubject;
  
  s.meta_observable::attach(&so, bind(&simple_observer::on_meta, &so, _1));
  s.meta_observable::attach((void*)&free_function, bind(&free_function,_1));
  d->meta_observable::attach(&so, bind(&simple_observer::on_meta, &so, _1));
  {
    shared_ptr<simple_observer> so1(new simple_observer);
    s.meta_observable::attach(&*so1, bind(&simple_observer::on_meta, weak_ptr<simple_observer>(so1), _1));
    // At this movement, we are destructing so1 without detaching it, 
    // it is safe if we pass shared_ptr to the bind(). Though, this 
    // casues lifetime of observers to be extended. Since I do not 
    // expect the observer keep alive after I disposed it. As you can
    // see, I used weak_ptr to pass the pointer. The obsevable 
    // template class can internally prevent to call dead observers.
  }

  // Other than notify(), one can use unsafe_notify() that ignores
  // detection of dead observers if one expects all observers are
  // alive.
  s.meta_observable::notify("test");
  d->meta_observable::notify("yoyo");
  delete d;
  cout<<"attached observers: "<<
    s.meta_observable::get_observers().size()<<"\n";
  
  // Although the observable class can notify alive observers only,
  // dead observers still exist in a observable subject. One 
  // can manually wipe(detach) these zombies.
  // XXX Actually, we can have a safe version of notify() which
  // collects and wipe dead observers automatically. However, this 
  // burden the notify() performance.
  s.meta_observable::wipe_dead_observers();

  cout<<"attached observers after wiped: "<<
    s.meta_observable::get_observers().size()<<"\n";

  return 0;
}
