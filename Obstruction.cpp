#include "Clique.hpp"
#include "Vertex.hpp"
#include "Demand.hpp"
#include "Obstruction.hpp"
#include "algorithm"

using namespace std;

bool Obstruction::AllowFeasible(unsigned bound) {
    bool cliqueReAllowed = false;
    bool becomesCircuit = false;
	vector<Clique*>::iterator it = circuitCliques_.begin();
    vector<Clique*> newCircuits;
	while (it != circuitCliques_.end())
	{
		if ((*it)->getOmega() == bound)
		{
			cout << "The clique: " << endl << **it << "Becomes feasible" << endl << endl;
            std::vector<Clique*> cliqueParents = (*it)->getCliqueParents();
            for (unsigned l = 0; l < cliqueParents.size(); l++) {
                becomesCircuit = cliqueParents[l]->eraseCliqueChild((*it));
                if (becomesCircuit) {
                    newCircuits.push_back(cliqueParents[l]);
                    cout << "The clique " << endl << *cliqueParents[l] << " becomes circuit" << endl << endl;
                    removeFromRedundantCliques(cliqueParents[l]->getIndex());
                }
            }
            
			it = circuitCliques_.erase(it); 
			cliqueReAllowed = true;
		}
		else
		{
			++it;
		}
	}

    // this needs to be separated because we were iterating on circuitCliques_ thus we shouldn't change it
    // also we don't need to check if the new circuits should be allowed as they had an allowed child
    for (unsigned i = 0; i < newCircuits.size(); i++) {
        circuitCliques_.push_back(newCircuits[i]); // we add the clique to the circuits
        permuteAndInsert(*newCircuits[i]); // and create all its symmetrical brothers
    }

	return cliqueReAllowed;
}

void Obstruction::removeFromRedundantCliques(unsigned id) {
    for (unsigned i = 0; i < redundantCliques_.size(); i++) {
        if (id == redundantCliques_[i]->getIndex()) {
            redundantCliques_.erase(redundantCliques_.begin() + i);
            break;
        }
    }
}

void Obstruction::removeFromCircuitCliques(unsigned id) {
    for (unsigned i = 0; i < circuitCliques_.size(); i++) {
        if (id == circuitCliques_[i]->getIndex()) {
            circuitCliques_.erase(circuitCliques_.begin() + i);
            break;
        }
    }
}

unsigned Obstruction::minForbiddenCliquesWeight() const {
    if (circuitCliques_.size() > 0) {
        unsigned minWeight = circuitCliques_[0]->getOmega();
        for (unsigned i = 1; i < circuitCliques_.size(); ++i)
        {
            if (circuitCliques_[i]->getOmega() < minWeight)
            {
                minWeight = circuitCliques_[i]->getOmega();
            }
        }
        return minWeight;
    }
    else {return 0;}
}

void Obstruction::setDemandGroups(std::vector<std::vector<std::vector<Demand*>>> superGroups) {
    for (unsigned i = 0; i < superGroups.size(); i++) {
        for (unsigned j = 0; j < superGroups[i].size(); j++) {
            demandGroups_.push_back(superGroups[i][j]);
        }
    }
}

void Obstruction::permuteAndInsert(Clique c) {
    c.setSymm();
    std::vector<Clique> derivedCliques;
    derivedCliques.push_back(c);
    for (unsigned j = 0; j < demandGroups_.size(); j++) {
        for (unsigned k = 0; k < demandGroups_[j].size(); k++) {
            for (unsigned l = 0; l < demandGroups_[j].size(); l++) {
                if (k < l) {
                    std::vector<Clique> auxDerivedCliques;
                    for (unsigned m = 0; m < derivedCliques.size(); m++) {
                        Clique permuted = permuteClique(derivedCliques[m], demandGroups_[j][k], demandGroups_[j][l]);
                        if (!(permuted == derivedCliques[m])) {
                            permuted.resetDependencies();
                            auxDerivedCliques.push_back(permuted);
                        }
                    }
                    derivedCliques.insert(derivedCliques.end(), auxDerivedCliques.begin(), auxDerivedCliques.end());
                }
            }
        }
    }
    for (unsigned i = 1; i < derivedCliques.size(); i++) {
        insert(derivedCliques[i]);
    }
}

Clique Obstruction::permuteClique(Clique c, Demand* d1, Demand* d2) const {
    bool d1inc = false; bool d2inc = false;
    for (unsigned i = 0; i < c.getDemandIndices().size(); i++) {
        if (d1->getIndex() == c.getDemandIndices()[i]) {d1inc = true;}
        else if (d2->getIndex() == c.getDemandIndices()[i]) {d2inc = true;}
        if (d1inc && d2inc) {break;}
    }

    if (d1inc && !d2inc) {c.symmetricalSubstitution(d1, d2);}
    else if (!d1inc && d2inc) {c.symmetricalSubstitution(d2, d1);}
    else if (d1inc && d2inc) {c.symmetricalInversion(d1, d2);}

    return c;
}

void Obstruction::insert(Clique c) {
    bool known = false;
    if (!c.getSymm()) {c.setDemands(allDemands_);}
    // we check if we already know this clique
    for (unsigned i = 0; i < allCliques_.size(); i++) {
        if (*allCliques_[i] == c) {known = true; break;}
    }

    if (!known) {
        c.setIndex(counterCliques_); counterCliques_++;
        Clique* nc = new Clique; // new clique created with new so we don't lose its data
        *nc = c;
        allCliques_.push_back(nc);
        if (circuitCliques_.size() == 0) {circuitCliques_.push_back(nc);}
        else {
            bool ainb = false, bina = false, anotb = false;
            // the included vector will keep the IDs of the cliques that are parents of nc
            std::vector<unsigned> included;
            ainb = false, bina = false, anotb = false;
            for (unsigned j = 0; j < circuitCliques_.size(); j++) {
                int counter = 0;
                for (unsigned l = 0; l < nc->getSize(); l++) {
                    for (unsigned m = 0; m < circuitCliques_[j]->getSize(); m++) {
                        if ((nc->getPaths()[l] == circuitCliques_[j]->getPaths()[m]) && (nc->getDemands()[l]->getIndex() == circuitCliques_[j]->getDemands()[m]->getIndex())) {
                            counter += 1;
                            break;
                        }
                    }
                }
                if (counter == circuitCliques_[j]->getSize()) { // the number of common demand-path pairs is equal to the size of the circuit -> it contains nc
                    ainb = true;
                    circuitCliques_[j]->addCliqueParent(nc);
                    nc->addCliqueChild(circuitCliques_[j]);
                    redundantCliques_.push_back(nc);} // even if we know nc won't be a circuit, we still need to create the dependency relation with all other cliques (so no break)
                else if (counter == nc->getSize()) { // the number of common demand-path pairs is equal to the size of nc -> it contains the circuit
                    bina = true;
                    circuitCliques_[j]->addCliqueChild(nc);
                    nc->addCliqueParent(circuitCliques_[j]);
                    redundantCliques_.push_back(circuitCliques_[j]);
                    included.push_back(circuitCliques_[j]->getIndex());}
                else {anotb = true;}
            }
            if ((bina) && !(ainb)) {
                for (int it = 0; it < included.size(); it++) {removeFromCircuitCliques(included[it]);}
                circuitCliques_.push_back(nc);
            }
            else if ((anotb) && !(ainb)) {
                circuitCliques_.push_back(nc);
            }
        }

        if (nc->getNumberOfChildren() == 0  && !(nc->getSymm())) {
            permuteAndInsert(*nc);
        }
    }
}

void Obstruction::insert(std::vector<std::vector<Path*>> & routings_old, std::vector<std::vector<Path*>*> & possiblePaths) {
    for (unsigned i = 0; i < routings_old.size(); i++) {
        Clique c(routings_old[i], allDemands_, possiblePaths);
        insert(c);
    }
}

