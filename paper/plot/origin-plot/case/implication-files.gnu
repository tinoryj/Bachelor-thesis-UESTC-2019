#!/usr/bin/gnuplot

set terminal postscript enhanced color font "bold, 14" 
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'

set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5

set style fill solid
set boxwidth 0.7

set xtics nomirror
set ytics nomirror

#set key above

#set xtics ("Office" 1, "Picture" 2, "Source" 3, "Database" 4, "Disk" 5)

set size 0.4, 0.4
set xtics nomirror
set ytics nomirror
set key top center outside
set key maxrows 2
#set key  maxrows 2 at screen 0.7, screen 0.55#for example
set ytics 10
unset xtics
set xlabel "File Categories"
set xrange [0.5:5.5]
set yrange [0:40]
set ylabel "Raw Inference Rate (%)" offset 1.5, 0
plot 'implication-files' \
		 every ::0::0 using 1:($3*100) with boxes ls 1 title "Office", \
''		 every ::1::1 using 1:($3*100) with boxes ls 2 title "Picture", \
''		 every ::2::2 using 1:($3*100) with boxes ls 3 title "Souce", \
''		 every ::3::3 using 1:($3*100) with boxes ls 4 title "Database", \
''		 every ::4::4 using 1:($3*100) with boxes ls 5 title "Disk"

