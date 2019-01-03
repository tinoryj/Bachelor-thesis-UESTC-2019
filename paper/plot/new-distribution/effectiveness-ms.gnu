#!/usr/bin/gnuplot

set terminal pdfcairo enhanced color font "Songti SC,19" 
set output 'dis-effect-ms.pdf'

set style line 1 pt 7 lt 1 lw 4 lc rgb "#e41a1c" ps 1.5
set style line 2 pt 11 lt 1 lw 4 lc rgb "#377eb8" ps 1.5
set style line 3 pt 9 lt 1 lw 4 lc rgb "#4daf4a" ps 1.5
set style line 4 pt 8 lt 1 lw 4 lc rgb "#984ea3" ps 1.5
set style line 5 pt 13 lt 1 lw 4 lc rgb "#ff7f00" ps 1.5
set style line 6 pt 12 lt 1 lw 4 lc rgb "#a65628" ps 1.5

set style fill solid
set boxwidth 0.65

set xtics nomirror
set ytics nomirror

#set key above

#set xtics ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6, "128" 7, "256" 8, "512" 9, "1024" 10, "2048" 11, "4096" 12, "8192" 13, "16384" 14, "32768" 15)

set size 0.8, 1
set multiplot layout 1,2 rowsfirst
set size 0.4, 0.5
set origin 0, 0
set xtics nomirror
set ytics nomirror
set key  maxrows 2 at screen 0.7, screen 0.55#for example
set ytics 5
set xrange [0:6]
set yrange [0:25]
set xlabel "Snapshot" offset 0, 0.25
set ylabel "推理率 (%)" offset 1.5, 0
unset xtics


plot 'effectiveness-ms' \
		 every ::0::0 using 1:($3*100) with boxes ls 1 title "Win7", \
''		 every ::1::1 using 1:($3*100) with boxes ls 2 title "Serv-03", \
''		 every ::2::2 using 1:($3*100) with boxes ls 3 title "Serv-08", \
''		 every ::3::3 using 1:($3*100) with boxes ls 4 title "Vista-B", \
''		 every ::4::4 using 1:($3*100) with boxes ls 5 title "Vista-U"

set size 0.4, 0.5
set origin 0.38, 0
set ytics 20
set yrange [0:100]
set xlabel "Snapshot" offset 0, 0.25
set ylabel "推理精度 (%)" offset 1.5, 0
set ytics 20
unset key
plot 'effectiveness-ms' \
		 every ::0::0 using 1:($4*100) with boxes ls 1 notitle, \
''		 every ::1::1 using 1:($4*100) with boxes ls 2 notitle, \
''		 every ::2::2 using 1:($4*100) with boxes ls 3 notitle, \
''		 every ::3::3 using 1:($4*100) with boxes ls 4 notitle, \
''		 every ::4::4 using 1:($4*100) with boxes ls 5 notitle 
unset multiplot
