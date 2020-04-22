
#ifndef COMPLEX_H
#define COMPLEX_H

#include <string>
#include <sstream>

//there is a Complex library, but I want to make my own
class Complex
{
public:
    Complex() {a = 0.0; b = 0.0;}
    Complex(float a, float b) : a(a), b(b) {}
    ~Complex() {}
    Complex operator*(Complex const &c2) const;
    Complex operator+(Complex const &c2) const;
    float lengthSquared() const;
    float getReal() const;
    float getImaginary() const;
    std::string toString() const;

private:
    float a; //real part
    float b; //imaginary part
};

#endif