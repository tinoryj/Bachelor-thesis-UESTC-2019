#!/usr/bin/gnuplot

set terminal pdfcairo enhanced color font "Songti SC,24" 
set output 'clu-impact-d-fsl-cro.pdf'
set style line 1 pt 2 lt 1 lw 3 lc rgb "#e41a1c" ps 1
set style line 2 pt 3 lt 1 lw 3 lc rgb "#377eb8" ps 1
set style line 3 pt 4 lt 1 lw 3 lc rgb "#4daf4a" ps 1
set style line 4 pt 5 lt 1 lw 3 lc rgb "#984ea3" ps 1
set style line 5 pt 6 lt 1 lw 3 lc rgb "#ff7f00" ps 1
set style line 6 pt 7 lt 1 lw 3 lc rgb "#a65628" ps 1

set autoscale
set xtics nomirror
set ytics nomirror
set ytics 20
set yrange [0:100]
set xrange [0.35:1.05]
set xlabel "k" #offset 0, 0.5
set ylabel "聚类正确性 (%)" #offset 1.5, 0

unset key
plot 'impact-d-fsl' \
	using 1:($3)*100 w lp ls 1 notitle, \
"" using 1:($5)*100 w lp ls 2 notitle, \
"" using 1:($7)*100 w lp ls 5 notitle, \
"" using 1:($9)*100 w lp ls 6 notitle
