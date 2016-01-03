#***********************************************#
# Define macro "PRINTING" as follows:		#
# NONE    - printing is disabled		#
# COMPACT - for basic information printing	#
# FULL    - thorough printing			#
#***********************************************#
CC	= g++

DATE	= `date +%d%m%Y-%H%M`

#MACROFLAGS += -DNONE=0 -DCOMPACT=1 -DFULL=2
#MACROFLAGS += -DPRINTING=FULL

ifeq ($(CC),icc)
	CFLAGS	= -O3
else ifeq ($(CC),g++)
	CFLAGS	= -O3 -Wno-unused-result
endif

#CFLAGS	+= $(MACROFLAGS)
CFLAGS	+= -fopenmp
#CFLAGS	+= -g
CFLAGS	+= -std=c++0x
CFLAGS	+= -static
#CFLAGS	+= -pg
#CFLAGS	+= -Wall

ROOT		= $(CURDIR)
SRCDIR		= $(ROOT)/src
LIBDIR		= $(SRCDIR)/liberty_parse-2.6/lib/
OBJDIR		= $(ROOT)/obj

INCLUDE		= -I $(SRCDIR)/liberty_parse-2.6/Include/

LIBS		= -L $(LIBDIR) -lm -lsi2dr_liberty -lboost_regex

CHECK_CC := $(shell which $(CC))
ifeq ($(CHECK_CC),)
	$(error No $(CC) found!)
endif

.PHONY: all clean tar dirs

all: dirs CCSopt

dirs:
	@mkdir -p $(OBJDIR)

CCSopt: $(OBJDIR)/Cell.o $(OBJDIR)/CellEdge.o $(OBJDIR)/Cellinst.o $(OBJDIR)/Edge.o $(OBJDIR)/Graph.o $(OBJDIR)/InputPin.o $(OBJDIR)/main.o $(OBJDIR)/Module.o $(OBJDIR)/Net.o $(OBJDIR)/NetEdge.o $(OBJDIR)/Node.o $(OBJDIR)/OutputPin.o $(OBJDIR)/Parasitic.o $(OBJDIR)/Path.o $(OBJDIR)/Pin.o $(OBJDIR)/System.o $(OBJDIR)/Timing.o $(OBJDIR)/Util.o $(OBJDIR)/When.o
	$(CC) $(OBJDIR)/Cell.o $(OBJDIR)/CellEdge.o $(OBJDIR)/Cellinst.o $(OBJDIR)/Edge.o $(OBJDIR)/Graph.o $(OBJDIR)/InputPin.o $(OBJDIR)/main.o $(OBJDIR)/Module.o $(OBJDIR)/Net.o $(OBJDIR)/NetEdge.o $(OBJDIR)/Node.o $(OBJDIR)/OutputPin.o $(OBJDIR)/Parasitic.o $(OBJDIR)/Path.o $(OBJDIR)/Pin.o $(OBJDIR)/System.o $(OBJDIR)/Timing.o $(OBJDIR)/Util.o $(OBJDIR)/When.o $(CFLAGS) -o CCSopt $(INCLUDE) $(LIBS)

$(OBJDIR)/Cell.o: $(SRCDIR)/Cell.cpp $(SRCDIR)/Cell.hpp $(SRCDIR)/InputPin.hpp $(SRCDIR)/OutputPin.hpp $(SRCDIR)/Timing.hpp $(SRCDIR)/types.hpp            
	$(CC) -c $(SRCDIR)/Cell.cpp $(CFLAGS) -o $(OBJDIR)/Cell.o $(INCLUDE) $(LIBS) 
	
$(OBJDIR)/CellEdge.o: $(SRCDIR)/CellEdge.cpp $(SRCDIR)/CellEdge.hpp $(SRCDIR)/Edge.hpp
	$(CC) -c $(SRCDIR)/CellEdge.cpp $(CFLAGS) -o $(OBJDIR)/CellEdge.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Cellinst.o: $(SRCDIR)/Cellinst.cpp $(SRCDIR)/Cellinst.hpp $(SRCDIR)/CellEdge.hpp          
	$(CC) -c $(SRCDIR)/Cellinst.cpp $(CFLAGS) -o $(OBJDIR)/Cellinst.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Edge.o: $(SRCDIR)/Edge.cpp $(SRCDIR)/Edge.hpp              
	$(CC) -c $(SRCDIR)/Edge.cpp $(CFLAGS) -o $(OBJDIR)/Edge.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Graph.o: $(SRCDIR)/Graph.cpp $(SRCDIR)/Graph.hpp $(SRCDIR)/Timing.hpp $(SRCDIR)/OutputPin.hpp $(SRCDIR)/InputPin.hpp $(SRCDIR)/System.hpp $(SRCDIR)/Util.hpp $(SRCDIR)/NetEdge.hpp $(SRCDIR)/CellEdge.hpp $(SRCDIR)/types.hpp
	$(CC) -c $(SRCDIR)/Graph.cpp $(CFLAGS) -o $(OBJDIR)/Graph.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/InputPin.o: $(SRCDIR)/InputPin.cpp $(SRCDIR)/InputPin.hpp           
	$(CC) -c $(SRCDIR)/InputPin.cpp $(CFLAGS) -o $(OBJDIR)/InputPin.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp $(SRCDIR)/Graph.hpp $(SRCDIR)/types.hpp $(SRCDIR)/System.hpp            
	$(CC) -c $(SRCDIR)/main.cpp $(CFLAGS) -o $(OBJDIR)/main.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Module.o: $(SRCDIR)/Module.cpp $(SRCDIR)/Module.hpp             
	$(CC) -c $(SRCDIR)/Module.cpp $(CFLAGS) -o $(OBJDIR)/Module.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Net.o: $(SRCDIR)/Net.cpp $(SRCDIR)/Net.hpp            
	$(CC) -c $(SRCDIR)/Net.cpp $(CFLAGS) -o $(OBJDIR)/Net.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/NetEdge.o: $(SRCDIR)/NetEdge.cpp $(SRCDIR)/NetEdge.hpp $(SRCDIR)/InputPin.hpp $(SRCDIR)/Node.hpp $(SRCDIR)/Edge.hpp      
	$(CC) -c $(SRCDIR)/NetEdge.cpp $(CFLAGS) -o $(OBJDIR)/NetEdge.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Node.o: $(SRCDIR)/Node.cpp $(SRCDIR)/Node.hpp 
	$(CC) -c $(SRCDIR)/Node.cpp $(CFLAGS) -o $(OBJDIR)/Node.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/OutputPin.o: $(SRCDIR)/OutputPin.cpp $(SRCDIR)/OutputPin.hpp $(SRCDIR)/types.hpp $(SRCDIR)/Timing.hpp           
	$(CC) -c $(SRCDIR)/OutputPin.cpp $(CFLAGS) -o $(OBJDIR)/OutputPin.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Parasitic.o: $(SRCDIR)/Parasitic.cpp $(SRCDIR)/Parasitic.hpp $(SRCDIR)/NetEdge.hpp $(SRCDIR)/InputPin.hpp $(SRCDIR)/types.hpp
	$(CC) -c $(SRCDIR)/Parasitic.cpp $(CFLAGS) -o $(OBJDIR)/Parasitic.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Path.o: $(SRCDIR)/Path.cpp $(SRCDIR)/Path.hpp $(SRCDIR)/OutputPin.hpp $(SRCDIR)/InputPin.hpp $(SRCDIR)/types.hpp $(SRCDIR)/NetEdge.hpp $(SRCDIR)/Util.hpp $(SRCDIR)/Net.hpp
	$(CC) -c $(SRCDIR)/Path.cpp $(CFLAGS) -o $(OBJDIR)/Path.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Pin.o: $(SRCDIR)/Pin.cpp $(SRCDIR)/Pin.hpp           
	$(CC) -c $(SRCDIR)/Pin.cpp $(CFLAGS) -o $(OBJDIR)/Pin.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/System.o: $(SRCDIR)/System.cpp $(SRCDIR)/System.hpp $(SRCDIR)/CellEdge.hpp $(SRCDIR)/NetEdge.hpp $(SRCDIR)/Node.hpp $(SRCDIR)/Util.hpp
	$(CC) -c $(SRCDIR)/System.cpp $(CFLAGS) -o $(OBJDIR)/System.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Timing.o: $(SRCDIR)/Timing.cpp $(SRCDIR)/Timing.hpp            
	$(CC) -c $(SRCDIR)/Timing.cpp $(CFLAGS) -o $(OBJDIR)/Timing.o $(INCLUDE) $(LIBS) 

$(OBJDIR)/Util.o: $(SRCDIR)/Util.cpp $(SRCDIR)/Util.hpp $(SRCDIR)/InputPin.hpp $(SRCDIR)/Edge.hpp $(SRCDIR)/NetEdge.hpp $(SRCDIR)/types.hpp
	$(CC) -c $(SRCDIR)/Util.cpp $(CFLAGS) -o $(OBJDIR)/Util.o $(INCLUDE) $(LIBS)

$(OBJDIR)/When.o: $(SRCDIR)/When.cpp $(SRCDIR)/When.hpp           
	$(CC) -c $(SRCDIR)/When.cpp $(CFLAGS) -o $(OBJDIR)/When.o $(INCLUDE) $(LIBS)

clean:
	@\rm -rf *~ $(SRCDIR)/*~ $(LIBDIR)/*~ $(OBJDIR) CCSopt

tar:
	@tar -czf CCSopt_$(DATE).tar.gz src Makefile README
	@echo "Tarball created: CCSopt_$(DATE).tar.gz"

