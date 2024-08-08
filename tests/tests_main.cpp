#include "delegate.hpp"

#include <functional>
#include <iostream>

template<typename T>
T add(const T lhs)
{
    return lhs;
}

template<typename T>
T add_2(const T lhs, const T rhs)
{
    return lhs + rhs;
}

template<typename T>
class Bar
{
public:
    Bar(const T v)
        : value(v)
    {
    }

    T add(const T x)
    {
        return x + 10;
    }

    T add_2(const T a, const T b)
    {
        return value + a + b;
    }

    T add_const(const T x) const
    {
        return x + 10;
    }

    T add_2_const(const T a, const T b) const
    {
        return value + a + b;
    }

    T value;
};

using DelegateString_v1 = sdaineka::v1::Delegate<std::string(const std::string&)>;
using DelegateInt_v1 = sdaineka::v1::Delegate<int(int)>;

using DelegateString_v2 = sdaineka::v2::Delegate<std::string(const std::string&)>;
using DelegateInt_v2 = sdaineka::v2::Delegate<int(int)>;

using StdFunctionString = std::function<std::string(const std::string&)>;
using StdFunctionInt = std::function<int(int)>;

template<>
struct sdaineka::v2::DelegateStackSize<int, int>
{
    static constexpr std::size_t size() { return 32; }
};

int main(int argc, char* argv[])
{
    Bar<std::string> bar_str("ss");
    Bar<int> bar_i(2);

    auto lambda_add = [](const auto v) { return v; };
    auto lambda_add_2 = [](const auto lhs, const auto rhs) { return lhs + rhs; };

    auto foo_v1 = DelegateInt_v1::CreateGlobal(&add_2<int>, 1);
    auto foo2_v1 = DelegateInt_v1::CreateMember(&bar_i, &Bar<int>::add_2, 2);
    auto foo3_v1 = DelegateInt_v1::CreateMember(&bar_i, &Bar<int>::add_2_const, 3);
    auto foo4_v1 = DelegateInt_v1::CreateLambda(lambda_add);

    std::cout << "foo_v1: " << foo_v1(5) << '\n';
    std::cout << "foo2_v1: " << foo2_v1(5) << '\n';
    std::cout << "foo3_v1: " << foo3_v1(5) << '\n';
    std::cout << "foo4_v1: " << foo4_v1(5) << '\n';

    // StdFunctionInt foo_std(&add<int>);
    // StdFunctionInt foo2_std = std::bind(&Bar<int>::add, &bar_i, std::placeholders::_1);
    // StdFunctionInt foo4_std = lambda_add;
    // std::cout << "foo_std size: " << sizeof(foo_std) << '\n';
    // std::cout << "foo2_std size: " << sizeof(foo2_std) << '\n';
    // std::cout << "foo4_std size: " << sizeof(foo2_std) << '\n';

    auto foo_v2 = DelegateInt_v2::CreateGlobal(&add_2<int>, 1);
    auto foo2_v2 = DelegateInt_v2::CreateMember(&bar_i, &Bar<int>::add_2, 2);
    auto foo3_v2 = DelegateInt_v2::CreateMember(&bar_i, &Bar<int>::add_2_const, 3);
    auto foo4_v2 = DelegateInt_v2::CreateLambda(lambda_add_2, 5);

    std::cout << "foo_v2: " << foo_v2(5) << '\n';
    std::cout << "foo2_v2: " << foo2_v2(5) << '\n';
    std::cout << "foo3_v2: " << foo3_v2(5) << '\n';
    std::cout << "foo4_v2: " << foo4_v2(5) << '\n';

    std::cout << "StackSize: " << foo4_v2.GetStackSize() << '\n';

    return 0;
}