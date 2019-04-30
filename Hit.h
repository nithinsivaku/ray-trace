#ifndef _HIT_H_
#define _HIT_H_

#include "GeomLib.h"
#include "Material.h"

class Hit {
public:
    Point4 hit_point;
    Vector4 normal;
    Material material;
    float t;

    Hit();
};

#endif
