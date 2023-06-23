#include <optional>
#include <iostream>

using namespace std;

class IterationInfo
{
private:
    int IterationId;
    int LowerBound;
    int CliqueBound;
    int UpperBound;
    optional<bool> EPFSolved;
    optional<bool> EPFSkipped;
    optional<int> EPFTime;
    optional<int> EPFOpt;
    optional<int> EPFSpan;
    optional<bool> MCMCFSolved;
    optional<int> MCMCFTime;
    optional<int> MCMCFOpt;
    optional<bool> NewPaths;
    optional<bool> NewCliques;
    optional<int> CliquesTime;
    optional<int> IterationTime;

public:
    IterationInfo(int itId, int boundLower, int boundClique, int boundUpper, optional<bool> mcmcfSolved = nullopt, optional<int> mcmcfTime = nullopt, 
        optional<int> mcmcfOpt = nullopt, optional<bool> epfSolved = nullopt, optional<bool> epfSkipped = nullopt, optional<int> epfTime = nullopt, 
        optional<int> epfOpt = nullopt, optional<int> epfSpan = nullopt, optional<bool> newPaths = nullopt, optional<bool> newCliques = nullopt, 
        optional<int> cliquesTime = nullopt, optional<int> iterationTime = nullopt);
    ~IterationInfo();
    friend ostream &operator<<(ostream &os, const IterationInfo &itInfo);
};

ostream &operator<<(ostream &os, const IterationInfo &itInfo);