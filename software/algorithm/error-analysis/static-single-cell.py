import os
import numpy as np
import math as m
from matplotlib import pyplot as plt

DATA_DIR = "location"

ax = plt.gca()

for filename in os.listdir(DATA_DIR):
    location = filename.split("_")
    location = np.array(location)
    location = location.astype(np.float)
    f = open(DATA_DIR + "/" + filename, 'r')
    plt.plot(location[:-1][0], location[:-1][1], 'bo')
    d_mean = 0
    d_cout = 0
    for line in f.readlines():
        est = np.array(line.split(" ")[:-1])
        est = est.astype(np.float)
        if((not np.isnan(est[0])) & (not np.isnan(est[1])) & (not np.isnan(est[2]))):
            dx = est[0] - location[0]
            dy = est[1] - location[1]
            dz = abs(est[2]) - abs(location[2])
            d = m.sqrt(dx**2 + dy**2 + dz**2)
            # print(d)
            d_mean += d
        d_cout += 1
    d_mean /= d_cout
    circle1 = plt.Circle(location[:-1], d_mean, color='r', fill=False)
    ax.add_artist(circle1)
    print(str(location) + ": " + str(d_mean))

ax.set_xlim((-2, 14))
ax.set_ylim((-2, 6))
plt.axes().set_aspect('equal')
plt.grid()
plt.show()