#!/usr/bin/gnuplot

set terminal postscript enhanced color font "bold, 20" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'

set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5
set style fill solid
set style histogram cluster gap 1.5
set style data histograms

set boxwidth 0.94
#set xtics ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6, "128" 7, "256" 8, "512" 9, "1024" 10, "2048" 11, "4096" 12, "8192" 13, "16384" 14, "32768" 15)

set size 1.2, 1.6
set multiplot layout 2,1 columnsfirst
set origin 0, 0.9
set size 1.2, 0.5
set xtics nomirror
set ytics nomirror
set key  maxrows 2 at screen 0.95, screen 1.45#for example
set ytics 5
set xrange [-0.5:2.5]
set yrange [0:25]
set xlabel "Attack Instance" offset 0, 0
#set xlabel "Sliding Window Size" offset 0, -1
set ylabel "Inference Rate (%)" offset 1.5, 0
#set auto x

plot 'comparison' \
		 using ($2*100):xtic(1) with histogram ls 1 title "User004", \
''		 using ($3*100):xtic(1) with histogram ls 2 title "User007", \
''		 using ($4*100):xtic(1) with histogram ls 3 title "User012", \
''		 using ($5*100):xtic(1) with histogram ls 4 title "User013", \
''		 using ($6*100):xtic(1) with histogram ls 5 title "User015", \
''		 using ($7*100):xtic(1) with histogram ls 6 title "User028"

set size 1.2, 0.5
set origin 0, 0.4
set yrange [0:90]
set xlabel "Attack Instance" offset 0, 0
set ylabel "Inference Accuracy (%)" offset 1.5, 0
set ytics 15
unset key

plot 'comparison' \
		 using ($8*100):xtic(1) with histogram ls 1 title "User004", \
''		 using ($9*100):xtic(1) with histogram ls 2 title "User007", \
''		 using ($10*100):xtic(1) with histogram ls 3 title "User012", \
''		 using ($11*100):xtic(1) with histogram ls 4 title "User013", \
''		 using ($12*100):xtic(1) with histogram ls 5 title "User015", \
''		 using ($13*100):xtic(1) with histogram ls 6 title "User028"
unset multiplot
