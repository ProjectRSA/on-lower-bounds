#include "Vertex.hpp"
#include "Arc.hpp"
#include "Edge.hpp"
#include "Path.hpp"
#include "Clique.hpp"
#include "Demand.hpp"

using namespace std;

// the vertices here represent the demands involved in the clique (vertices of the intersection graph)
Clique::Clique(std::vector<Vertex*> v) : vertices_(v)
{
	size_ = v.size();
	weight_ = 0;
    index_ = 0;
    numberOfChildren_ = 0;
    isSymm = false;

	for (std::vector<Vertex*>::iterator it = v.begin(); it!= v.end(); it++){
		weight_ += (*it)->getWeight() ;
	}
}

Clique::Clique(std::vector<Path*> paths, std::vector<Demand*> demands, std::vector<std::vector<Path*>*> & possiblePaths) {
    size_ = demands.size();
    weight_ = 0;
    index_ = 0;
    numberOfChildren_ = 0;
    isSymm = false;
    demands_ = demands;
    paths_ = paths;

    for (std::vector<Demand*>::iterator it = demands.begin(); it!= demands.end(); it++){
		weight_ += (*it)->getSlots();
	}

    for (unsigned i = 0; i < paths.size(); i++) {
        int pthIdx = 0;
        while(!(*paths[i] == *(*possiblePaths[i])[pthIdx]))
        {
            pthIdx++;
        }
        std::string lbl = "P" + to_string(i + 1) + "_" + to_string(pthIdx + 1);
        pathsLabels_.push_back(lbl);

        for (unsigned j = 0; j < paths.size(); j++) {
            if (i < j) {
                for (unsigned k = 0; k < paths[i]->getEdges().size(); k++) {
                    for (unsigned l = 0; l < paths[j]->getEdges().size(); l++) {
                        if (k < l) {
                            if (paths[i]->getEdges()[k]->getIndex() == paths[j]->getEdges()[l]->getIndex()) {
                                addForbiddenEdgesFromCliques(paths[i]->getEdges()[k], demands[i], demands[j]);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Clique::addVertex(Vertex* & v)
{
    vertices_.push_back(v);
    weight_ += v->getWeight();
    size_ ++;
}

void Clique::removeTheLastVertex()
{
    weight_ -= vertices_[vertices_.size()-1]->getWeight();
    vertices_.pop_back();
    size_ --;
}

void Clique::addPath(string label, Path* & p)
{
    pathsLabels_.push_back(label);
    paths_.push_back(p);
}

bool Clique::tryAddVertex(Vertex* & v)
{
    if (vertices_.size() == 0)
    {
        addVertex(v);
        return true;
    }
    bool isPossible;
    for (unsigned i = 0; i < vertices_.size(); ++i)
    {
        isPossible = false;
        for (unsigned j = 0; j < vertices_[i]->getNeighborhood().size(); ++j)
        {
            if (vertices_[i]->getNeighborhood()[j]->getIndex() == v->getIndex() )
            {
                isPossible = true;
                break;
            }
        }
        if(isPossible == false)
            return false;
    }
    addVertex(v);
    return true;
}

void Clique::addForbiddenEdgesFromCliques(Edge* & e, Demand* d1, Demand* d2)
{
    for (vector<forbiddenEdgesFromCliques>::iterator it = forbiddenEdgesFromCliques_.begin(); it != forbiddenEdgesFromCliques_.end(); ++it)
    {
        if((*it).edges->getIndex() == e->getIndex())
        {
            bool v1 = true;
            bool v2 = true;
            for (vector<Demand*>::const_iterator it2 = (*it).indexDemand.begin(); it2 != (*it).indexDemand.end(); ++it2)
            {
                if ((*it2)->getIndex() == d1->getIndex())
                    v1 = false;
                if ((*it2)->getIndex() == d2->getIndex())
                    v2 = false;
            }
            if (v1 == true)
                (*it).indexDemand.push_back(d1);
            if (v2 == true)
                (*it).indexDemand.push_back(d2);

            return ;
        }
    }
    forbiddenEdgesFromCliques fefc;
    fefc.edges = e;
    fefc.indexDemand.push_back(d1);
    fefc.indexDemand.push_back(d2);
    forbiddenEdgesFromCliques_.push_back(fefc);
    return ;
}

std::vector<unsigned> Clique::getDemandIndices() const {
    std::vector<unsigned> demands;
    for (unsigned i = 0; i < demands_.size(); i++) {
        demands.push_back(demands_[i]->getIndex());
    }
    return demands;
}

bool Clique::eraseCliqueChild(Clique* c) {
    for (unsigned i = 0; i < numberOfChildren_; i++) {
        if (cliqueChildren_[i]->getIndex() == c->getIndex()) {
            cliqueChildren_.erase(cliqueChildren_.begin()+i);
            numberOfChildren_ --;
            break;
        }
    }
    bool noMoreChildren = false;
    if (numberOfChildren_ == 0) {noMoreChildren = true;}
    return noMoreChildren;
}

void Clique::resetDependencies() {
    cliqueParents_.clear();
    cliqueChildren_.clear();
    numberOfChildren_ = 0;
}

void Clique::setDemands(std::vector<Demand*> allDemands) {
    for (unsigned j = 0; j < vertices_.size(); j++) {
        for (unsigned i = 0; i < allDemands.size(); i++) {
            if (allDemands[i]->getIndex() == vertices_[j]->getIndex()) {
                demands_.push_back(allDemands[i]);
                break;
            }
        }
    }
}

void Clique::symmetricalSubstitution(Demand* dout, Demand* din) {
    unsigned indexDOut;
    for (unsigned i = 0; i < size_; i++) {
        if (vertices_[i]->getIndex() == dout->getIndex()) {
            indexDOut = i;
            break;
        }
    }

    std::string lblout = pathsLabels_[indexDOut];
    std::string newlblout = "P" + std::to_string(dout->getIndex()) + "_";
    std::string newlblin = "P" + std::to_string(din->getIndex()) + "_";
    newlblin += lblout.substr(newlblout.size());

    pathsLabels_[indexDOut] = newlblin;
    forbiddenEdgesSymmSubstitution(dout, din);
    demandsSymmSubstitution(dout, din);
}

void Clique::demandsSymmSubstitution(Demand* dout, Demand* din) {
    for (unsigned i = 0; i < demands_.size(); i++) {
        if (demands_[i]->getIndex() == dout->getIndex()) {
            demands_[i] = din;
            break;
        }
    }
}

void Clique::forbiddenEdgesSymmSubstitution(Demand* dout, Demand* din) {
    for (unsigned i = 0; i < forbiddenEdgesFromCliques_.size(); i++) {
        for (unsigned j = 0; j < forbiddenEdgesFromCliques_[i].indexDemand.size(); j++) {
            if (forbiddenEdgesFromCliques_[i].indexDemand[j]->getIndex() == dout->getIndex()) {
                forbiddenEdgesFromCliques_[i].indexDemand[j] = din;
                break;
            }
        }
    }
}

void Clique::symmetricalInversion(Demand* d1, Demand* d2) {
    unsigned indexD1; unsigned indexD2;
    if (vertices_.size() > 0) { // this is a partial clique created with the intersection graph, thus it has vertices
        for (unsigned i = 0; i < size_; i++) {
            if (vertices_[i]->getIndex() == d1->getIndex()) {
                indexD1 = i;
            }
            else if (vertices_[i]->getIndex() == d2->getIndex()) {
                indexD2 = i;
            }
        }
        std::string lbl1 = pathsLabels_[indexD1];
        std::string lbl2 = pathsLabels_[indexD2];
        std::string newlbl1 = "P" + std::to_string(d1->getIndex()) + "_";
        std::string newlbl2 = "P" + std::to_string(d2->getIndex()) + "_";
        unsigned size1 = newlbl1.size();
        newlbl1 += lbl2.substr(newlbl2.size());
        newlbl2 += lbl1.substr(size1);

        if (lbl1 != newlbl1) {
            pathsLabels_[indexD1] = newlbl1;
            pathsLabels_[indexD2] = newlbl2;

            Path* aux = paths_[indexD1];
            paths_[indexD1] = paths_[indexD2];
            paths_[indexD2] = aux;

            forbiddenEdgesSymmInversion(d1, d2);
        }
    }
    else { // this is a complete clique derived from a routing, thus it hasn't vertices
        indexD1 = d1->getIndex() - 1;
        indexD2 = d2->getIndex() - 1;
        if (!(*paths_[indexD1] == *paths_[indexD2])) {
            std::string lbl1 = pathsLabels_[indexD1];
            std::string lbl2 = pathsLabels_[indexD2];
            std::string newlbl1 = "P" + std::to_string(d1->getIndex()) + "_";
            std::string newlbl2 = "P" + std::to_string(d2->getIndex()) + "_";
            unsigned size1 = newlbl1.size();
            newlbl1 += lbl2.substr(newlbl2.size());
            newlbl2 += lbl1.substr(size1);
            pathsLabels_[indexD1] = newlbl1;
            pathsLabels_[indexD2] = newlbl2;

            Path* aux = paths_[indexD1];
            paths_[indexD1] = paths_[indexD2];
            paths_[indexD2] = aux;

            forbiddenEdgesSymmInversion(d1, d2);
        }
    }   
}

void Clique::forbiddenEdgesSymmInversion(Demand* d1, Demand* d2) {
    for (unsigned i = 0; i < forbiddenEdgesFromCliques_.size(); i++) {
        for (unsigned j = 0; j < forbiddenEdgesFromCliques_[i].indexDemand.size(); j++) {
            if (forbiddenEdgesFromCliques_[i].indexDemand[j]->getIndex() == d1->getIndex()) {
                forbiddenEdgesFromCliques_[i].indexDemand[j] = d2;
            }
            else if (forbiddenEdgesFromCliques_[i].indexDemand[j]->getIndex() == d2->getIndex()) {
                forbiddenEdgesFromCliques_[i].indexDemand[j] = d1;
            }
        }
    }
}

bool Clique::operator==(Clique& c2) const {
    // we are going to count how many demand-path pairs the cliques share
    unsigned counter = 0;
    if (c2.getSize() == this->getSize()) {
        for (unsigned l = 0; l < c2.getSize(); l++) {
            for (unsigned m = 0; m < this->getSize(); m++) {
                if ((c2.getDemands()[l]->getIndex() == this->getDemands()[m]->getIndex()) && (c2.getPaths()[l] == this->getPaths()[m])) {
                    counter += 1;
                    break;
                }
            }
            if (counter != l+1) {return false;} // we know that the cliques are different (they don't share one of their demand-path pairs)
        }
        return true;
    }
    else {return false;} // the cliques must have the same size in order to be equal
}

std::ostream & operator << (std::ostream & flux, const Clique & c)
{
    flux << "Clique ID: " << c.getIndex() << endl;
	flux << "Size: " << c.getSize() << "| Weight: " << c.getOmega() << std::endl ;
	flux << "Clique: ( " ;

	for (std::vector<Demand*>::const_iterator it = c.getDemands().begin(); it != c.getDemands().end(); it++) {
		flux << (*it)->getIndex() << " ";
    }
	flux << ")" << std::endl ;

    flux << "Paths : ( " ;
	for (vector<string>::const_iterator pLbl = c.pathsLabels_.begin(); pLbl != c.pathsLabels_.end(); pLbl++)
		flux << *pLbl << " ";
	flux << ")" << std::endl ;
    for (unsigned i = 0; i < c.getPaths().size(); ++i)
    {
        flux << (*(c.getPaths()[i]));
    }
	flux << "-----------------------" << std::endl ;
	return flux;
}


