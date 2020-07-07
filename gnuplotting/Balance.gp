# Run from command line: gnuplot < 'gnuplotting/Balance.gp'
# or using script: ./bin/PlotBalance

inputfile = "Results/profits.csv"    # (match 'profit_file' in main)
# Trade    Entry Date    DOW     Exit Date    Qty    Ticks    MAE    MFE    PL    Equity
outputfile = "Results/equity_line.png"


reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,14" size 640,480
set output outputfile

set grid ytics
set style line 1 lw 1 linecolor 'blue' # '#0000FF'
#set style line 2 lw 1 linecolor '#DC143C' # crimson
#set style line 2 lw 1.5 dt '-'  linecolor '#696969'

#set title "Account Balance"
set xlabel "Trade Number"
set ylabel "Equity (USD) "
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set ytics nomirror font "Helvetica, 12"
set y2tics nomirror font "Helvetica, 12"

#set style fill transparent solid 0.1

plot \
    datafile using 1:10 with lines ls 1
    #datafile using 1:($0 == 0 ? InitialValue = $2 : InitialValue) with lines ls 2

print "Equity line plotted on file: ", outputfile
