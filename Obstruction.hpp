#ifndef OBSTRUCTION_H
#define OBSTRUCTION_H
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

//Forward declaration
class Clique;
class Demand;

class Obstruction
{
	private:
		unsigned							counterCliques_; 	// used to give each new clique an unique ID
		std::vector<Clique*>				circuitCliques_; 	// cliques with no children so far
		std::vector<Clique*>				redundantCliques_; 	// cliques with one or more children so far
		std::vector<Clique*>				allCliques_; 		// stores pointers to all known unique cliques
		std::vector<std::vector<Demand*>>	demandGroups_;		// the groups of demands that are symmetrical between themselves
		std::vector<Demand*>				allDemands_;		// all the demands of this instance

	public:
		//Constructors
		Obstruction(): counterCliques_(0) {}

		//Getters
		const std::vector<Clique*> 		getCircuitCliques() const {return circuitCliques_;}
		unsigned						getNumberCliques() const {return circuitCliques_.size();}

		//Setters
		void 							setDemandGroups(std::vector<std::vector<std::vector<Demand*>>>);
		void 							setDemands(std::vector<Demand*> ds) {allDemands_ = ds;}

		//Functions
		void							insert(std::vector<std::vector<Path*>>&, std::vector<std::vector<Path*>*> &); // checks uniqueness, verifies and creates dependency relations for the clique
		void							insert(Clique);									// checks uniqueness, verifies and creates dependency relations for the clique

		void							permuteAndInsert(Clique);						// creates all the possibilities of symmetrical cliques for a given originalclique
		Clique							permuteClique(Clique, Demand*, Demand*) const;	// returns a symmetrical clique by either replacing or inverting demands of it

		bool							AllowFeasible(unsigned);						// erases all the cliques with weight equal and under a bound. also defines new circuits and its symmetrical brothers
		unsigned						minForbiddenCliquesWeight() const;				// returns the value of the smallest weight from known cliques

		void							removeFromRedundantCliques(unsigned);
		void							removeFromCircuitCliques(unsigned);

		//Destructors
		~Obstruction(){}
};

#endif // OBSTRUCTION_H
