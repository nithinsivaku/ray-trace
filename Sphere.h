#if !defined(_SPHERE_H_)

#define _SPHERE_H_

#include "Object.h"
#include "GeomLib.h"
#include "Material.h"
#include "Hit.h"

class Sphere : public virtual Object {
public:
    Sphere(Point4& center, float radius, Material& color);
    bool intersects(Ray4& ray, Hit& hit);

private:
    Point4 c;
    float r;
    Material m;
};

#endif
