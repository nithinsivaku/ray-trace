#include "Sphere.h"

Sphere::Sphere(Point4& center, float radius, Material& color)
    : Object(color) {

	this -> c = center;
	this -> r = radius;
	this -> m = color;

	


}

bool Sphere::intersects(Ray4& ray, Hit& hit) {

	Point4 P_s = ray.start;

	Vector4 V = ray.direction;

	Point4 P_sphere;
	Vector4 N_sphere;

	float a = V * V;
	float b = ( (2*V) * (P_s - c));
	float _c = (P_s - c) * (P_s - c)  - (r*r) ;

	// discriminant
	float d = ((b*b) - (4 * a * _c));


	if(d < 0)			// There is no intersection between ray and sphere
	{
		return false;
	}

	else // One or more intersections
	{
		float t_1 = (-b + sqrt(d))/(2*a);
        float t_2 = (-b - sqrt(d))/(2*a);
        float t = 0;

        if (t_1<t_2 && t_1>0)
        {
            t = t_1;
            
        }
        else if (t_2<=t_1 && t_2>0)
        {
            t = t_2;
        }
        else
        {
            return false;
        }

       	P_sphere = P_s + t*V;

        N_sphere = (P_sphere - c).normalized();

        hit.hit_point = ray.at(t);
        hit.normal = N_sphere;
        hit.material = m;
        hit.t = t;



        return true;

	}

 
    
}

