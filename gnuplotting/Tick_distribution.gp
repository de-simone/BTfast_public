# Run from command line: gnuplot < 'gnuplotting/Tick_distribution.gp'
# or using script: ./bin/PlotTickdistribution

inputfile = "Results/profits.csv"    # (match 'profit_file' in main)
# Trade    Entry Date    DOW     Exit Date    Qty    Ticks    MAE    MFE    PL    Equity
outputfile = "Results/Tick_distribution.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 640,480
set output outputfile

#set title "Profit / Loss distribution"
set xlabel "Ticks"
set ylabel "Number of Trades"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc 'orange'
set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'
set yzeroaxis ls 2      # Add a vertical dotted line at x=0

set grid ytics
set yrange [0:]

# Histrogram settings
bin_width = 50    # bin size in ticks
bin(x)= bin_width * floor(x/bin_width) + bin_width/2.0
set boxwidth bin_width * 0.8   # box width is 80% of bin width

# Statistics
#stats datafile using 6 nooutput
#set label sprintf(" Median = %5.1f", STATS_median) right at graph 0.95,0.95 front
#set label sprintf("   Avg = %5.1f", STATS_mean) right at graph 0.95,0.90 front
#set label sprintf("   Std = %5.1f", STATS_stddev) right at graph 0.95,0.85 front

plot datafile using (bin($6)):(1.0) smooth freq with boxes lt 1

print "Tick distribution plotted on file: ", outputfile
