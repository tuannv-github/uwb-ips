import os
import numpy as np
import math as m
from matplotlib import pyplot as plt

ax = plt.gca()

DATA_DIR = "mobile-multi-cell/"
FILE_NAME = DATA_DIR + "location"

f = open(FILE_NAME, 'r')
for line in f.readlines():
    est = np.array(line.split(" ")[:-1])
    print(est)
    est = est.astype(np.float)
    if((not np.isnan(est[0])) & (not np.isnan(est[1]))):
        plt.plot(est[0], est[1], 'b+', zorder=0)

X = [5.5, 5.5,  12,     0,  0,      12.2,   14.5,   30.2,   30.2,   17,     13.5,   5.5]
Y = [-2,  -0.8, 3.6,    0,  3.6,    0,      -2.65,  -2.65,  -1.15,  -1.15,  -2.3,     -2]
plt.plot(X, Y, 'r', zorder=1)

X = [0, 0,      12.6,   12,     21.3,   21.3,   30.3,   30.3]
Y = [0, 3.6,    0,      3.6,    -1.15, -2.65,   -1.15,  -2.55]
plt.plot(X, Y, 'go', zorder=1)

plt.arrow(5.5, -2, 0, 0.3, width = 0.1, color ='g', ec='r', zorder=2) 

ax.set_xlim((-0.5, 31))
ax.set_ylim((-3.6, 3.8))
plt.axes().set_aspect('equal', 'datalim')
plt.axes().set_aspect('equal')
plt.grid()
plt.show()