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
#include <fstream>

class SplitSelector
{
public:
    SplitSelector(
            List<PiecewiseLinearConstraint *> plConstraints );

    ~SplitSelector();

    PiecewiseLinearConstraint *getNextConstraint();

    void logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates );

    void logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates );

private:

    struct LogEntry
    {

        LogEntry( int numOfConstraints )
                : isViolated( numOfConstraints )
                , isActive( numOfConstraints )
                , splittedConstraint( nullptr )
                , numVisitedTreeStatesAtSplit( -1 )
                , numVisitedTreeStatesAtUnsplit( -1 )
        {
            for ( int i = 0; i < numOfConstraints; ++i )
            {
                isActive[i] = false;
                isViolated[i] = false;
            }
        }

        std::vector<bool> isViolated;
        std::vector<bool> isActive;
        PiecewiseLinearConstraint* splittedConstraint;
        int numVisitedTreeStatesAtSplit;
        int numVisitedTreeStatesAtUnsplit;
    };

    List<PiecewiseLinearConstraint *> _plConstraints;

    int _numOfConstraints;

    std::map<PiecewiseLinearConstraint *, int> _constraint2index;

    std::map<PiecewiseLinearConstraint *, LogEntry *> _constraint2OpenLogEntry;

    std::list<LogEntry *> _log;

    std::default_random_engine _generator;

    std::fstream _fout;

    void writeHeadLine();
    void writeLogEntry(LogEntry* logEntry);
};


#endif //MARABOU_SPLITSELECTOR_H
