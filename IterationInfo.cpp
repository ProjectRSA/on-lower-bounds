#include "IterationInfo.hpp"

IterationInfo::IterationInfo(int itId, int boundLower, int boundClique, int boundUpper, optional<bool> mcmcfSolved, optional<int> mcmcfTime, 
        optional<int> mcmcfOpt, optional<bool> epfSolved, optional<bool> epfSkipped, optional<int> epfTime,
        optional<int> epfOpt, optional<int> epfSpan, optional<bool> newPaths, optional<bool> newCliques, optional<int> cliquesTime, optional<int> iterationTime)
{
    IterationId = itId;
    EPFSolved = epfSolved;
    EPFSkipped = epfSkipped;
    EPFTime = epfTime;
    EPFOpt = epfOpt;
    MCMCFSolved = mcmcfSolved;
    MCMCFTime = mcmcfTime;
    MCMCFOpt = mcmcfOpt;
    LowerBound = boundLower;
    CliqueBound = boundClique;
    UpperBound = boundUpper;
    NewPaths = newPaths;
    NewCliques = newCliques;
    EPFSpan = epfSpan;
    CliquesTime = cliquesTime;
    IterationTime = iterationTime;
}

IterationInfo::~IterationInfo()
{
}

std::ostream &operator<<(std::ostream &os, const IterationInfo &itInfo)
{
    string epfs;
    string epfts;
    string epfopts;
    string epfspans;
    if(itInfo.EPFSkipped && *itInfo.EPFSkipped)
    {
        epfs = "Skipped";
        epfts = "-";
        epfopts = "-";
        epfspans = "-";
    }
    else
    {
        epfs = itInfo.EPFSolved ? *itInfo.EPFSolved ? "S" : "I" : "-";
        epfts = itInfo.EPFTime ? to_string(*itInfo.EPFTime) : "-";
        epfopts = itInfo.EPFSolved ? *itInfo.EPFSolved ? to_string(*itInfo.EPFOpt) : "-" : "-";
        epfspans = itInfo.EPFSolved ? *itInfo.EPFSolved ? to_string(*itInfo.EPFSpan) : "-" : "-";
    }

    string mcmcfs = itInfo.MCMCFSolved ? *itInfo.MCMCFSolved ? "S" : "I" : "-";
    string mcmcfopts = itInfo.MCMCFSolved ? *itInfo.MCMCFSolved ? to_string(*itInfo.MCMCFOpt) : "-" : "-";
    string newPathsS = itInfo.NewPaths ? *itInfo.NewPaths ? "true" : "false" : "-";
    string newCliquesS = itInfo.NewCliques ? *itInfo.NewCliques ? "true" : "false" : "-";
    string mcmcfts = itInfo.MCMCFTime ? to_string(*itInfo.MCMCFTime) : "-";
    string cliquests = itInfo.CliquesTime ? to_string(*itInfo.CliquesTime) : "-";
    string iterTimeS = itInfo.IterationTime ? to_string(*itInfo.IterationTime) : "-";
    os << itInfo.IterationId << "; " << epfs << "; " << epfts << "; " << epfopts << "; " << epfspans << "; "
        << mcmcfs << "; " << mcmcfts << "; " << mcmcfopts << "; " 
        << itInfo.LowerBound << "; " << itInfo.CliqueBound << "; " << itInfo.UpperBound << "; " 
        << newPathsS << "; " << newCliquesS << "; " << cliquests << "; " << iterTimeS;
    return os;
}