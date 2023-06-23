#ifndef RSA_ALGO_HPP
#define RSA_ALGO_HPP
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>

class RSA_Input;
class RSA_Output;
class MCMCF_Output;
class Path;
class Graph;
class Clique;
class Obstruction;

class RSA_Algorithms
{
	private:
		RSA_Input 								RSA_Input_; 				//data loaded from external files
		Graph									GPrime_;					//graph for MCMCF model
		Graph									G_;							//graph for RSA 
		MCMCF_Output 							MCMCF_Output_;				//output from last execution of MCMCF model (solution)
		RSA_Output 								RSA_Output_; 				//output from last execution of RSA model (solution)	
		vector<vector<Path*>*>					possiblePathsNew_;			//PBar, paths possible to be chosen in EPF
		Obstruction								obstructions_;				//contains all forbidden cliques and routings that are circuits
        unsigned								upperBound_;				
        unsigned                                lowerBound_;
        unsigned                                cliqueBound_;
        unsigned								smallestClique_;			//This variable is the smallest value between the smallest forbidden clique or the ub
        unsigned 								maxSlots_;					//The maximal number of slots avaible in the problem
        bool									MCMCFSolved_;				//a flag that is true when the MCMCF have a solution
        bool									RSASolved_;					//a flag that is true when the RSA have a solution
		int										MCMCFDuration_;
		int                                     EPFDuration_;
		int                                     CliquesDuration_;
        double									gap_;						//gap of the current EPF solution
        bool                                    isOptimal_;					//framework found a possible solution
        unsigned								iterations_;
		double									tConstraintsMCF_;
		double									tConstraintsEPF_;
		double									tTimeMCF_;
		double									tTimeEPF_;
		unsigned								nCallsMCF_;
		unsigned								nCallsEPF_;
		vector<vector<vector<Demand *>>> mirrorDemands_;

		//Functions
		void 			Construct_G_Prime();
		bool 			UpdatePossiblePathsSet();								// Adds found paths to the pool of possible paths. Returns true if at least one new path was added, false otherwise. I.e., the set remains the same
		void 			maxWeightedClique(std::vector<Path*> &  routing);		// function that finds weighted cliques that are bigger than the lower bound
		vector<vector<vector<Demand *>>> findMirrorDemands(vector<Demand *> demands);
		string getPathLabel(int demandIdx, Path* pth);
		vector<vector<Path*>> generateMirrorRoutings(vector<Path*> initialRouting);
		vector<vector<Path *>> permute(vector<Path *> routing, vector<Demand*> demands);

	public:
		//Constructors
		RSA_Algorithms(RSA_Input rsa);

		//Getters
		const RSA_Output & getRSAOutput() const { return RSA_Output_; }

		//Functions
		void			   framework(char *summaryFile = NULL, bool skipEPF = false);
		void 			   solveMinCostMultiCommodityFlow_Cplex();
		void 			   solveEdgePathFormulation_Cplex();
		//Shows
		void 			   showPossiblePaths() const ;
		void 			   showForbiddenCliques() const;

		~RSA_Algorithms(){};
};

#endif // RSA_ALGO_HPP