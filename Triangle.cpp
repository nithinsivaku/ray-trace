#include "Triangle.h"

Triangle::Triangle(Point4& v1, Point4& v2, Point4& v3, Material& color)
    : Object(color) {

    	this -> A = v1;
    	this -> B = v2;
    	this -> C = v3;
    	this -> mat = color;


}

bool Triangle::intersects(Ray4& ray, Hit& hit) {
    Point4 S = ray.start;
	Vector4 V = ray.direction;
	// The coefficients in the linear system of equation
    float a = V.X(), b = A.X() - B.X(), c = A.X() - C.X(), k = A.X() - S.X();
    float d = V.Y(), e = A.Y() - B.Y(), f = A.Y() - C.Y(), l = A.Y() - S.Y();
    float g = V.Z(), h = A.Z() - B.Z(), j = A.Z() - C.Z(), m = A.Z() - S.Z();
	// Cramer's rule
    float denom = Matrix4::det3x3(a,b,c,
                                  d,e,f,
                                  g,h,j);
    float t = Matrix4::det3x3(k,b,c,
                              l,e,f,
                              m,h,j) / denom;
    float u = Matrix4::det3x3(a,k,c,
                              d,l,f,
                              g,m,j) / denom;
    float v = Matrix4::det3x3(a,b,k,
                              d,e,l,
                              g,h,m) / denom;
    Point4 P_triang;
    Vector4 N_triang;
    float t_triang = -1;

    if (0 <= u && u <= 1 &&
        0 <= v && v <= 1 &&
        0 <= u+v && u+v <= 1 &&
        0 <= t) {
        t_triang = t;
        P_triang = S + t_triang * V;
        N_triang = ((B - A) ^ (C - A)).normalized();

        hit.hit_point = P_triang;
        hit.normal = N_triang;
        hit.material = mat;
        hit.t = t_triang;

        return true;
    }

    else
    {
    	return false;
    }
}
