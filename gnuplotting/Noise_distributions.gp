# Run from command line: gnuplot < 'gnuplotting/Noise_distributions.gp'
# or using script: ./bin/PlotNoiseDistributions

inputfile = "Results/noise.csv"    # (match 'noise_file' in main)
#  Ntrades, AvgTicks, AvgTrade, PftFactor, WinPerc, NP/MDD, Expectancy, parameters...
outputfile = "Results/noise_distributions.png"

reset
unset key

datafile = inputfile
set datafile separator ","  # CSV input file

set term pngcairo font "Helvetica,12" size 1000,600
set output outputfile

set style fill solid 1.0 noborder
set linetype 1 lc 'blue'
set style line 2 lw 2.5 dt '-' linecolor rgb '#696969'
set style line 3 lw 2.5  linecolor 'red' #test

set grid ytics

# Histrogram settings
bin(x,width)= width * floor(x/width) + width/2.0
set style fill transparent solid 0.5 border rgb '#696969'

# Statistics
stats datafile using 2 name "AvgTicks" nooutput
stats datafile using 3 name "AvgTrade" nooutput
stats datafile using 4 name "PftFactor" nooutput
stats datafile using 5 name "NPMDD" nooutput
stats datafile using 6 name "Expectancy" nooutput
stats datafile using 7 name "Zscore" nooutput

### Metrics of strategy on original data (first row of datafile)
set table '/dev/null'
plot datafile using 1:($0==0 ? (OriginalAvgTicks=$2, OriginalAvgTrade=$3,\
                                OriginalPftFactor=$4, OriginalNPMDD=$5, \
                                OriginalExpectancy=$6, OriginalZscore=$7) : 1/0)
unset table

### Start multiplot
set multiplot layout 2,3 title \
    sprintf("Distributions of performance metrics (%d randomizations)", \
            AvgTicks_records) font "Helvetica, 16"


# Plot (1,1): Avg Ticks

#set title "Avg Ticks  distribution"
set xlabel "Avg Ticks"
#set xtics AvgTicks_min,(AvgTicks_max-AvgTicks_min)/5  # 6 tics

bin_width = 1                   # bin size in ticks
set boxwidth bin_width * 0.8    # box width is 80% of bin width
unset label
set label "[BTfast]" right at screen 0.995, 0.99 font "Courier,10"
set label sprintf(" Median = %5.0f", AvgTicks_median) left at graph 0,0.95 front
set label sprintf("   Avg = %5.0f", AvgTicks_mean) left at graph 0,0.90 front
set label sprintf("   Std = %5.0f", AvgTicks_stddev) left at graph 0,0.85 front
set label sprintf("   Min = %5.0f", AvgTicks_min) left at graph 0,0.80 front
set arrow from OriginalAvgTicks,graph 0 to OriginalAvgTicks,graph 1 nohead ls 2 front
plot datafile using (bin($2, bin_width)):(1.0) smooth freq with boxes lc 'blue'
unset arrow
set xtics auto  # restore automatic xtics


# Plot (1,2): Average Trade

set xlabel "Avg Trade (USD)"
#set yrange [0:]

bin_width = 10                  # bin size in USD
set boxwidth bin_width * 0.8    # box width is 80% of bin width
unset label
set label sprintf(" Median = %5.0f $", AvgTrade_median) left at graph 0,0.95 front
set label sprintf("    Avg = %5.0f $", AvgTrade_mean) left at graph 0,0.90 front
set label sprintf("    Std = %5.0f $", AvgTrade_stddev) left at graph 0,0.85 front
set label sprintf("    Min = %5.0f $", AvgTrade_min) left at graph 0,0.80 front
set arrow from OriginalAvgTrade,graph 0 to OriginalAvgTrade,graph 1 nohead ls 2 front
plot datafile using (bin($3, bin_width)):(1.0) smooth freq with boxes lc 'green'
unset arrow


# Plot (1,3): Profit Factor

set xlabel "Profit Factor"
bin_width = 0.1                # bin size
set boxwidth bin_width * 0.8    # box width is 80% of bin width
unset label
set label sprintf(" Median = %5.2f", PftFactor_median) left at graph 0,0.95 front
set label sprintf("    Avg = %5.2f", PftFactor_mean) left at graph 0,0.90 front
set label sprintf("    Std = %5.2f", PftFactor_stddev) left at graph 0,0.85 front
set label sprintf("    Min = %5.2f", PftFactor_min) left at graph 0,0.80 front
set arrow from OriginalPftFactor,graph 0 to OriginalPftFactor,graph 1 nohead ls 2 front
plot datafile using (bin($4, bin_width)):(1.0) smooth freq with boxes lc rgb '#f8c932' # yellow-orange
unset arrow


# Plot (2,1):

set xlabel "Net PL/Max DD"
bin_width = 1                   # bin size
set boxwidth bin_width * 0.8    # box width is 80% of bin width
unset label
set label sprintf(" Median = %5.1f", NPMDD_median) left at graph 0,0.95 front
set label sprintf("    Avg = %5.1f", NPMDD_mean) left at graph 0,0.90 front
set label sprintf("    Std = %5.1f", NPMDD_stddev) left at graph 0,0.85 front
set label sprintf("    Min = %5.1f", NPMDD_min) left at graph 0,0.80 front
set arrow from OriginalNPMDD,graph 0 to OriginalNPMDD,graph 1 nohead ls 2 front
plot datafile using (bin($5, bin_width)):(1.0) smooth freq with boxes lc rgb '#0072bd' # blue
unset arrow


# Plot (2,2): Expectancy

set xlabel "Expectancy"
bin_width = 0.1                  # bin size
set boxwidth bin_width * 0.8      # box width is 80% of bin width
unset label
set label sprintf(" Median = %5.2f", Expectancy_median) left at graph 0,0.95 front
set label sprintf("    Avg = %5.2f", Expectancy_mean) left at graph 0,0.90 front
set label sprintf("    Std = %5.2f", Expectancy_stddev) left at graph 0,0.85 front
set label sprintf("    Min = %5.2f", Expectancy_min) left at graph 0,0.80 front
set arrow from OriginalExpectancy,graph 0 to OriginalExpectancy,graph 1 nohead ls 2 front
plot datafile using (bin($6, bin_width)):(1.0) smooth freq with boxes lc rgb '#472c7a' # purple
unset arrow

# Plot (2,3): Z-score

set xlabel "Z-score"
bin_width = 0.5                 # bin size
set boxwidth bin_width * 0.8    # box width is 80% of bin width
unset label
set label sprintf(" Median = %5.1f", Zscore_median) left at graph 0,0.95 front
set label sprintf("    Avg = %5.1f", Zscore_mean) left at graph 0,0.90 front
set label sprintf("    Std = %5.1f", Zscore_stddev) left at graph 0,0.85 front
set label sprintf("    Min = %5.1f", Zscore_min) left at graph 0,0.80 front
set arrow from OriginalZscore,graph 0 to OriginalZscore,graph 1 nohead ls 2 front
plot datafile using (bin($7, bin_width)):(1.0) smooth freq with boxes lc 'red'
unset arrow

### End Multiplot
unset multiplot

print "Distributions of performance metrics under noise test plotted on file: ",\
outputfile
