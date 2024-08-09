#include "delegate.hpp"
#include "simple_heap_delegate.hpp"

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
T add_ref(const T& lhs)
{
    return lhs;
}

template<typename T>
T add_ref_2(const T& lhs, const T& rhs)
{
    return lhs + rhs;
}

template<typename T>
T add_ptr_2(const T& lhs, const T* rhs)
{
    return lhs + *rhs;
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

    T add_ref(const T& x)
    {
        return x + 10;
    }

    T add_ref_2(const T& a, const T& b)
    {
        return value + a + b;
    }

    T add_ptr_2(const T& a, const T* b)
    {
        return value + a + *b;
    }

    T add_ref_const(const T& x) const
    {
        return x + 10;
    }

    T add_ref_2_const(const T& a, const T& b) const
    {
        return value + a + b;
    }

    T add_ptr_2_const(const T& a, const T* b) const
    {
        return value + a + *b;
    }

    T value;
};

template<>
struct sdaineka::DelegateStorageStackSize<std::string, const std::string&>
{
    static constexpr std::size_t size()
    {
        return 64;
    }
};

static void test_add_int(const int value, const int bindValue, const int callValue)
{
    using T = int;
    using BarType = Bar<T>;

    BarType bar(value);

    auto lambda_add = [](const T v) { return v; };
    auto lambda_add_2 = [](const T lhs, const T rhs) { return lhs + rhs; };

    // StdFunction
    {
        using StdFunction = std::function<T(const T)>;

        std::vector<StdFunction> delegates = {std::bind(&add_2<T>, std::placeholders::_1, bindValue),
                                              std::bind(&BarType::add_2, &bar, std::placeholders::_1, bindValue),
                                              std::bind(&BarType::add_2_const, &bar, std::placeholders::_1, bindValue),
                                              std::bind(lambda_add_2, std::placeholders::_1, bindValue)};

        for (int i = 0; i < delegates.size(); i++)
        {
            const auto& d = delegates[i];
            std::cout << "std::function[" << i << "] - value: " << d(callValue) << ", size: " << sizeof(d) << '\n';
        }
    }

    // Delegate
    {
        using Delegate = sdaineka::SimpleHeapDelegate<T(const T)>;

        std::vector<Delegate> delegates;
        delegates.push_back(std::move(Delegate::CreateGlobal(&add_2<T>, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_2, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_2_const, bindValue)));
        delegates.push_back(std::move(Delegate::CreateLambda(lambda_add_2, bindValue)));

        for (int i = 0; i < delegates.size(); i++)
        {
            const auto& d = delegates[i];
            std::cout << "SimpleHeapDelegate[" << i << "] - value: " << d(callValue) << ", size: " << sizeof(d) << '\n';
        }
    }

    // Delegate
    {
        using Delegate = sdaineka::Delegate<T(const T)>;

        std::vector<Delegate> delegates;
        delegates.push_back(std::move(Delegate::CreateGlobal(&add_2<T>, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_2, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_2_const, bindValue)));
        delegates.push_back(std::move(Delegate::CreateLambda(lambda_add_2, bindValue)));

        for (int i = 0; i < delegates.size(); i++)
        {
            const auto& d = delegates[i];
            std::cout << "Delegate[" << i << "] - value: " << d(callValue) << ", size: " << sizeof(d) << '\n';
        }
    }
}

static void test_add_string(const std::string& value, const std::string& bindValue, const std::string& callValue)
{
    using T = std::string;
    using BarType = Bar<T>;

    BarType bar(value);

    auto lambda_add = [](const T& v) { return v; };
    auto lambda_add_2 = [](const T& lhs, const T rhs) { return lhs + rhs; };

    // StdFunction
    {
        using StdFunction = std::function<T(const T&)>;

        std::vector<StdFunction> delegates = {std::bind(&add_ref_2<T>, std::placeholders::_1, bindValue),
                                              std::bind(&add_ptr_2<T>, std::placeholders::_1, &bindValue),
                                              std::bind(&BarType::add_ref_2, &bar, std::placeholders::_1, bindValue),
                                              std::bind(&BarType::add_ptr_2, &bar, std::placeholders::_1, &bindValue),
                                              std::bind(&BarType::add_ref_2_const, &bar, std::placeholders::_1, bindValue),
                                              std::bind(&BarType::add_ptr_2_const, &bar, std::placeholders::_1, &bindValue),
                                              std::bind(lambda_add_2, std::placeholders::_1, bindValue)};

        for (int i = 0; i < delegates.size(); i++)
        {
            const auto& d = delegates[i];
            std::cout << "std::function[" << i << "] - value: " << d(callValue) << ", size: " << sizeof(d) << '\n';
        }
    }

    // SimpleHeapDelegate
    {
        using Delegate = sdaineka::SimpleHeapDelegate<T(const T&)>;

        std::vector<Delegate> delegates;
        delegates.push_back(std::move(Delegate::CreateGlobal(&add_ref_2<T>, bindValue)));
        delegates.push_back(std::move(Delegate::CreateGlobal(&add_ptr_2<T>, &bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ref_2, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ptr_2, &bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ref_2_const, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ptr_2_const, &bindValue)));
        delegates.push_back(std::move(Delegate::CreateLambda(lambda_add_2, bindValue)));

        for (int i = 0; i < delegates.size(); i++)
        {
            const auto& d = delegates[i];
            std::cout << "SimpleHeapDelegate[" << i << "] - value: " << d(callValue) << ", size: " << sizeof(d)
                      << ", heapSize: " << d.GetHeapSize() << '\n';
        }
    }

    // Delegate
    {
        using Delegate = sdaineka::Delegate<T(const T&)>;

        std::vector<Delegate> delegates;
        delegates.push_back(std::move(Delegate::CreateGlobal(&add_ref_2<T>, bindValue)));
        delegates.push_back(std::move(Delegate::CreateGlobal(&add_ptr_2<T>, &bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ref_2, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ptr_2, &bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ref_2_const, bindValue)));
        delegates.push_back(std::move(Delegate::CreateMember(&bar, &BarType::add_ptr_2_const, &bindValue)));
        delegates.push_back(std::move(Delegate::CreateLambda(lambda_add_2, bindValue)));

        for (int i = 0; i < delegates.size(); i++)
        {
            const auto& d = delegates[i];
            std::cout << "Delegate[" << i << "] - value: " << d(callValue) << ", size: " << sizeof(d) << ", heapSize: " << d.GetHeapSize()
                      << '\n';
        }
    }
}

int main(int argc, char* argv[])
{
    std::cout << "test_add(int)\n";
    test_add_int(5, 5, 5);

    std::cout << "test_add(std::string)\n";
    test_add_string(std::string("hello"), std::string("stan"), std::string("longlonglonglonglonglonglonglonglonglonglongstring"));

    return 0;
}