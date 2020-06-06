# Run from command line: gnuplot < 'gnuplotting/DOW_ticks.gp'
# or using script: ./bin/PlotDOWTicks

inputfile = "Results/profits.csv"    # (match 'profit_file' in main)
# Trade,    Entry Date,    DOW,    Exit Date,   Qty, Ticks,  PL,    Equity
outputfile = "Results/DOW_ticks.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 640,480
set output outputfile

#set title "Day of Week"
set xlabel "Day of Week"
set ylabel "Avg Ticks"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc 'blue'
set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'

set grid ytics
set xtics ("Mon" 1, "Tue" 2, "Wed" 3, "Thu" 4, "Fri" 5)
set xrange [0.5:5.5]
set yrange [-10:30]

# Histrogram settings
bin_width = 1    # bin size
bin(x)= bin_width * floor(x/bin_width) + bin_width/2.0
set boxwidth bin_width * 0.8   # box width is 80% of bin width


plot datafile using 3:6  smooth unique with boxes lt 1
#plot datafile using 3:6  smooth freq with boxes lt 1

print "DOW ticks distribution plotted on file: ", outputfile
