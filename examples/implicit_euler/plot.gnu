set xlabel 'time'
set ylabel 'volume'
set xtics 0.5
set xrange [0.75:4.5]
set key left
plot 'soln' using 1:2 with points title "exact",\
'data_ie' using 1:2 with lines title "ie",\
'data_rk4' using 1:2 with lines title "rk4"
set terminal postscript eps mono 14
set output 'bucket.eps'
set size 0.8,0.7
replot
