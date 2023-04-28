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

class RSA_Algorithms
{
	private:
		RSA_Input 								RSA_Input_; 				//data loaded from external files
		Graph									GPrime_;					//graph for MCMCF model
		Graph									G_;							//graph for RSA 
		MCMCF_Output 							MCMCF_Output_;				//output from last execution of MCMCF model (solution)
		RSA_Output 								RSA_Output_; 				//output from last execution of RSA model (solution)	
        std::vector<std::vector<Path*>> 		possiblePaths_;				//paths possible to be chosen by the RSA model
        std::vector<Clique> 					forbiddenCliques_;			//cliques that should be forbidden
        std::vector<RSA_Output> 		        impossibleRoutings_;		//all previous RSA solutions
        std::vector<MCMCF_Output> 		        impossibleRoutings_MCMCF;	//all previous MCMCF solution
        unsigned								upperBound_;				
        unsigned                                lowerBound_;
        unsigned								smallestClique_;			//This variable is the smallest value between the smallest forbidden clique or the ub
        unsigned 								maxSlots_;					//The maximal number of slots avaible in the problem
        bool									MCMCFSolved_;				//a flag that is true when the MCMCF have a solution
        bool									RSASolved_;					//a flag that is true when the RSA have a solution
        double									gap_;						//gap of the current EPF solution
        bool                                    isOptimal_;					//framework found a possible solution
        unsigned								iterations_;        

		//Functions
		void 			Construct_G_Prime();
		void 			addNewRoutesToRSA();									//function that add new possible paths to RSA
		void 			maxWeightedClique(std::vector<Path*> &  routing);		//function that finds weighted cliques that are bigger than the lower bound
		void			lookForFeasibleSolution();								//function that verify if there is as solution with value lower than the lower bound
		bool			allowFeasibleCliques();									//funtion that verify it there are cliques with weight lower than the lower bound

	public:
		//Constructors
		RSA_Algorithms(RSA_Input rsa);

		//Functions
		void            framework_1();
		void 			solveMinCostMultiCommodityFlow_Cplex();
		void 			solveEdgePathFormulation_Cplex();
		//Shows
		void 			showPossiblePaths() const ;
		void 			showForbiddenCliques() const;

		~RSA_Algorithms(){};
};

#endif // RSA_ALGO_HPP