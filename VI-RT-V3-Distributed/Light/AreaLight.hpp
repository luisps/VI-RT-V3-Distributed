//
//  PointLight.hpp
//  VI-RT-LPS
//
//  Created by Luis Paulo Santos on 06/04/2023.
//

#ifndef AreaLight_hpp
#define AreaLight_hpp

#include "light.hpp"
#include "triangle.hpp"
#include <math.h>

class AreaLight: public Light {
public:
    RGB intensity, power;
    Triangle *gem;
    float pdf;
    AreaLight (RGB _power, Point _v1, Point _v2, Point _v3, Vector _n): power(_power) {
        type = AREA_LIGHT;
        gem = new Triangle (_v1, _v2, _v3, _n);
        pdf = 1.f/gem->area();  // for uniform sampling over the area
        intensity = _power * pdf;
    }
    ~AreaLight () {delete gem;}
    // return the Light RGB radiance for a given point : p
    RGB L (Point p) {return power;}
    RGB L () {return power;}
    // return a point p and RGB radiance for a given probability pair r[2]
    // thye pdf should be tajen as 1/Area
    RGB Sample_L (float *r, Point *p) {
        // sample point as described in the "Gloabl illumination Compendium", page 12, item 18
        //return intensity;
        return RGB(0., 0., 0.);
    }
    // return a point p, RGB radiance and pdf given a pair of random number in [0..[
    RGB Sample_L (float *r, Point *p, float& _pdf) {
        RGB _intensity = Sample_L (r, p);
        _pdf = pdf;
        return _intensity;
    }
};

#endif /* AreaLight_hpp */
