import numpy as np
from scipy.interpolate import interp1d
import matplotlib.pyplot as plt
import os

layer1_data = 'run027_1layer/fit_results.txt'
layer2_data = 'run030_2layer/fit_results.txt'

M_1layer = np.genfromtxt(layer1_data)
M_1layer = M_1layer[np.argsort(M_1layer[:, 0])]
f1_int=interp1d(np.append(0,M_1layer[:,0]), np.append(0,M_1layer[:,2]))
x_1layer = np.linspace(M_1layer[0,0],M_1layer[-1,0]-1,300)
#
plt.plot(x_1layer,f1_int(x_1layer))
plt.plot(M_1layer[:,0], M_1layer[:,2],'.', label = '1 layer')
##plt.show()
#
M_2layer = np.genfromtxt(layer2_data)
M_2layer = M_2layer[np.argsort(M_2layer[:, 0])]
np.savetxt('test.out', M_2layer, fmt='%1.4e')
f2_int=interp1d(np.append(0,M_2layer[:,0]), np.append(0,M_2layer[:,2]))
x_2layer = np.linspace(M_2layer[0,0],M_2layer[-1,0]-1,300)
plt.plot(x_2layer,f2_int(x_2layer))
plt.plot(M_2layer[:,0], M_2layer[:,2], '.', label = '2 layer')

def new_x(x1, x2):
    low = max([x1[0], x2[0]])
    high = min([x1[-1], x2[-1]])
    return np.linspace(low, high, 300)

xtest = new_x(x_1layer, x_2layer)
plt.plot(xtest,f1_int(f1_int(xtest)))
plt.xlabel('Applied Field [mT]')
plt.ylabel('$B_{leak}$ [mT]')
plt.legend(loc = 'best')
#plt.xlim(0,100)
#plt.ylim(0,80)
plt.loglog()
plt.show()
