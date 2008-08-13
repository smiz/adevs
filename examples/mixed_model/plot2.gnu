set xrange [0:4.5]
set xlabel 'time'
set key left
plot 'soln' using 1:4 with steps title 'Queue length' , \
'soln' using 1:5 with steps title 'Switch', \
'soln' using 1:6 with lines title 'Dough' 
