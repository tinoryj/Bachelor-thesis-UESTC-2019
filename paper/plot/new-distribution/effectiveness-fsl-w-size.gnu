#!/usr/bin/gnuplot

set terminal pdfcairo enhanced color font "Songti SC,19" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'



set style line 1 pt 2 lt 1 lw 3 lc rgb "#e41a1c" ps 2
set style line 2 pt 3 lt 1 lw 3 lc rgb "#377eb8" ps 2
set style line 3 pt 4 lt 1 lw 3 lc rgb "#4daf4a" ps 2
set style line 4 pt 5 lt 1 lw 3 lc rgb "#984ea3" ps 2
set style line 5 pt 6 lt 1 lw 3 lc rgb "#ff7f00" ps 2
set style line 6 pt 7 lt 1 lw 3 lc rgb "#a65628" ps 2

set style fill pattern
set style histogram cluster gap 1
set style data histograms

set boxwidth 0.92
#set xtics ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6, "128" 7, "256" 8, "512" 9, "1024" 10, "2048" 11, "4096" 12, "8192" 13, "16384" 14, "32768" 15)

set size 0.9, 0.8
set multiplot layout 2,1 rowsfirst
set size 0.9, 0.4
set origin 0, 0.4
set xtics nomirror
set ytics nomirror
#set key  maxrows 1 at screen 1.05, 0.5#for example
#set key  samplen 1.5
set ytics 5
set xrange [-0.5:5.5]
set yrange [0:25]
set xtics font ", 18"
#set xlabel "Sliding Window Size" offset 0, -1
set ylabel "推理率 (%)" offset 1.5, 0
#set auto x

plot 'effectiveness-fsl-with-size' \
		 using ($2*100):xtic(1) with histogram ls 1 fill pattern 6 notitle, \
''		 using ($3*100):xtic(1) with histogram ls 2 fill pattern 2 notitle, \
''		 using ($4*100):xtic(1) with histogram ls 3 fill pattern 4 notitle 

set size 0.9, 0.4
set origin 0, 0
set ytics 20
set yrange [0:100]
#set xlabel "Sliding Window Size" offset 0, -1
set ylabel "推理精度 (%)" offset 1.5, 0
set ytics 20
unset key

plot 'effectiveness-fsl-with-size' \
		 using ($5*100):xtic(1) with histogram ls 1  fill pattern 6 notitle, \
''		 using ($6*100):xtic(1) with histogram ls 2  fill pattern 2 notitle, \
''		 using ($7*100):xtic(1) with histogram ls 3  fill pattern 4 notitle
unset multiplot
