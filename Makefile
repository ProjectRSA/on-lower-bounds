SYSTEM = x86-64_linux
LIBFORMAT = static_pic
CCC = g++ -O2
FLAGS = -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -g -Wall -Wextra -std=c++14 -Wno-ignored-attributes

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX 12.8 programs
# please copy this makefile and set CPLEXINSTALL to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

CPLEXINSTALL = /home/ubuntu/Desktop/ibm/ILOG/CPLEX_Studio128/

#
# INCLUDE HEADERS AND LIBRARIES OF CPLEX and CONCERT DIRECTORIES 
#
#---------------------------------------
CPLEXINC = $(CPLEXINSTALL)cplex/include/
CONCERTINC = $(CPLEXINSTALL)concert/include/
CPLEXLIB = $(CPLEXINSTALL)cplex/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIB = $(CPLEXINSTALL)concert/lib/$(SYSTEM)/$(LIBFORMAT)

#---------------------------------------
#
# EXECUTABLE
#
#---------------------------------------
exec: RSA_Solver.cpp
		$(CCC) $(FLAGS) -I$(CPLEXINC) -I$(CONCERTINC)  \
		-L$(CPLEXLIB) -L$(CONCERTLIB)  \
		 RSA_Solver.cpp Vertex.cpp Edge.cpp Arc.cpp Graph.cpp Demand.cpp Path.cpp RSA_Input.cpp Clique.cpp RSA_Output.cpp MCMCF_Output.cpp RSA_Algorithms.cpp \
		-lconcert -lilocplex -lcplex \
		-lm -lpthread -ldl -o exe

clean: 
		rm -f RSA_Solver *.o *~ exe



