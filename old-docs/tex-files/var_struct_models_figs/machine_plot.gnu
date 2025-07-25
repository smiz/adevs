reset
unset key
set xrange [0:360]
set yrange [0:5]
set xlabel 'time (days)'
set ylabel 'machine count'
plot '../../../examples/book/factory/data' with steps
set terminal postscript eps mono 14
set size 0.8,0.7
set output 'machine_plot.eps'
replot
