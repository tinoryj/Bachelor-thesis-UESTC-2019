#!/usr/bin/gnuplot

set terminal postscript enhanced color font ", 18" 
# set output '| epstopdf --filter --autorotate=All --outfile=out.pdf'
set output '| epstopdf --filter --autorotate=All | pdfcrop - out.pdf'

set style line 1 pt 2 lt 1 lw 2.5 lc rgb "#e41a1c" ps 2
set style line 2 pt 3 lt 1 lw 2.5 lc rgb "#377eb8" ps 2
set style line 3 pt 4 lt 1 lw 2.5 lc rgb "#4daf4a" ps 2
set style line 4 pt 5 lt 1 lw 2.5 lc rgb "#984ea3" ps 2
set style line 5 pt 6 lt 1 lw 2.5 lc rgb "#ff7f00" ps 2
set style line 6 pt 7 lt 1 lw 2.5 lc rgb "#a65628" ps 2

set size 1.4, 1.1

set style fill solid
set style histogram cluster gap 1
set style data histograms

set noborder
set noxtics
set noytics
set xrange [-10:-1]
set yrange [-10:10]  
set key maxrows 1
set key font ", 16" width -2 samplen 1.5  

plot 'comparison-fsl' \
		 using ($2*100):xtic(1) with histogram ls 1 fill pattern 6 title "Baseline", \
''		 using ($3*100):xtic(1) with histogram ls 2 fill pattern 2 title "Distribution", \
''		 using ($4*100):xtic(1) with histogram ls 3 fill pattern 4 title "Distribution^S", \
''		 using ($5*100):xtic(1) with histogram ls 4  fill pattern 1 title "Distribution-o", \
''		 using ($6*100):xtic(1) with histogram ls 5  fill pattern 7 title "Distribution^S-o"
