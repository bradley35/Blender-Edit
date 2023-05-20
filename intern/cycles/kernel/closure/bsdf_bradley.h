#pragma once

CCL_NAMESPACE_BEGIN
typedef struct BradleyBsdf {
  SHADER_CLOSURE_BASE;
    float reflection;
    Spectrum diff_color;
    Spectrum spec_color;
} BradleyBsdf;

static_assert(sizeof(ShaderClosure) >= sizeof(BradleyBsdf), "BradleyBsdf is too large!");

ccl_device int bsdf_bradley_setup(ccl_private BradleyBsdf *bsdf)
{
    //bsdf->type = CLOSURE_BSDF_BRADLEY_A_ID;//ALWAYS BECOMES A HERE
    return SD_BSDF | SD_BSDF_HAS_EVAL;
}

ccl_device Spectrum bsdf_bradley_eval(ccl_private const ShaderClosure *sc,
                                      const float3 wi,
                                      const float3 wo,
                                      ccl_private float *pdf)
{
    ccl_private const BradleyBsdf *bsdf = (ccl_private const BradleyBsdf *)sc;
    *pdf = 1.0f;//Uniform over hemisphere
    float3 N = bsdf->N;
    
    float diff = dot(N, wo);//Ray comes in from wi (camera) and goes out towards wo (light source)
    if (diff <= 0){
        return make_spectrum(0.0f);
    }
    float3 r = -wo + 2*N*dot(wo, N);//Get amount in normal direction, and subtract twice to reflect across it
    float spec;
    if(dot(wo, N) <= 0){
        spec = 0.0f;
    }else{
        float dot_p = dot(wi, r);
        spec = powf(dot_p > 0 ? dot_p : 0.0f, bsdf->reflection);
    }
    
    return bsdf->diff_color*diff +bsdf->spec_color*spec;
}
//randu and randv distributed over 0 -> 1
ccl_device int bsdf_bradley_sample(ccl_private const ShaderClosure *sc,
                                   float3 Ng,
                                   float3 wi,
                                   float randu,
                                   float randv,
                                   ccl_private Spectrum *eval,
                                   ccl_private float3 *wo,
                                   ccl_private float *pdf)
{
    ccl_private const BradleyBsdf *bsdf = (ccl_private const BradleyBsdf *)sc;
    float3 N = bsdf->N;
    
    float3 a,b;
    make_orthonormals(N, &a, &b);//Using blender function since this seemed unnessarily complicated to implement and is irrelevent for the goal of this course
    
    float theta = (randu)*2.0f*M_PI_F; // From 0 to 360 degrees
    float phi = (randv)*M_PI_F/2.0f; // From 0 to 90 degrees
    
    //Use u & v to choose random amounts of a & b respectively
    float3 perp = (a * cosf(theta) + b * sinf(theta));
    *wo = N * cosf(phi) + perp * sinf(phi);
    
    
    *eval = bsdf_bradley_eval(sc, wi, *wo, pdf);
    return LABEL_DIFFUSE | LABEL_REFLECT;
}
CCL_NAMESPACE_END
