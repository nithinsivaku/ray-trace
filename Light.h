#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "GeomLib.h"
#include "Color.h"

class Light {
public:
    // Point4 position;
    // Color color;
    // Light();

    Color ambient_light;
    Light(Point4 position, Color color);

    inline Color& getLightColor() {return c;};
    inline Point4& getLightPos() {return pos;};
    //inline Color& getambientLight() {return ambient_light;};
    


private:
	Color c;
	Point4 pos;
	
};

#endif
