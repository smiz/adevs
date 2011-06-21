reset
set key left
set y2tics auto
plot 'soln' using 1:2 with lines title 'v',\
'' using 1:3 with lines title 'omega',\
'' using 1:4 with lines title 'theta',\
'' using 1:5 with lines title 'x' axis x1y2,\
'' using 1:6 with lines title 'y' axis x1y2
