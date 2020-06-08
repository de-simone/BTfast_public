# Run from command line: gnuplot < 'gnuplotting/Bars_DOW.gp'
# or using script: ./bin/PlotBarsDOW

inputfile = "Results/overview_bars.csv"    # (match in btfast_overview.cpp)
# Hour,   Volume
outputfile = "Results/Bars_DOW.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 640,480
set output outputfile

#set title "Day of Week"
set xlabel "Day of Week"
set ylabel "Number of Up Bars - Number Down Bars"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc rgb '#00331A' # dark green
set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'

set grid ytics
set xrange [0.5:7.5]
set xtics ("Mon" 1, "Tue" 2, "Wed" 3, "Thu" 4, "Fri" 5, "Sat" 6, "Sun" 7)


# Histrogram settings
bin_width = 1    # bin size
bin(x)= bin_width * floor(x/bin_width) + bin_width/2.0
set boxwidth bin_width * 0.8   # box width is 80% of bin width

plot datafile using 1:2  smooth unique with boxes lt 1

print "Bars per DOW plotted on file: ", outputfile
