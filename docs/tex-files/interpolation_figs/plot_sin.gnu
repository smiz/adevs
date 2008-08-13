reset
set yrange [-1.2:1.2]
set xlabel 't'
set xtics 0.5
set key bottom
set key left
plot '../../../examples/book/interpolate/data4' using 1:2 with lines title 'sin(t)',\
'../../../examples/book/interpolate/data4' using 1:3 with lines title '4 points',\
'../../../examples/book/interpolate/data5' using 1:3 with lines title '5 points',\
'../../../examples/book/interpolate/data6' using 1:3 with lines title '6 points'
set terminal postscript eps mono 14
set size 0.8,0.7
set output 'sin.eps'
replot
