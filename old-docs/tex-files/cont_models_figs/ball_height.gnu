reset
unset key
set xrange [0:2.1]
set yrange [0:1.1]
set xlabel 'time (seconds)'
set ylabel 'height (meters)'
set label "BOOM !"  at 1.85,0.78
plot '../../../examples/book/cherry_bomb/data' using 1:2 with lines
set terminal postscript eps mono 14
set size 0.8,0.7
set output 'ball_height.eps'
replot
