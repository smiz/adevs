reset
unset key
set xlabel 'time'
set ylabel 'volts'
set arrow from 1,0 to 1,0.5 
set label "Vref=0.5" at 1,0.55
set arrow from 1.94484,0 to 1.94484,0.5 
set label "Vref reached" at 1.94484,0.55
plot 'soln' using 1:2 with lines
