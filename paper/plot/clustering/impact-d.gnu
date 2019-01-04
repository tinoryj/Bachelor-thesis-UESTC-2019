#!/usr/bin/gnuplot

set terminal postscript enhanced color font "bold,18" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'
set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5




set size 1.2, 1
set multiplot layout 1,2 rowsfirst
set size 0.6, 0.5
set origin 0, 0
set xtics nomirror
set ytics nomirror
set ytics 10 
set xrange [0.37:1.03]
set yrange [0:50]
set xlabel "d" offset 0, 0.5
set ylabel "Clustering Difference (%)" offset 1.5, 0
# set xtics ("2" 1, "8" 3, "32" 5,  "128" 7,  "512" 9,  "2048" 11)
set key maxrows 1 at screen 0.75, screen 0.55#for example
plot 'impact-d' \
	using 1:($4)*100 w lp ls 1 title "FSL", \
""  using 1:($2)*100 w lp ls 2 title "VM"
#set key center left maxcols 5 maxrows 3  width 0

set size 0.6, 0.5
set origin 0.58, 0
set xtics nomirror
set ytics nomirror
set ytics 20
set xrange [0.37:1.03]
set yrange [0:100]
set xlabel "d" offset 0, 0.5
set ylabel "Clustering Precision (%)" offset 1.5, 0

unset key
plot 'impact-d' \
	using 1:($3)*100 w lp ls 2 title "VM", \
""	using 1:($5)*100 w lp ls 1 title "FSL"

unset multiplot

