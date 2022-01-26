import random
from math import *

wl = [0.01, 0.05, 0.1, 0.2];
n = 21;

for i in range(1,n):
	line = str(i);
	for w in wl:
		p1 = w*pow(1.0-w,float(i-1));
		p2 = w*(1.0-exp(-w*float(i)));
		line = line + ' ' + str(p1-p2);
	print(line)
