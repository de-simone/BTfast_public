# BTfast
Event-driven backtesting and optimization of trading systems

This is BTfast, written by Andrea De Simone. All rights reserved. 2019-2020.

BTfast is a software written in C++ to do event-driven backtesting, optimization and validation of trading systems.

### Data Storage
 - CSV files

### System Requirements:

 - Unix OS, with bash
 - C++17 compiler
 - gnuplot, for plotting


### Use BTfast from command line (main directory):

* Compile the code by typing “make”.
  This will create the executable bin/BTfast.o

* Run BTfast by typing “./run”,
  which is a script to run bin/BTfast.o 

* To remove all output files, type “make clean”.

* To plot results, run plotting scripts in bin/

### Required files:
   - Configuration file: 'settings.xml'
   - Data file, in 'data_dir' directory
   - Strategy files: 'Strategy/strategyname.cpp',
                     'Strategy/strategyname.h',
                     'Strategy/StrategyName.xml'

### Files to Edit:
   - Input settings        (in 'settings.xml')
   - Strategy parameters   (in 'Strategies/StrategyName.xml')
