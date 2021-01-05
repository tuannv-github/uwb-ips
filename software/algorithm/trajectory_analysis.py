import os
import numpy as np
import math as m
from matplotlib import pyplot as plt

ax = plt.gca()

DATA_DIR = "square"

DATA = "trajectory/"
FILE_NAME = DATA + "out"

f = open(FILE_NAME, 'r')
for line in f.readlines():
    est = np.array(line.split(" ")[:-1])
    print(est)
    est = est.astype(np.float)
    if((not np.isnan(est[0])) & (not np.isnan(est[1]))):
        plt.plot(est[0], est[1], 'b+')

ax.set_xlim((0, 35))
ax.set_ylim((-3.6, 3.6))
# plt.axes().set_aspect('equal', 'datalim')
plt.axes().set_aspect('equal')
plt.grid()
plt.show()