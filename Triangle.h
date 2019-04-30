#if !defined(_TRIANGLE_H_)

#define _TRIANGLE_H_

#include "Object.h"
#include "GeomLib.h"
#include "Hit.h"

class Triangle : public virtual Object {
public:
    Triangle(Point4& v1, Point4& v2, Point4& v3, Material& color);
    void setNormal();
    bool intersects(Ray4& ray, Hit& hit);

private:
    Point4 A,B,C;
    Material mat;
};

#endif
