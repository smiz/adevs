set xrange [0:4.5]
set xlabel 'time'
set key left
#set y2tics -5,2,16
set y2range [-10:16]
plot 'soln' using 1:2 with steps title 'Dough balls' axes x1y2, \
'soln' using 1:3 with steps title 'Baked product' axes x1y2 , \
'soln' using 1:4 with steps title 'Queue length' , \
'soln' using 1:5 with steps title 'Switch', \
'soln' using 1:6 with lines title 'Dough' 
