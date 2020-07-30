# Run from command line: gnuplot < 'gnuplotting/Month_ticks.gp'
# or using script: ./bin/PlotMonthTicks

inputfile = "Results/profits.csv"    # (match 'profit_file' in main)
# Trade    Entry Date    DOW     Exit Date    Qty    Ticks    MAE    MFE    PL    Equity
outputfile = "Results/Month_ticks.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 640,480
set output outputfile


set xlabel "Month"
set ylabel "Avg Ticks"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc rgb '#b7410e' # dark orange
set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'

set grid ytics
set xtics ("Jan" 0, "Feb" 1, "Mar" 2, "Apr" 3, "May" 4, "Jun" 5, \
            "Jul" 6, "Aug" 7, "Sep" 8, "Oct" 9, "Nov" 10, "Dec" 11)
set yrange [-10:50]


# Histrogram settings
bin_width = 1    # bin size
set boxwidth bin_width * 0.8   # box width is 80% of bin width

plot datafile using (tm_mon(strptime('%Y-%m-%d', strcol(4)))):6 skip 2 \
            smooth unique with boxes lt 1

print "Year ticks distribution plotted on file: ", outputfile
