### Path to main directory
MAINDIR 	:= /Users/Andrea/Dropbox/Trading/BTfast

### Directories containing source and strategy files
SRCDIR 	   	:= $(MAINDIR)/src
STRATDIR   	:= $(MAINDIR)/Strategies
INCLUDEDIR 	:= -I$(MAINDIR)/include -I$(STRATDIR)/indicators


### Location of source and strategy files
SRCFILES   	:= IndicatorTest.cpp $(SRCDIR)/events.cpp $(SRCDIR)/instruments.cpp \
		   $(SRCDIR)/datetime.cpp $(SRCDIR)/utils_math.cpp \
		$(STRATDIR)/indicators/TA_indicators.cpp

### Name of executable output files 
OUTPUT 		:= $(MAINDIR)/indi_test/IndicatorTest.o

### C++ compiler to use
CC         	:= g++ # standard C++ compiler

CFLAGS 		:= -std=c++17 -Wall 


### Create executables 

all: 		# compile everything
	
	$(CC) $(CFLAGS) $(INCLUDEDIR) $(SRCFILES) -o $(OUTPUT) 
	@echo ">>>" IndicatorTest compiled successfully. Run with: ./IndicatorTest.o
	@echo

