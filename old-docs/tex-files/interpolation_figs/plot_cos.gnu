reset
set yrange [-1.1:2]
set xlabel 't'
set xtics 0.5
set key bottom
plot '../../../examples/book/interpolate/data4' using 1:4 with lines title 'cos(t)',\
'../../../examples/book/interpolate/data4' using 1:5 with lines title '4 points',\
'../../../examples/book/interpolate/data5' using 1:5 with lines title '5 points',\
'../../../examples/book/interpolate/data6' using 1:5 with lines title '6 points'
set terminal postscript eps mono 14
set size 0.8,0.7
set output 'cos.eps'
replot
