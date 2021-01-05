import os
import numpy as np
import math as m
from matplotlib import pyplot as plt

ax = plt.gca()

plt.plot([0.6, 12, 12, 0.6, 0.6], [0.6, 0.6, 3, 3, 0.6], 'r')

DATA_DIR = "square"
f = open(DATA_DIR + "/" + "06", 'r')
for line in f.readlines():
    est = np.array(line.split(" ")[:-1])
    print(est)
    est = est.astype(np.float)
    if((not np.isnan(est[0])) & (not np.isnan(est[1]))):
        plt.plot(est[0], est[1], 'b+')

ax.set_xlim((0, 12.6))
ax.set_ylim((0, 3.6))
# plt.axes().set_aspect('equal', 'datalim')
plt.axes().set_aspect('equal')
plt.grid()
plt.show()