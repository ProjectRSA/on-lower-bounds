#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX 12.8 programs
# please copy this makefile and set CPLEXINSTALL to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------
CPLEXINSTALL = /opt/ibm/ILOG/CPLEX_Studio1210/
LIBFORMAT = static_pic
SYSTEM = x86-64_linux

#---------------------------------------
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
CCC = g++
RFLAGS = -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -g -Wall -Wextra -std=c++17 -Wno-ignored-attributes
DFLAGS = -m64 -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -g -Wall -Wextra -std=c++17 -Wno-ignored-attributes
OUTDIR = ./bin/
APPNAME = RSASolver

exec: RSA_Solver.cpp
		$(CCC) $(RFLAGS) -I$(CPLEXINC) -I$(CONCERTINC)  \
		-L$(CPLEXLIB) -L$(CONCERTLIB)  \
		 RSA_Solver.cpp Vertex.cpp Edge.cpp Arc.cpp Graph.cpp Demand.cpp Path.cpp RSA_Input.cpp Clique.cpp RSA_Output.cpp MCMCF_Output.cpp RSA_Algorithms.cpp IterationInfo.cpp \
		-lconcert -lilocplex -lcplex \
		-lm -lpthread -ldl -o $(OUTDIR)$(APPNAME)

clean: 
		rm -rfv $(OUTDIR)*

debug: RSA_Solver.cpp
		$(CCC) $(DFLAGS) -I$(CPLEXINC) -I$(CONCERTINC)  \
		-L$(CPLEXLIB) -L$(CONCERTLIB)  \
		 RSA_Solver.cpp Vertex.cpp Edge.cpp Arc.cpp Graph.cpp Demand.cpp Path.cpp RSA_Input.cpp Clique.cpp RSA_Output.cpp MCMCF_Output.cpp RSA_Algorithms.cpp IterationInfo.cpp\
		-lconcert -lilocplex -lcplex \
		-lm -lpthread -ldl -o $(OUTDIR)$(APPNAME)_Debug