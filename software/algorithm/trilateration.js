/** Javascript implementation for trilateration
 * 
 * @param {*} ranges range data with format: [[x,y,z,d],[x,y,z,d],[x,y,z,d]]
 */
function trilaterate(ranges){

    /* Prepare all parameters */
    a0x = ranges[0][0];
    a0y = ranges[0][1];
    a0z = ranges[0][2];

    a1x = ranges[1][0];
    a1y = ranges[1][1];
    a1z = ranges[1][2];

    a2x = ranges[2][0];
    a2y = ranges[2][1];
    a2z = ranges[2][2];

    r0 = ranges[0][3];
    r1 = ranges[1][3];
    r2 = ranges[2][3];

    /* Transform to standard coordinate */
    a0tx = 0;
    a0ty = 0;
    a0tz = 0;

    a1tx = a1x - a0x;
    a1ty = a1y - a0y;
    a1tz = a1z - a0z;

    a2tx = a2x - a0x;
    a2ty = a2y - a0y;
    a2tz = a2z - a0z;

    U = Math.sqrt(a1tx*a1tx + a1ty*a1ty + a1tz*a1tz);
    exx = a1tx/U;
    exy = a1ty/U;
    exz = a1tz/U;
    Vx = a2tx*exx + a2ty*exy + a2tz*exz;
    temp_x = a2tx - Vx*exx;
    temp_y = a2ty - Vx*exy;
    temp_z = a2tz - Vx*exz;
    Vy = Math.sqrt(temp_x*temp_x + temp_y*temp_y + temp_z*temp_z);
    eyx = temp_x/Vy;
    eyy = temp_y/Vy;
    eyz = temp_z/Vy;
    ezx = exy*eyz - exz*eyy;
    ezy = exz*eyx - exx*eyz;
    ezz = exx*eyy - exy*eyx;

    /* Trilateration */
    x  = (r0*r0 - r1*r1 + U*U) / (2*U);
    y  = (r0*r0 - r2*r2 + Vx*Vx + Vy*Vy - 2*Vx*x) / (2*Vy);
    z0 = Math.sqrt(r0**2 - x**2 - y**2);
    z1 = -z0;

    /* Transform to original coordinate */
    PAx = a0x + x*exx + y*eyx + z0*ezx;
    PAy = a0y + x*exy + y*eyy + z0*ezy;
    PAz = a0z + x*exz + y*eyz + z0*ezz;

    PBx = a0x + x*exx + y*eyx + z1*ezx;
    PBy = a0y + x*exy + y*eyy + z1*ezy;
    PBz = a0z + x*exz + y*eyz + z1*ezz;

    /* Return the result */
    return [[PAx, PAy, PAz], [PBx, PBy, PBz]];
}

function nearest_finder(sphere, locationA, locationB){
    distA = (locationA[0]- sphere[0])*(locationA[0]- sphere[0]) 
            + (locationA[1]- sphere[1])*(locationA[1]- sphere[1])
            + (locationA[2]- sphere[2])*(locationA[2]- sphere[2]);
    distB = (locationB[0]- sphere[0])*(locationB[0]- sphere[0]) 
            + (locationB[1]- sphere[1])*(locationB[1]- sphere[1])
            + (locationB[2]- sphere[2])*(locationB[2]- sphere[2]);    
    range = sphere[3]*sphere[3];
    if(Math.abs(distA - range) < Math.abs(distB - range)){
        return locationA;
    }
    else{
        return locationB;
    }
}

const fs = require('fs');
let data = fs.readFileSync('dataset.txt');
let json = JSON.parse(data);

// ranges = json[0].data
// console.log(ranges)
// location = trilaterate(ranges);
// console.log(location);

fs.writeFileSync("location_js.txt","",(err) => {
    if (err) throw err;
});
for(let i = 0; i < json.length; i++) {
    data = json[i]["data"];
    ranges = [[data[0].x, data[0].y, data[0].z, data[0].d], 
              [data[1].x, data[1].y, data[1].z, data[1].d],
              [data[2].x, data[2].y, data[2].z, data[2].d]];
    location = trilaterate(ranges);
    P = nearest_finder([data[3].x, data[3].y, data[3].z, data[3].d], location[0], location[1])
    fs.appendFileSync("location_js.txt", P.toString() + "\r\n", (err) => {
        if (err) throw err;
    });
    console.log(P);
}