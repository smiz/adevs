reset
set terminal postscript eps mono 18
set output 'fig.eps'
set multiplot layout 1,3
set xlabel 'x error'
set ylabel 'z error'
unset key
set title 'zero sources'
set yrange [-0.007:0.007]
set xtics 0.006
set ytics 0.003
plot 'data/qerr0.dat' using 2:3 with lines 
set title 'ten sources'
plot 'data/qerr10.dat' using 2:3 with lines 
set title 'thirty sources'
set yrange [-0.035:0.035]
set xtics 0.04
set ytics 0.01
plot 'data/qerr30.dat' using 2:3 with lines 
