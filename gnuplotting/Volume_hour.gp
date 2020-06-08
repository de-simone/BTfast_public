# Run from command line: gnuplot < 'gnuplotting/Volume_hour.gp'
# or using script: ./bin/PlotVolumeHour

inputfile = "Results/overview_volume.csv"    # (match in btfast_overview.cpp)
# Hour,   Volume
outputfile = "Results/volume_hour.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 640,480
set output outputfile

#set title "Day of Week"
set xlabel "Hour"
set ylabel "Volume"
set label "[BTfast]" right at screen 0.98, 0.02 font "Courier,10"

set style fill solid 1.0 noborder
set linetype 1 lc rgb '#3399ff' # dodger blue
set style line 2 lw 1.5 dt '-'  linecolor rgb '#696969'

set grid ytics
set xrange [-0.5:23.5]
set xtics 0,1,23

# Histrogram settings
bin_width = 1    # bin size
bin(x)= bin_width * floor(x/bin_width) + bin_width/2.0
set boxwidth bin_width * 0.8   # box width is 80% of bin width

plot datafile using 1:2  smooth unique with boxes lt 1

print "Volume per hour plotted on file: ", outputfile
