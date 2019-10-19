//
// Created by evyat on 10/16/2019.
//

#include "SplitSelector.h"

#include <iostream>
#include <list>

#include "Debug.h"



SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> plConstraints )
        :
        _plConstraints( plConstraints )
        , _numOfConstraints( plConstraints.size() )
        , _constraint2index()
        , _constraint2OpenLogEntry()
        , _log()
        , _generator()
        , _fout()
{
    std::cout << "start SS constructor" << '\n';
    int i = 0;
    for ( auto constraint: _plConstraints )
    {
        _constraint2index[constraint] = i;
        _constraint2OpenLogEntry[constraint] = nullptr;
        ++i;
    }

    _fout.open("SplitSelector_statistics.csv");
    writeHeadLine();

    std::cout << "end SS constructor" << '\n';
}

SplitSelector::~SplitSelector()
{
    std::cout << '\n' << "SplitSelector statistics:" << '\n';
    for ( auto logEntry: _log )
    {
        int size = logEntry->numVisitedTreeStatesAtUnsplit - logEntry->numVisitedTreeStatesAtSplit;
        std::cout << "splittedConstraint: " << logEntry->splittedConstraint << '\t';
        std::cout << "size of subtree: " << size << '\n';
        delete logEntry;
    }
    _fout.close();

}

PiecewiseLinearConstraint *SplitSelector::getNextConstraint()
{
    std::cout << "start SS getNextConstraint" << '\n';

    std::list<PiecewiseLinearConstraint *> activeConstraints;
    for (auto constraint: _plConstraints)
    {
        if ( constraint->isActive() )
        {
            activeConstraints.push_back(constraint);
        }
    }

    if (activeConstraints.size() == 0)
    {
        return nullptr;
    }
    PiecewiseLinearConstraint *constraint = nullptr;
    std::uniform_int_distribution<int> distribution( 0, activeConstraints.size() - 1 );
    int i = distribution( _generator );

    std::cout << "i = " << i << " SS 3 getNextConstraint" << std::endl;
    std::cout << "_numOfConstraints = " << _numOfConstraints << " SS 3.1 getNextConstraint" << std::endl;
    auto it = _plConstraints.begin();

    std::advance( it, i );
    constraint = *it;

    std::cout << "end SS getNextConstraint" << '\n';
    return constraint;
}

void SplitSelector::logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintSplit" << '\n';

    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end() );
    ASSERT( _constraint2OpenLogEntry[constraintForSplitting] == nullptr );

    std::cout << "1 SS logPLConstraintSplit" << '\n';


    LogEntry *logEntry = new LogEntry( _numOfConstraints );
    logEntry->splittedConstraint = constraintForSplitting;
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    std::cout << "2 SS logPLConstraintSplit" << '\n';

    for ( auto constraint: _plConstraints )
    {
//        std::cout << constraint << " 3 SS logPLConstraintSplit" << std::endl;

        int i = _constraint2index[constraint];
        logEntry->isActive[i] = constraint->isActive();
    }

    std::cout << "4 SS logPLConstraintSplit" << '\n';


    _constraint2OpenLogEntry[constraintForSplitting] = logEntry;
    _log.push_back( logEntry );
    std::cout << "start SS logPLConstraintSplit" << '\n';
}

void SplitSelector::logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintUnsplit" << '\n';
    ASSERT( _constraint2OpenLogEntry.find( constraintForUnsplitting ) != _constraint2OpenLogEntry.end() );

    LogEntry *logEntry = _constraint2OpenLogEntry[constraintForUnsplitting];
    ASSERT( logEntry != nullptr );

    _constraint2OpenLogEntry[constraintForUnsplitting] = nullptr;

    logEntry->numVisitedTreeStatesAtUnsplit = numVisitedTreeStates;

    writeLogEntry(logEntry);

    std::cout << "start SS logPLConstraintUnsplit" << '\n';
}

void SplitSelector::writeHeadLine()
{
    _fout << "current constraint" << ", " << "sub-tree size";
    for (auto constraint: _plConstraints)
    {
        _fout << ", " << constraint;
    }

    _fout << "\n";
}

void SplitSelector::writeLogEntry(LogEntry* logEntry)
{
    int size = logEntry->numVisitedTreeStatesAtUnsplit - logEntry->numVisitedTreeStatesAtSplit;
    _fout << logEntry->splittedConstraint << ", " << size;
    for (auto x: logEntry->isActive)
    {
        _fout << ", " << x;
    }

    _fout << "\n";
}