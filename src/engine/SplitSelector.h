//
// Created by evyat on 10/16/2019.
//

#ifndef MARABOU_SPLITSELECTOR_H
#define MARABOU_SPLITSELECTOR_H

#include "PiecewiseLinearConstraint.h"
#include <map>
#include <random>
#include <vector>
#include <list>

class SplitSelector
{
public:
    SplitSelector(
            List<PiecewiseLinearConstraint *> *plConstraints,
            List<PiecewiseLinearConstraint *> *violatedPlConstraints );

    ~SplitSelector();

    PiecewiseLinearConstraint *getNextConstraint();

    void logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates );

    void logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates );

    //TODO add destructor

private:

    struct LogEntry
    {

        LogEntry( int numOfConstraints )
                : isViolated( numOfConstraints ), isActive( numOfConstraints ), splittedConstraint( -1 ), numVisitedTreeStatesAtSplit( -1 ), numVisitedTreeStatesAtUnsplit( -1 )
        {
            for ( int i = 0; i < numOfConstraints; ++i )
            {
                isActive[i] = false;
                isViolated[i] = false;
            }
        }

        std::vector<bool> isViolated;
        std::vector<bool> isActive;
        int splittedConstraint;
        int numVisitedTreeStatesAtSplit;
        int numVisitedTreeStatesAtUnsplit;
    };

    List<PiecewiseLinearConstraint *> *_plConstraints;
    List<PiecewiseLinearConstraint *> *_violatedPlConstraints;

    int _numOfConstraints;

    std::map<PiecewiseLinearConstraint *, int> _constraint2index;

    std::map<PiecewiseLinearConstraint *, LogEntry *> _constraint2OpenLogEntry;

    std::list<LogEntry *> _log;

    std::default_random_engine _generator;
};


#endif //MARABOU_SPLITSELECTOR_H
