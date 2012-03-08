#include "observable.hpp"
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>

using namespace observer;

struct meta;
typedef observable<void(std::string const&), meta>
meta_observable;

struct input;
typedef observable<void(char const*, size_t), input>
input_observable;

struct simple_observer
: boost::enable_shared_from_this<simple_observer>
{
  simple_observer()
  : state("state")
  {}
  
  ~simple_observer()
  { std::cout<<"simple_observer is deleted\n"; }

  void 
  on_input(char const* tr_chunk, size_t byte_off)
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
{};

int main()
{
  using namespace boost;
  using namespace std;

  simple_observer so;
  mySubject s;
  s.meta_observable::attach(&so, bind(&simple_observer::on_meta, &so, _1));
  s.meta_observable::attach((void*)&free_function, bind(&free_function,_1));
  {
    shared_ptr<simple_observer> so1(new simple_observer);
    s.meta_observable::attach(&*so1, bind(&simple_observer::on_meta, weak_ptr<simple_observer>(so1), _1));
    // destruct so1 without detach, it is safe when using shared_ptr
    // though, this casues lifetime of observers be extened.
    // Therefore, you can see I used weak_ptr to pass the pointer so that
    // prevent from calling dead observers via upgrading weak_ptr to
    // shared_ptr. 
  }
  s.meta_observable::notify("test");
  
  cout<<"attached observers: "<<
    s.meta_observable::get_observers().size()<<"\n";
  
  s.meta_observable::wipe_dead_observers();

  cout<<"attached observers after wiped: "<<
    s.meta_observable::get_observers().size()<<"\n";

  return 0;
}
