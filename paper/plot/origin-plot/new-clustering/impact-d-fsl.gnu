#!/usr/bin/gnuplot

set terminal postscript eps enhanced color font ",18" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'
set style line 1 pt 2 lt 1 lw 4 lc rgb "#e41a1c" ps 2
set style line 2 pt 3 lt 1 lw 4 lc rgb "#377eb8" ps 2
set style line 3 pt 4 lt 1 lw 4 lc rgb "#4daf4a" ps 2
set style line 4 pt 5 lt 1 lw 4 lc rgb "#984ea3" ps 2
set style line 5 pt 6 lt 1 lw 4 lc rgb "#ff7f00" ps 2
set style line 6 pt 7 lt 1 lw 4 lc rgb "#a65628" ps 2


#set key spacing 1.70

set size 0.6, 1.1
set multiplot layout 1,2 rowsfirst
set size 0.6, 0.5
set origin 0, 0.5
set xtics nomirror
set ytics nomirror
set ytics 10
set yrange [0:50]
set xrange [0.35:1.05]
set xlabel "k" offset 0, 0.5
set ylabel "Clustering Closeness (%)" offset 1.5, 0
set key maxrows 2 at screen 0.52, screen 1.1#for example

plot 'impact-d-fsl' \
	using 1:($2)*100 w lp ls 1 title "User004", \
"" using 1:($4)*100 w lp ls 2 title "User007", \
"" using 1:($6)*100 w lp ls 5 title "User015", \
"" using 1:($8)*100 w lp ls 6 title "User028"
#set key center left maxcols 5 maxrows 3  width 0

set size 0.6, 0.5
set origin 0, 0
set xtics nomirror
set ytics nomirror
set ytics 20
set yrange [0:100]
set xrange [0.35:1.05]
set xlabel "k" offset 0, 0.5
set ylabel "Clustering Correctness (%)" offset 1.5, 0

unset key
plot 'impact-d-fsl' \
	using 1:($3)*100 w lp ls 1 notitle, \
"" using 1:($5)*100 w lp ls 2 notitle, \
"" using 1:($7)*100 w lp ls 5 notitle, \
"" using 1:($9)*100 w lp ls 6 notitle
unset multiplot
