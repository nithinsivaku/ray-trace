#include "Material.h"

Material::Material() {
}

Material::Material(Color& ambient, Color& diffuse, Color& specular, int shininess)
{
	this->ka = ambient;
	this->kd = diffuse;
	this->ks = specular;
	this->n = shininess;
}




