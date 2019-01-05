#!/usr/bin/gnuplot

set terminal postscript enhanced color font ", 18" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'

set style line 1 pt 2 lt 1 lw 4 lc rgb "#e41a1c" ps 2
set style line 2 pt 3 lt 1 lw 4 lc rgb "#377eb8" ps 2
set style line 3 pt 4 lt 1 lw 4 lc rgb "#4daf4a" ps 2
set style line 4 pt 5 lt 1 lw 4 lc rgb "#984ea3" ps 2
set style line 5 pt 6 lt 1 lw 4 lc rgb "#ff7f00" ps 2
set style line 6 pt 7 lt 1 lw 4 lc rgb "#a65628" ps 2

set size 1.4, 1.1

set noborder
set noxtics
set noytics
set xrange [-10:-1]
set yrange [-10:10]  
set key maxrows 1
set key font ", 16"  

plot 'effectiveness' \
	using 1:($2)*100 w lp ls 1 title "w=1", \
"" using 1:($4)*100 w lp ls 2 title "w=2", \
"" using 1:($6)*100 w lp ls 3 title "w=3"
