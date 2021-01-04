import os
import numpy as np
import math as m
from matplotlib import pyplot as plt

ax = plt.gca()

DATA_DIR = "square"
f = open(DATA_DIR + "/" + "06", 'r')
for line in f.readlines():
    est = np.array(line.split(" ")[:-1])
    est = est.astype(np.float)
    if((not np.isnan(est[0])) & (not np.isnan(est[1]))):
        plt.plot(est[0], est[1], 'bo')

ax.set_xlim((0, 12.6))
ax.set_ylim((0, 3.6))
# plt.axes().set_aspect('equal', 'datalim')
plt.axes().set_aspect('equal')
plt.show()