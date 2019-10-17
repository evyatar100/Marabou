//
// Created by evyat on 10/16/2019.
//

#include "SplitSelector.h"

#include <iostream>

#include "Debug.h"
SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> *plConstraints,
                              List<PiecewiseLinearConstraint *> *violatedPlConstraints )
        :
        _plConstraints( plConstraints ),
        _violatedPlConstraints( violatedPlConstraints ),
        _numOfConstraints( plConstraints->size() ),
        _constraint2index(),
        _constraint2OpenLogEntry(),
        _log(),
        _generator()
{
    std::cout << "start SS constructor" << std::endl;
    int i = 0;
    for ( auto constraint: *_violatedPlConstraints )
    {
        _constraint2index[constraint] = i;
        _constraint2OpenLogEntry[constraint] = nullptr;
        ++i;
    }
    std::cout << "end SS constructor" << std::endl;
}

PiecewiseLinearConstraint *SplitSelector::getNextConstraint()
{
    std::cout << "start SS getNextConstraint" << std::endl;

    std::uniform_int_distribution<int> distribution( 0, _violatedPlConstraints->size() - 1 );
    int i = distribution( _generator );
    auto it = _violatedPlConstraints->begin();
    std::advance( it, i );

    std::cout << "start SS getNextConstraint" << std::endl;
    return *it;
}

void SplitSelector::logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintSplit" << std::endl;

    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end() );
    ASSERT( _constraint2OpenLogEntry[constraintForSplitting] == nullptr );

    std::cout << "1 SS logPLConstraintSplit" << std::endl;


    LogEntry *logEntry = new LogEntry( _numOfConstraints );
    logEntry->splittedConstraint = _constraint2index[constraintForSplitting];
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    std::cout << "2 SS logPLConstraintSplit" << std::endl;

    for ( auto constraint: *_violatedPlConstraints )
    {
        std::cout << constraint << " 3 SS logPLConstraintSplit" << std::endl;

        //int i = _constraint2index[constraint];
        //logEntry->isActive[i] = constraint->isActive();
    }

    std::cout << "4 SS logPLConstraintSplit" << std::endl;


    for ( auto constraint: *_violatedPlConstraints )
    {
        std::cout <<constraint<< "  5 SS logPLConstraintSplit" << std::endl;

//        int i = _constraint2index[constraint];
//        logEntry->isViolated[i] = true;
    }
    std::cout << " 6 SS logPLConstraintSplit" << std::endl;

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