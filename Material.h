#if !defined(_MATERIAL_H_)

#define _MATERIAL_H_

#include "Color.h"

class Material {
public:
    Material();
    Material(Color& ambient, Color& diffuse, Color& specular, int shininess);
    void set(Color& ambient, Color& diffuse, Color& specular, int shininess);
    inline Color& getAmbient() {return ka;};
    inline Color& getDiffuse() {return kd;};
    inline Color& getSpecular() {return ks;};
    inline int getShininess() {return n;};

private:
    Color ka; // Ambient reflectance
    Color kd; // Diffuse reflectance
    Color ks; // Specular reflectance
    int n;    // Shininess exponent
};

#endif
