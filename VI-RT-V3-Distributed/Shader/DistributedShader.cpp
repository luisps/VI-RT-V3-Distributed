//
//  WhittedShader.cpp
//  VI-RT-LPS
//
//  Created by Luis Paulo Santos on 14/03/2023.
//

#include "DistributedShader.hpp"
#include "BRDF.hpp"
#include "ray.hpp"


inline Vector refract(const Vector& V, const Vector& N, double IOR) {
    auto cos_theta = std::fmin(N.dot(-1.*V), 1.0);

    // is there Total Internal Reflection ?
    
    Vector const r_out_perp =  IOR * (V + cos_theta*N);
    Vector const r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.normSQ())) * N;
    Vector T = r_out_perp + r_out_parallel;
    T.normalize();
    return T;
}

inline Vector reflect(const Vector& V, const Vector& N) {
    float cos = N.dot(V);
    return 2.f * cos * N - V;
}

RGB DistributedShader::specularReflection (Intersection isect, BRDF *f, int depth) {
    RGB color(0.,0.,0.);

    // generate the specular ray
    // direction R = 2 (N.V) N - V
    Vector Rdir = reflect(isect.wo, isect.sn);
    Ray specular(isect.p, Rdir);
    
    specular.pix_x = isect.pix_x;
    specular.pix_y = isect.pix_y;
    
    specular.FaceID = isect.FaceID;

    specular.adjustOrigin(isect.gn);
    specular.propagating_eta = isect.incident_eta;  // same medium

    // OK, we have the ray : trace and shade it recursively
    bool intersected;
    Intersection s_isect;
    // trace ray
    intersected = scene->trace(specular, &s_isect);

    // shade this intersection
    color = f->Ks * shade (intersected, s_isect, depth+1);

    return color;
}

RGB DistributedShader::specularTransmission (Intersection isect, BRDF *f, int depth) {
    RGB color(0., 0., 0.);

    // generate the transmission ray
    // from https://raytracing.github.io/books/RayTracingInOneWeekend.html#dielectrics
    
    // direction T = IOR * V
    float const IOR = isect.incident_eta / isect.f->eta;
    Vector const V = -1.*isect.wo;
    Vector const N = isect.sn;
    
    auto cos_theta = std::fmin(N.dot(-1.*V), 1.0);
    double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);
    
    // is there total internal reflection ?
    bool const cannot_refract = (IOR*sin_theta>1.);
    
    Vector const dir = (cannot_refract ? reflect(V,N) : refract (V, N, IOR));

    Ray refraction(isect.p, dir);
    
    refraction.pix_x = isect.pix_x;
    refraction.pix_y = isect.pix_y;
    
    refraction.FaceID = isect.FaceID;

    refraction.adjustOrigin(-1. * isect.gn);
    
    refraction.propagating_eta = (cannot_refract ? isect.incident_eta : isect.f->eta);

    // OK, we have the ray : trace and shade it recursively
    bool intersected;
    Intersection t_isect;
    // trace ray
    intersected = scene->trace(refraction, &t_isect);

    // shade this intersection
    color = f->Kt * shade (intersected, t_isect, depth+1);
   
    return color;
}

RGB DistributedShader::shade(bool intersected, Intersection isect, int depth) {
    RGB color(0.,0.,0.);
    
    // if no intersection, return background
    if (!intersected) {
        return (background);
    }
    if (isect.isLight) { // intersection with a light source
        return isect.Le;
    }
    // get the BRDF
    BRDF *f = isect.f;
    
    #define MAX_DEPTH 4
    // if there is a specular component sample it
    if (!f->Ks.isZero() && depth<MAX_DEPTH) {
        color += specularReflection (isect, f, depth+1);
    }
    // if there is a specular component sample it
    if (!f->Kt.isZero() && depth<MAX_DEPTH) {
        color += specularTransmission (isect, f, depth+1);
    }
    
    color += directLighting(scene, isect, f, rng, U_dist);

    //color += directLighting(scene, isect, f, rng, U_dist, UNIFORM_ONE);
    //color += directLighting(scene, isect, f, rng, U_dist, ALL_LIGHTS);

    return color;
};
