//
// Created by evyat on 10/16/2019.
//

#include "SplitSelector.h"

#include "Debug.h"
SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> *plConstraints,
                              List<PiecewiseLinearConstraint *> *violatedPlConstraints )
        :
        _plConstraints( plConstraints ),
        _violatedPlConstraints( violatedPlConstraints ),
        _numOfConstraints( _plConstraints->size() ),
        _constraint2index(),
        _constraint2OpenLogEntry(),
        _log(),
        _generator(),
{
    int i = 0;
    for ( auto constraint: *_violatedPlConstraints )
    {
        _constraint2index[constraint] = i;
        _constraint2OpenLogEntry[constraint] = nullptr;
        ++i;
    }
}

PiecewiseLinearConstraint *SplitSelector::getNextConstraint()
{
    std::uniform_int_distribution<int> distribution( 0, _violatedPlConstraints->size() - 1 );
    int i = distribution( _generator );
    auto it = _violatedPlConstraints->begin();
    std::advance( it, i );
    return *it;
}

void SplitSelector::logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates )
{
    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end() );
    ASSERT( _constraint2OpenLogEntry[constraintForSplitting] == nullptr );

    LogEntry *logEntry = new LogEntry( _numOfConstraints );
    logEntry->splittedConstraint = _constraint2index[constraintForSplitting];
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    for ( auto constraint: *_violatedPlConstraints )
    {
        int i = _constraint2index[constraint];
        logEntry->isActive[i] = constraint->isActive();
    }

    for ( auto constraint: *_violatedPlConstraints )
    {
        int i = _constraint2index[constraint];
        logEntry->isViolated[i] = true;
    }

    _constraint2OpenLogEntry[constraintForSplitting] = logEntry;
    _log.push_back( logEntry );
}

void SplitSelector::logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates )
{
    ASSERT( _constraint2OpenLogEntry.find( constraintForUnsplitting ) != _constraint2OpenLogEntry.end() );

    LogEntry *logEntry = _constraint2OpenLogEntry[constraintForUnsplitting];
    ASSERT( logEntry != nullptr );

    _constraint2OpenLogEntry[constraintForUnsplitting] = nullptr;

    logEntry->numVisitedTreeStatesAtUnsplit = numVisitedTreeStates;
}