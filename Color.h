#if !defined(_COLOR_H_)

#define _COLOR_H_

#include "GeomLib.h"

class Color : public Float4 {
public:
    float R();
    float G();
    float B();

    Color();
    Color(float x, float y, float z);
    void set(float x, float y, float z);
    Color operator^(const Color& other);
    Color operator*(float d);
    Color operator+(const Color& other);
    float operator*(const Color& other);
    Color operator+=(const Color& other);

};

#endif

