#!/usr/bin/gnuplot

set terminal postscript enhanced color 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'
set size 0.6,0.5

set style line 1 pt 7 lt 1 lw 2 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 2 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 2 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 2 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 2 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 2 lc rgb "#a65628" ps 1.5



set xtics nomirror
set ytics nomirror
set xrange [0.75:7.25]

set xlabel "Average Chunk Size" offset 0, 0.5
set ylabel "Unique Inference Rate (%)"
set key above

set xtics ("2KB" 1, "4KB" 2, "8KB" 3, "16KB" 4, "32KB" 5, "64KB" 6, "128KB" 7)

plot 'unique-chunk-size' \
	using 1:($2)*100 w lp ls 1 title "User004", \
"" using 1:($3)*100 w lp ls 2 title "User007", \
"" using 1:($4)*100 w lp ls 4 title "User013", \
"" using 1:($5)*100 w lp ls 6 title "User028"
#set key center left maxcols 5 maxrows 3  width 0
