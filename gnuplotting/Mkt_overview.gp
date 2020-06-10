# Run from command line: gnuplot < 'gnuplotting/Mkt_overview.gp'
# or using script: ./bin/PlotMktOverview

inputfile = "Results/mkt_overview.csv"    # (match 'overview_file' in main)
outputfile = "Results/Mkt_overview.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 1000,600
set output outputfile


set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc rgb '#3399ff' # dodger blue
set linetype 2 lc rgb '#00331A' # dark green
set linetype 3 lc rgb '#FFA500' # orange
#set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'

set grid ytics

# Histrogram settings
bin(x,width)= width * floor(x/width) + width/2.0
set style fill transparent solid 0.8 border rgb '#696969'

# Statistics
stats datafile index 2 using 1 name "AvgTicks" nooutput


### Start multiplot
set multiplot layout 2,2 title \
    sprintf("Market Overview Features") font "Helvetica, 16"


# Plot (1,1): Volume per hour

set xlabel "Hour"
set ylabel "Volume"
set xrange [-0.5:23.5]
set xtics 0,1,23
bin_width = 1                   # bin size in ticks
set boxwidth bin_width * 0.8    # box width is 80% of bin width
plot datafile index 0 using 1:2  smooth unique with boxes lt 1


# Plot (1,2): Close-Open range per DOW

set xlabel "Day of Week"
set ylabel "Sum of Close-Open (ticks)"
set xrange [0.5:7.5]
set xtics ("Mon" 1, "Tue" 2, "Wed" 3, "Thu" 4, "Fri" 5, "Sat" 6, "Sun" 7)
bin_width = 1                   # bin size in ticks
set boxwidth bin_width * 0.8    # box width is 80% of bin width
plot datafile index 1 using 1:2 smooth unique with boxes lt 2

# Plot (2,1): High-Low daily range

unset xrange
unset ylabel
set xtics auto
set xlabel "High-Low range (ticks)"
bin_width = 50                   # bin size in ticks
set boxwidth bin_width * 0.8    # box width is 80% of bin width
unset label
set label sprintf("   Median = %5.0f", AvgTicks_median) left at graph 0.7,0.90 front
plot datafile index 2 using (bin($1, bin_width)):(1.0) smooth freq with boxes lt 3

print "Market overview plotted on file: ", outputfile
