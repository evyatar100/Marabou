//
// Created by evyat on 10/16/2019.
//

#include "SplitSelector.h"

#include <iostream>

#include "Debug.h"

SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> plConstraints )
        :
        _plConstraints( plConstraints )
        , _numOfConstraints( plConstraints.size() )
        , _constraint2index()
        , _constraint2OpenLogEntry()
        , _log()
        , _generator()
{
    std::cout << "start SS constructor" << std::endl;
    int i = 0;
    for ( auto constraint: _plConstraints )
    {
        _constraint2index[constraint] = i;
        _constraint2OpenLogEntry[constraint] = nullptr;
        ++i;
    }
    std::cout << "end SS constructor" << std::endl;
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

}

PiecewiseLinearConstraint *SplitSelector::getNextConstraint()
{
    std::cout << "start SS getNextConstraint" << std::endl;
    if (_numOfConstraints == 0)
    {
        return nullptr;
    }
    PiecewiseLinearConstraint *constraint = nullptr;
    bool foundActiveConstraint = false;
    std::cout << "1 SS getNextConstraint" << std::endl;
    std::uniform_int_distribution<int> distribution( 0, _numOfConstraints - 1 );
    int i = 0;
    while ( !foundActiveConstraint )
    {
        std::cout << " 2 SS getNextConstraint" << std::endl;
        i = distribution( _generator );
        std::cout << "i = " << i << " SS 3 getNextConstraint" << std::endl;
        std::cout << "_numOfConstraints = " << _numOfConstraints << " SS 3.1 getNextConstraint" << std::endl;
        auto it = _plConstraints.begin();
        std::cout << "4 SS getNextConstraint" << std::endl;
        std::advance( it, i );
        std::cout << "5 SS getNextConstraint" << std::endl;
        constraint = *it;
        std::cout << "6 SS getNextConstraint" << std::endl;

        if ( constraint != nullptr && constraint->isActive() )
        {
            std::cout << "7.5 SS getNextConstraint" << std::endl;
            foundActiveConstraint = true;
        }
        std::cout << "7 SS getNextConstraint" << std::endl;
    }

    std::cout << "start SS getNextConstraint" << std::endl;
    return constraint;
}

void SplitSelector::logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintSplit" << std::endl;

    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end() );
    ASSERT( _constraint2OpenLogEntry[constraintForSplitting] == nullptr );

    std::cout << "1 SS logPLConstraintSplit" << std::endl;


    LogEntry *logEntry = new LogEntry( _numOfConstraints );
    logEntry->splittedConstraint = constraintForSplitting;
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    std::cout << "2 SS logPLConstraintSplit" << std::endl;

    for ( auto constraint: _plConstraints )
    {
//        std::cout << constraint << " 3 SS logPLConstraintSplit" << std::endl;

        int i = _constraint2index[constraint];
        logEntry->isActive[i] = constraint->isActive();
    }

    std::cout << "4 SS logPLConstraintSplit" << std::endl;


    _constraint2OpenLogEntry[constraintForSplitting] = logEntry;
    _log.push_back( logEntry );
    std::cout << "start SS logPLConstraintSplit" << std::endl;
}

void SplitSelector::logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintUnsplit" << std::endl;
    ASSERT( _constraint2OpenLogEntry.find( constraintForUnsplitting ) != _constraint2OpenLogEntry.end() );

    LogEntry *logEntry = _constraint2OpenLogEntry[constraintForUnsplitting];
    ASSERT( logEntry != nullptr );

    _constraint2OpenLogEntry[constraintForUnsplitting] = nullptr;

    logEntry->numVisitedTreeStatesAtUnsplit = numVisitedTreeStates;
    std::cout << "start SS logPLConstraintUnsplit" << std::endl;
}