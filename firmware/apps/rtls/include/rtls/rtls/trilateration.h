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

void trilaterate(sphere_t sphere[3], trilateration_result_t *trilateration_result);
void nearest_finder(sphere_t *sphere, location_t *a, location_t *b, location_t *c);