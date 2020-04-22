#include "Complex.h"

Complex Complex::operator*(Complex const &c2) const
{
    //(ac-bd) + (ad + bc)i
    return Complex(((this->a * c2.a) - (this->b * c2.b)), ((this->a * c2.b) + (this->b * c2.a)));
}

Complex Complex::operator+(Complex const &c2) const
{
    //(a+c) + (b+d)i
    return Complex((this->a + c2.a), (this->b + c2.b));
}

float Complex::lengthSquared() const
{
    //(a*a) + (b*b)
    return (this->a * this->a) + (this->b * this->b);
}

float Complex::getReal() const
{
    return this->a;
}

float Complex::getImaginary() const
{
    return this->b;
}

std::string Complex::toString() const
{
    std::stringstream ss;
    ss << this->a << " + " << this->b << "i";
    return ss.str();
}