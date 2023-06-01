SYSTEM = x86-64_linux
LIBFORMAT = static_pic
CCC = g++ -O3
FLAGS = -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -g -Wall -Wextra -std=c++14 -Wno-ignored-attributes

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX 12.8 programs
# please copy this makefile and set CPLEXINSTALL to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

#CPLEXINSTALL = opt/ibm/ILOG/CPLEX_Studio1210/

#
# INCLUDE HEADERS AND LIBRARIES OF CPLEX and CONCERT DIRECTORIES 
#
#---------------------------------------
#CPLEXINC = $(CPLEXINSTALL)cplex/include/
#CONCERTINC = $(CPLEXINSTALL)concert/include/
#CPLEXLIB = $(CPLEXINSTALL)cplex/lib/$(SYSTEM)/$(LIBFORMAT)
#CONCERTLIB = $(CPLEXINSTALL)concert/lib/$(SYSTEM)/$(LIBFORMAT)

# CHANGEME: Cplex paths ok
CPLEXVERSION = CPLEX_Studio1210
CPLEXDIR = /opt/ibm/ILOG/$(CPLEXVERSION)/cplex
CPLEXINCDIR = $(CPLEXDIR)/include/
CPLEXLIBDIR = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CPLEXLIBFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex 

# CHANGEME: Concert paths ok
CONCERTVERSION = concert
CONCERTDIR = /opt/ibm/ILOG/$(CPLEXVERSION)/$(CONCERTVERSION)
CONCERTINCDIR = $(CONCERTDIR)/include/
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBFLAGS = -L$(CONCERTLIBDIR) -lconcert -lm -lpthread -ldl






#---------------------------------------
#
# EXECUTABLE
#
#---------------------------------------
exec: RSA_Solver.cpp
		$(CCC) $(FLAGS) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)  \
		-L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)  \
		 RSA_Solver.cpp Vertex.cpp Edge.cpp Arc.cpp Graph.cpp Demand.cpp Path.cpp RSA_Input.cpp Clique.cpp RSA_Output.cpp MCMCF_Output.cpp RSA_Algorithms.cpp \
		-lconcert -lilocplex -lcplex \
		-lm -lpthread -ldl -o exe

clean: 
		rm -f RSA_Solver *.o *~ exe



