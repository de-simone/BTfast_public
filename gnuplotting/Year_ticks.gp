# Run from command line: gnuplot < 'gnuplotting/Year_ticks.gp'
# or using script: ./bin/PlotYearTicks

inputfile = "Results/profits.csv"    # (match 'profit_file' in main)
# Trade    Entry Date    DOW     Exit Date    Qty    Ticks    MAE    MFE    PL    Equity
outputfile = "Results/Year_ticks.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 640,480
set output outputfile

#set title "Day of Week"
set xlabel "Year"
set ylabel "Avg Ticks"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc rgb '#472c7a' # purple
set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'

set grid ytics
#set xtics ("Mon" 1, "Tue" 2, "Wed" 3, "Thu" 4, "Fri" 5)
set yrange [-10:50]

# Histrogram settings
bin_width = 1    # bin size
set boxwidth bin_width * 0.8   # box width is 80% of bin width

plot datafile using (tm_year(strptime('%Y-%m-%d', strcol(4)))):6 skip 2 \
            smooth unique with boxes lt 1

print "Year ticks distribution plotted on file: ", outputfile
