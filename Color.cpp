#include "Color.h"

Color::Color() {
}

Color::Color(float x, float y, float z)
{
	set(x, y, z);
}

void Color::set(float x, float y, float z)
{
	v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = 0.0f;
}

float Color::R() {
    return v[0];
}

float Color::G() {
    return v[1];
}

float Color::B() {
    return v[2];
}

Color Color::operator^(const Color& other) {
	return Color(v[0] * other.v[0], v[1] * other.v[1], v[2] * other[2]);
}

float Color::operator*(const Color& other) {
	double sum = 0;
    for (int i = 0; i < 4; i++)
        sum += v[i] * other.v[i];
    return sum;
}

Color Color::operator*(float d){
	return Color(v[0] * d, v[1] * d, v[2] * d);
}


Color Color::operator+(const Color& other){
	return Color(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
}




Color Color::operator+=(const Color& other){
	this->plus(other, *this);
    return *this;
}


