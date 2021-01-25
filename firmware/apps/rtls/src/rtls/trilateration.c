
#include <math.h>
#include <string.h>
#include <rtls/rtls/trilateration.h>

void trilaterate(sphere_t sphere[3], trilateration_result_t *trilateration_result){

    /* Prepare all parameters */
    float a0x = sphere[0].x;
    float a0y = sphere[0].y;
    float a0z = sphere[0].z;

    float a1x = sphere[1].x;
    float a1y = sphere[1].y;
    float a1z = sphere[1].z;

    float a2x = sphere[2].x;
    float a2y = sphere[2].y;
    float a2z = sphere[2].z;

    float r0 = sphere[0].r;
    float r1 = sphere[1].r;
    float r2 = sphere[2].r;

    /* Transform to standard coordinate */
    // float a0tx = 0;
    // float a0ty = 0;
    // float a0tz = 0;

    float a1tx = a1x - a0x;
    float a1ty = a1y - a0y;
    float a1tz = a1z - a0z;

    float a2tx = a2x - a0x;
    float a2ty = a2y - a0y;
    float a2tz = a2z - a0z;

    float U = sqrt(a1tx*a1tx + a1ty*a1ty + a1tz*a1tz);
    float exx = a1tx/U;
    float exy = a1ty/U;
    float exz = a1tz/U;
    float Vx = a2tx*exx + a2ty*exy + a2tz*exz;
    float temp_x = a2tx - Vx*exx;
    float temp_y = a2ty - Vx*exy;
    float temp_z = a2tz - Vx*exz;
    float Vy = sqrt(temp_x*temp_x + temp_y*temp_y + temp_z*temp_z);
    float eyx = temp_x/Vy;
    float eyy = temp_y/Vy;
    float eyz = temp_z/Vy;
    float ezx = exy*eyz - exz*eyy;
    float ezy = exz*eyx - exx*eyz;
    float ezz = exx*eyy - exy*eyx;

    /* Trilateration */
    float x  = (r0*r0 - r1*r1 + U*U) / (2*U);
    float y  = (r0*r0 - r2*r2 + Vx*Vx + Vy*Vy - 2*Vx*x) / (2*Vy);
    float z  = r0*r0 - x*x - y*y;
    if(z < 0) z = 0;
    float z0 = sqrt(z);
    float z1 = -z0;

    /* Transform to original coordinate */
    trilateration_result->PA.x = a0x + x*exx + y*eyx + z0*ezx;
    trilateration_result->PA.y = a0y + x*exy + y*eyy + z0*ezy;
    trilateration_result->PA.z = a0z + x*exz + y*eyz + z0*ezz;

    trilateration_result->PB.x = a0x + x*exx + y*eyx + z1*ezx;
    trilateration_result->PB.y = a0y + x*exy + y*eyy + z1*ezy;
    trilateration_result->PB.z = a0z + x*exz + y*eyz + z1*ezz;
}

void nearest_finder(sphere_t *sphere, location_t *a, location_t *b, location_t *c){
    float distA = (a->x - sphere->x)*(a->x - sphere->x)
                + (a->y - sphere->y)*(a->y - sphere->y)
                + (a->z - sphere->z)*(a->z - sphere->z);
    float distB = (b->x - sphere->x)*(b->x - sphere->x)
                + (b->y - sphere->y)*(b->y - sphere->y)
                + (b->z - sphere->z)*(b->z - sphere->z);
    float range = sphere->r*sphere->r;
    if(fabs(distA - range) < fabs(distB - range) || (a->z < 3)){
        memcpy(c, a, sizeof(location_t));
    }
    else{
        memcpy(c, b, sizeof(location_t));
    }
}
