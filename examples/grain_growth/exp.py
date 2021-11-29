import random

w = 0.99;

trials = 10000000;
samples = dict();

for i in range(0,trials):
	count = 0;
	while random.random() > w:
		count = count+1;
	if not count in samples:
		samples[count] = 1;
	else:
		samples[count] = samples[count]+1;

for key in samples:
	print(float(key),float(samples[key])/float(trials));
