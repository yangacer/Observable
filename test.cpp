#include "bind_weak_ptr.hpp"

////////////////////////////////////////////////////////////////////////////////
//
//  Test
//
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>

struct C
{
    void f0()
    {
        std::cout << "f0 Called" << std::endl;
    }

    int f0c() const
    {
        std::cout << "f0c Called" << std::endl;
        return 0;
    }

    int f1(int a)
    {
        std::cout << "f1 Called" << std::endl;
        return 0;
    }

    int f1c(int a) const
    {
        std::cout << "f1c Called" << std::endl;
        return 0;
    }

    void f2(double a, const int &b)
    {
        std::cout << "f2 Called" << std::endl;
    }

    int f2c(const char *a, char &b) const
    {
        std::cout << "f2c Called" << std::endl;
        return 0;
    }
};

void test0()
{
    boost::shared_ptr<C> sp(new C);
    boost::weak_ptr<C> wp(sp);
    boost::function0<void> fp = boost::bind(&C::f0, wp);

    fp();
    sp.reset();

    try
    {
        fp();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void test0c()
{
    boost::shared_ptr<C> sp(new C);
    boost::weak_ptr<C> wp(sp);
    boost::function0<int> fp = boost::bind(&C::f0c, wp);

    fp();
    sp.reset();

    try
    {
        fp();
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void test1()
{
    boost::shared_ptr<C> sp(new C);
    boost::weak_ptr<C> wp(sp);
    boost::function1<int, int> fp = boost::bind(&C::f1, wp, _1);

    fp(1);
    sp.reset();

    try
    {
        fp(2);
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void test1c()
{
    boost::shared_ptr<C> sp(new C);
    boost::weak_ptr<C> wp(sp);
    boost::function1<int, int> fp = boost::bind(&C::f1c, wp, _1);

    fp(1);
    sp.reset();

    try
    {
        fp(2);
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void test2()
{
    boost::shared_ptr<C> sp(new C);
    boost::weak_ptr<C> wp(sp);
    boost::function2<void, double, const int&> fp = boost::bind(&C::f2, wp, _1, _2);

    fp(3.3, 1);
    sp.reset();

    try
    {
        fp(-0.1, 2);
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

void test2c()
{
    boost::shared_ptr<C> sp(new C);
    boost::weak_ptr<C> wp(sp);
    boost::function2<int, const char *, char &> fp = boost::bind(&C::f2c, wp, _1, _2);
    char c;

    fp("test", c);
    sp.reset();

    try
    {
        fp(0, c);
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

int main()
{
    test0();
    test0c();
    test1();
    test1c();
    test2();
    test2c();
    return 0;
}
