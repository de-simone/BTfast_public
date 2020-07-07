# Run from command line: gnuplot < 'gnuplotting/MaxExcursions.gp'
# or using script: ./bin/PlotMaxExcursions

inputfile = "Results/profits.csv"    # (match 'profit_file' in main)
# Trade    Entry Date    DOW     Exit Date    Qty    Ticks    MAE    MFE    PL    Equity
outputfile = "Results/max_excursions.png"


reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,14" size 1000,500
set output outputfile

set style line 1 lt 1 lw 2 pt 9 ps 0.5 linecolor rgb 'green'
set style line 2 lt 1 lw 2 pt 11 ps 0.5 linecolor rgb 'red'
set style line 3 lw 1 dt '-' linecolor rgb '#696969'
set grid

f(x) = x  # 45-degree line

### START MULTIPLOT ###
set multiplot layout 1, 2 \
            title sprintf("Maximum Excursions") font "Helvetica, 16"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

# Plot 1: MAE

unset xrange
set title "MAE"
set xlabel "Drawdown (USD)"
set ylabel "Profit/Loss (USD)"
#set xtics 200
#set ytics 200
plot datafile using 7:($9>0 ? $9 : 1/0) with points ls 1,\
datafile using 7:($9<0 ? -$9 : 1/0) with points ls 2,\
f(x) ls 3

# Plot 2: MFE

set title "MFE"
set xlabel "Run-up (USD)"
set ylabel "Profit/Loss (USD)"
#set xtics 200
#set ytics 200
plot datafile using 8:($9>0 ? $9 : 1/0) with points ls 1,\
datafile using 8:($9<0 ? -$9 : 1/0) with points ls 2,\
f(x) ls 3


print "Maximum Excursions plotted on file: ", outputfile
