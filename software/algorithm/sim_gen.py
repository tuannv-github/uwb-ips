from matplotlib.pyplot import errorbar
import numpy as np
import matplotlib.pyplot as plt
import math as m

ANCHORS = np.array([
    [0, 0, 1],
    [5, 0, 2],
    [5, 5, 3],
    [0, 5, 5]
])

CENTER_X        = 2.5
CENTER_Y        = 2.5
RADIUS          = 2
STD_DEVIATION   = 0.1

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

anchor_x = ANCHORS[:, 0]
anchor_y = ANCHORS[:, 1]
anchor_z = ANCHORS[:, 2]
ax.scatter(anchor_x, anchor_y, anchor_z)

f = open("dataset.txt", 'w')
# f.write('anchor_x, anchor_y, anchor_z, d, tag_x, tag_y, tag_z\n')

TAGS = []
DISTANCES = []
f.write("[")
phis = np.arange(0, 2*m.pi, m.pi/50)
for j, phi in enumerate(phis):
    x = CENTER_X + RADIUS*m.cos(phi)
    y = CENTER_Y + RADIUS*m.sin(phi)
    z = 0
    TAGS.append([x,y,z])

    f.write('{"location": [' +str(x)+', '+str(y)+', '+str(z) + '], "data": [')
    data = []
    for i, anchor in enumerate(ANCHORS):
        distance = (x - anchor[0])*(x - anchor[0]) + (y - anchor[1])*(y - anchor[1]) + (z - anchor[2])*(z - anchor[2])
        distance = m.sqrt(distance) + np.random.rand()*STD_DEVIATION
        data.append(distance)

        f.write('{"x": ' + str(anchor[0])+', "y":'+str(anchor[1])+', "z":'+str(anchor[2])+', "d":'+str(distance)+"}")
        if i != len(ANCHORS) - 1:
            f.write(", ")
    if j != len(phis) - 1:
        f.write("]},\n")
    else:
        f.write("]}\n")
    DISTANCES.append(data)
f.write("]")

# print(DISTANCES)

TAGS = np.array(TAGS)
tag_x = TAGS[:, 0]
tag_y = TAGS[:, 1]
tag_z = TAGS[:, 2]
ax.scatter(tag_x, tag_y, tag_z)

f.close()
plt.show()