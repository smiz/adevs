reset
set terminal pngcairo
set output 'circuit.png'
set key opaque
set y2range [-0.1:1.2]
set ylabel 'capacitor voltage'
set xlabel 'seconds'
set ytics nomirror
set y2tics nomirror
set y2tics ("conducting" 1,"not conducting" 0)
plot 'soln.txt' using 1:2 with lines title 'voltage' lw 2, '' using 1:3 with steps lw 2 dashtype 2 title 'switch' axis x1y2, '' using 1:4 with steps title 'diode' lw 2 dashtype 4 axis x1y2

