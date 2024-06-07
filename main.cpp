#include <iostream>

#include <flow/heterogeneous.hpp>

using namespace flow;

struct printer : visitor<int, double, std::string> {
    void operator()(auto && value) const {
        std::cout << value << '\n';
    }
};

class base {
public:
    void foo() {
        // ...
        m_additional_data.visit(printer{});
    }

protected:
    heterogeneous_container m_additional_data;
};

class derived : public base {
public:
    derived() {
        m_additional_data.push_back(42);
        m_additional_data.emplace_back<std::string>("Hello world");
    }
};

int main() {
    heterogeneous_container container;
    container.push_back(42);
    container.push_back(4.5);
    container.emplace_back<std::string>("Hello world");

    container.visit(printer{});

    return 0;
}