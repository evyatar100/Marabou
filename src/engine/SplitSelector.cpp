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
        _generator(),
        _constraint2index(),
        _log(),
        _constraint2OpenLogEntry(),
        _numOfConstraints(
        _plConstraints->size() )
{
    for ( int i = 0; i<_numOfConstraints; ++i )
    {
        _constraint2index[_plConstraints->OPERATOR[]( i )] = i;
        _constraint2OpenLogEntry[_plConstraints->OPERATOR[]( i )] = nullptr;
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

    LogEntry &logEntry = new LogEntry( _numOfConstraints );
    logEntry.splittedConstraint = _constraint2index[constraintForSplitting];
    logEntry.numVisitedTreeStatesAtSplit( numVisitedTreeStates );

    for ( auto constraint: *_violatedPlConstraints )
    {
        int i = _constraint2index[constraint];
        logEntry.isActive[i] = constraint->isActive();
    }

    for ( auto constraint: *_violatedPlConstraints )
    {
        int i = _constraint2index[constraint];
        logEntry.isViolated[i] = true;
    }

    _constraint2OpenLogEntry[constraint] = logEntry;
    _log->push_back( logEntry );
}

void SplitSelector::logPLConstraintUnsplit( PiecewiseLinearConstraint *constraint, int numVisitedTreeStates )
{
    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end() );

    LogEntry *logEntry = _constraint2OpenLogEntry[constraint];
    ASSERT( logEntry != nullptr );

    _constraint2OpenLogEntry[constraint] = nullptr;

    logEntry->numVisitedTreeStatesAtUnsplit = numVisitedTreeStates;
}