import numpy as np
import json
import math as m

def trilaterate(ranges):
    p0=np.array(ranges[0][:3])
    p1=np.array(ranges[1][:3])
    p2=np.array(ranges[2][:3])

    d0=ranges[0][-1]
    d1=ranges[1][-1]
    d2=ranges[2][-1]

    pt0 = [0,0,0]
    pt1 = p1 - p0
    pt2 = p2 - p0

    U = np.linalg.norm(pt1)
    e_x = pt1/U
    Vx = np.dot(pt2, e_x)
    pt2y = (pt2 - Vx*e_x)
    Vy  = np.linalg.norm(pt2y)
    e_y = pt2y / Vy
    e_z = np.cross(e_x, e_y)

    x  = (d0**2 - d1**2 + U**2) / (2*U)
    y  = (d0**2 - d2**2 + Vx**2 + Vy**2 - 2*Vx*x)/(2*Vy)
    z0 = np.sqrt(d0**2 - x**2 - y**2)
    z1 = -z0

    PA=p0+(x*e_x)+(y*e_y)+(z0*e_z)
    PB=p0+(x*e_x)+(y*e_y)+(z1*e_z)

    return [PA, PB]

def trilaterate3D(distances):
    # p0=np.array(distances[0][:3])
    # p1=np.array(distances[1][:3])
    # p2=np.array(distances[2][:3])

    # d0=distances[0][-1]
    # d1=distances[1][-1]
    # d2=distances[2][-1]

    # # ROTATION OF NORMALIZED VECTOR OF P TO z AXIS
    # a = (p1[1] - p0[1])*(p2[2] - p0[2]) - (p2[1] - p0[1])*(p1[2] - p0[2])
    # b = (p2[0] - p0[0])*(p1[2] - p0[2]) - (p1[0] - p0[0])*(p2[2] - p0[2])
    # c = (p1[0] - p0[0])*(p2[1] - p0[1]) - (p2[0] - p0[0])*(p1[1] - p0[1])
    # rms_abc = m.sqrt(a**2 + b**2 + c**2)
    # u = a/rms_abc
    # v = b/rms_abc
    # w = c/rms_abc

    # rms_uv = m.sqrt(u**2 + v**2)

    # A1xy = [0,0,0]
    # A2xy = [0,0,0]

    # if rms_uv != 0:
    #     cos_theta = u/rms_uv
    #     sin_theta = v/rms_uv
    #     cos_psi = w
    #     sin_psi = rms_uv
    #     # ROTATION OF A2 AND A3 TO x y PLANE
    #     A1xy[0] = w*(u*(p1[0]-p0[0] + v*(p1[1]-p0[1])))/rms_uv - (p1[2]-p0[2])*rms_uv
    #     A1xy[1] = (u*(p1[1]-p0[1])- v*(p1[0]-p0[0]))/rms_uv
    #     A1xy[2] = 0

    #     A2xy[0] = w*(u*(p2[0]-p0[0] + v*(p2[1]-p0[1])))/rms_uv - (p2[2]-p0[2])*rms_uv
    #     A2xy[1] = (u*(p2[1]-p0[1])- v*(p2[0]-p0[0]))/rms_uv
    #     A2xy[2] = 0
    # else:
    #     A1xy = p1
    #     A2xy = p2
    
    # rms = m.sqrt(A1xy[0]**2 + A1xy[1]**2)
    # cos_phi = A1xy[0]/rms
    # sin_phi = A1xy[1]/rms

    # A1 = [0,0,0]
    # A2 = [0,0,0]

    # A1[0] =  A1xy[0]*cos_phi + A1xy[1]*sin_phi
    # A1[1] = -A1xy[0]*sin_phi + A1xy[1]*cos_phi
    # A1[2] = 0

    # A2[0] =  A2xy[0]*cos_phi + A2xy[1]*sin_phi
    # A2[1] = -A2xy[0]*sin_phi + A2xy[1]*cos_phi
    # A2[2] = 0

    # # SIMPLIFIED TRILATERATION
    # PA = [0,0,0]
    # PB = [0,0,0]
    # PA[0] = PB[0] = (d0**2 - d1**2 + A1[0]**2)/(2*A1[0])
    # PA[1] = PB[1] = (d0**2 - d2**2 + A2[0]**2 + A2[1]**2 - 2*A2[0]*PA[0])/(2*A2[1])
    # PA[2] = m.sqrt(d0**2 - PA[0]**2 - PA[1]**2)
    # PB[2] = -PA[2]    
    # return [0, 0 ,0]

    p1=np.array(distances[0][:3])
    p2=np.array(distances[1][:3])
    p3=np.array(distances[2][:3])
    p4=np.array(distances[3][:3])
    r1=distances[0][-1]
    r2=distances[1][-1]
    r3=distances[2][-1]
    r4=distances[3][-1]
    e_x=(p2-p1)/np.linalg.norm(p2-p1)
    i=np.dot(e_x,(p3-p1))
    e_y=(p3-p1-(i*e_x))/(np.linalg.norm(p3-p1-(i*e_x)))
    e_z=np.cross(e_x,e_y)
    d=np.linalg.norm(p2-p1)
    j=np.dot(e_y,(p3-p1))
    x=((r1**2)-(r2**2)+(d**2))/(2*d)
    y=(((r1**2)-(r3**2)+(i**2)+(j**2))/(2*j))-((i/j)*(x))
    z1=np.sqrt(r1**2-x**2-y**2)
    z2=np.sqrt(r1**2-x**2-y**2)*(-1)
    ans1=p1+(x*e_x)+(y*e_y)+(z1*e_z)
    ans2=p1+(x*e_x)+(y*e_y)+(z2*e_z)
    dist1=np.linalg.norm(p4-ans1)
    dist2=np.linalg.norm(p4-ans2)
    if np.abs(r4-dist1)<np.abs(r4-dist2):
        return ans1
    else: 
        return ans2

read = open("dataset.txt", "r")
write = open("location.txt", "w")
lines = json.load(read)

for dataset in lines:
    eq = []
    for data in dataset["data"]:
        temp = []
        temp.append(data['x'])
        temp.append(data['y'])
        temp.append(data['z'])
        temp.append(data['d'])
        eq.append(temp)
    location = trilaterate(eq)
    # location = trilaterate3D(eq)
    print(location)
    write.write(str(location) + "\n")

read.close()
write.close()