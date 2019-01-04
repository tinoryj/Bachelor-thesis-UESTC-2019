#!/usr/bin/gnuplot

set terminal postscript enhanced color font "bold, 18" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'

set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5


#set xtics ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6, "128" 7, "256" 8, "512" 9, "1024" 10, "2048" 11, "4096" 12, "8192" 13, "16384" 14, "32768" 15)

set size 1.3, 1
set multiplot layout 1,2 rowsfirst
set size 0.65, 0.5
set origin 0, 0
set xtics nomirror
set ytics nomirror
set ytics 5
set yrange [0:25]
set xlabel "Auxiliary Information" offset 0, 0.5
set ylabel "Inference Rate (%)" offset 1.5, 0
set xtics nomirror
set ytics nomirror
set key maxrows 1 at screen 0.9, screen 0.55#for example
plot 'unique-severity' \
	using 1:($2)*100 w lp ls 1 title "w = 1", \
''	using 1:($3)*100 w lp ls 2 title "w = 2", \
''	using 1:($4)*100 w lp ls 3 title "w = 3"

set size 0.65, 0.5
set origin 0.58, 0
set ytics 20
set yrange [0:80]
set xlabel "Auxiliary Information" offset 0, 0.5
set ylabel "Inference Accuracy (%)" offset 1.5, 0
set ytics 20
unset key
plot 'accuracy-severity' \
	using 1:($2)*100 w lp ls 1 title "w = 1", \
''	using 1:($3)*100 w lp ls 2 title "w = 2", \
''	using 1:($4)*100 w lp ls 3 title "w = 3"
#'' using 1:($2)*100:($3)*100:($4)*100 ls 1 w yerrorbars notitle, \
#'' using 1:($5)*100:($6)*100:($7)*100 ls 2 w yerrorbars notitle, \
#'' using 1:($8)*100:($9)*100:($10)*100 ls 3 w yerrorbars notitle
unset multiplot
