
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct location
{
    float x;
    float y;
    float z;
}location_t;

typedef struct sphere
{
    float x;
    float y;
    float z;
    float r;
}sphere_t;

typedef struct trilateration_result{
    location_t PA;
    location_t PB;
}trilateration_result_t;

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
    float a0tx = 0;
    float a0ty = 0;
    float a0tz = 0;

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
    if(fabs(distA - range) < fabs(distB - range)){
        memcpy(c, a, sizeof(location_t));
    }
    else{
        memcpy(c, b, sizeof(location_t));
    }
}

int main(){
    sphere_t spheres[3] = {
        {
            0,0,1,5.311632844683143
        },{
            5,0,2,3.33704088063755
        },{
            5,5,3,3.999118684674722
        }
    };
    trilateration_result_t tr;
    trilaterate(spheres, &tr);
    printf("PA: %.5f %.5f %.5f\n", tr.PA.x, tr.PA.y, tr.PA.z);
    printf("PA: %.5f %.5f %.5f\n", tr.PB.x, tr.PB.y, tr.PB.z);

    sphere_t sphere = {
        0,5,5,7.180278010403454
    };
    location_t result;
    nearest_finder(&sphere, &tr.PA, &tr.PB, &result);
    printf("Result: %.5f %.5f %.5f\n", result.x, result.y, result.z);
    return 0;
}