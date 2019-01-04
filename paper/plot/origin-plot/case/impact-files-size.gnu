#!/usr/bin/gnuplot

set terminal postscript enhanced color font "bold, 19" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'

set size 0.85, 0.6

set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5
set style fill solid
set style histogram cluster gap 1
set style data histograms

set boxwidth 0.95
#set xtics ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6, "128" 7, "256" 8, "512" 9, "1024" 10, "2048" 11, "4096" 12, "8192" 13, "16384" 14, "32768" 15)

set xtics nomirror
set ytics nomirror
#set key  maxrows 1 at screen 0.6, screen 0.55#for example
#set key  maxrows 1 at screen 0.75, screen 0.5
set key top left
set ytics 20
set xrange [-0.5:4.5]
set yrange [0:80]
#set xlabel "File Types" offset 0, -1
set ylabel "Rate (%)" offset 1.5, 0
#set auto x

plot 'impact-files' \
		 using ($2*100):xtic(1) with histogram ls 1 title "Raw Corruption", \
''		 using ($3*100):xtic(1) with histogram ls 2 title "Unrecovery", \
