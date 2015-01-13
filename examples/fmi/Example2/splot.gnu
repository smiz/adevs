reset
set terminal postscript eps mono 18
set output 'fig.eps'
set multiplot layout 1,3
set xlabel 'x error'
set ylabel 'z error'
unset key
set title 'zero sources'
set yrange [-0.007:0.007]
set xrange [-0.0065:0.0065]
set xtics 0.006
set ytics 0.003
plot 'data/serr0.dat' using 2:3 with lines 
set title 'ten sources'
plot 'data/serr10.dat' using 2:3 with lines 
set title 'thirty sources'
set yrange [-0.02:0.02]
set xrange [-0.015:0.015]
set xtics 0.01
set ytics 0.01
plot 'data/serr30.dat' using 2:3 with lines 
