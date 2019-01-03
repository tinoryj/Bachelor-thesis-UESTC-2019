#!/usr/bin/gnuplot

set terminal pdfcairo enhanced color font "Songti SC,19" 
set output 'dis-effect-fsl.pdf'

set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5
set style fill solid
set style histogram cluster gap 1
set style data histograms

set boxwidth 0.92
#set xtics ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6, "128" 7, "256" 8, "512" 9, "1024" 10, "2048" 11, "4096" 12, "8192" 13, "16384" 14, "32768" 15)

set size 1.7, 1
set multiplot layout 1,2 rowsfirst
set size 0.85, 0.5
set origin 0, 0
set xtics nomirror
set ytics nomirror
set key  maxrows 1 at screen 1.05, screen 0.55#for example
set ytics 5
set xrange [-0.5:5.5]
set yrange [0:29]
#set xlabel "Sliding Window Size" offset 0, -1
set ylabel "推理率 (%)" offset 1.5, 0
#set auto x

plot 'effectiveness-fsl-without-size' \
		 using ($2*100):xtic(1) with histogram ls 1 title "w=1", \
''		 using ($3*100):xtic(1) with histogram ls 2 title "w=2", \
''		 using ($4*100):xtic(1) with histogram ls 3 title "w=3"

set size 0.85, 0.5
set origin 0.83, 0
set ytics 15
set yrange [0:100]
#set xlabel "Sliding Window Size" offset 0, -1
set ylabel "推理精度 (%)" offset 1.5, 0
set ytics 20
unset key

plot 'effectiveness-fsl-without-size' \
		 using ($5*100):xtic(1) with histogram ls 1 title "w=1", \
''		 using ($6*100):xtic(1) with histogram ls 2 title "w=2", \
''		 using ($7*100):xtic(1) with histogram ls 3 title "w=3"
unset multiplot
