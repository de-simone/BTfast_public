### Path to main directory
MAINDIR 	:= $(PWD)

### Directories containing source and strategy files
SRCDIR 	   	:= $(MAINDIR)/src
STRATDIR   	:= $(MAINDIR)/Strategies
INCLUDEDIR 	:= -I$(MAINDIR)/include


### Location of source and strategy files
SRCFILES   	:= $(SRCDIR)/*.cpp $(STRATDIR)/*.cpp $(STRATDIR)/filters/*.cpp

### C++ compiler to use
CC         	:= g++ # standard C++ compiler

### Compile-time options
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)	#- Mac OS -#
	
	CFLAGS 	:= -std=c++17 -Wall -Xpreprocessor -fopenmp -lomp -lsqlite3
			#   -lm -lcurl  
endif

ifeq ($(UNAME),Linux)		#- Linux -#
	
	CFLAGS 	:= -std=c++17 -fopenmp -lsqlite3 
	LIBDIR	:= /usr/local
    	#INCLUDEDIR += -I$(LIBDIR)/include 			# path to sqlite.h	
	#INCLUDEDIR += -L$(LIBDIR)/lib				# path to libsqlite3.so (<- should have this name)

	INCLUDEDIR += -I/home/andrea/anaconda3/include 	# path to sqlite.h
	INCLUDEDIR += -L/home/andrea/anaconda3/lib # path to libsqlite3.21.0.so
endif

CFLAGSPROF	:= -lprofiler -ltcmalloc 
#CFLAGSSO 	:= -std=c++17 -lm -lcurl -lsqlite3 -fPIC -shared # for .so generation


### Name of executable output files 
OUTPUT 		:= $(MAINDIR)/bin/BTfast.o
#OUTPUTSO 	:= $(MAINDIR)/bin/BTfast.so	

### Enable extended globbing
#shopt -s extglob


### Create executables 

all: 		# compile everything
	
	$(CC) $(CFLAGS) $(INCLUDEDIR) $(SRCFILES) -o $(OUTPUT) 
	@echo ">>>" BTfast compiled successfully. Run with: ./run
	@echo


debug:  	# compile for debugging (lldb)

	$(CC) $(CFLAGS) -g $(INCLUDEDIR) $(SRCFILES) -o $(OUTPUT) 
	@echo ">>>" BTfast compiled successfully for profiling.
	@echo

profile:  	# compile for profiling (gperftools)

	$(CC) $(CFLAGS) $(CFLAGSPROF) $(INCLUDEDIR) $(SRCFILES) -o $(OUTPUT) 
	@echo ">>>" BTfast compiled successfully for profiling.
	@echo

so:		# generate .so library file

	$(CC) $(CFLAGSSO) -I$(INCLUDEDIR) $(SRCFILES) -o $(OUTPUTSO) 
	@echo BTfast shared library created successfully
	@echo 

clean:		# remove all outputs

	rm $(OUTPUT)
	#rm $(OUTPUTSO)	
	@echo BTfast executables removed successfully	
	@echo 	
