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
        unsigned								upperBound_;				
        unsigned                                lowerBound_;
        unsigned								smallestClique_;			//This variable is the smallest value between the smallest forbidden clique or the ub
        unsigned 								maxSlots_;					//The maximal number of slots avaible in the problem
        bool									RSASolved_;					//a flag that is true when the RSA have a solution
        double									gap_;						//gap of the current EPF solution
        bool                                    isOptimal_;					//framework found a possible solution
        unsigned								iterations_;        

		//Functions
		void 			Construct_G_Prime();
		void 			addNewRoutesToRSA();									//function that add new possible paths to RSA

	public:
		//Constructors
		RSA_Algorithms(RSA_Input rsa);

		//Functions
		void            framework_1();
		void 			solveKShortest();
		void 			solveMinCostMultiCommodityFlow_Cplex();
		void 			solveEdgePathFormulation_Cplex();
		//Shows
		void 			showPossiblePaths() const ;

		~RSA_Algorithms(){};
};

#endif // RSA_ALGO_HPP