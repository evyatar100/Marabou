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

class SplitSelector {
public:
    SplitSelector(IEngine *engine,
                  List<PiecewiseLinearConstraint *> &plConstraints,
                  List<PiecewiseLinearConstraint *> &violatedPlConstraints);

    PiecewiseLinearConstraint *getNextConstraint();

    void logPLConstraintSplit(PiecewiseLinearConstraint *constraint, int numVisitedTreeStates);

    void logPLConstraintUnsplit(PiecewiseLinearConstraint *constraint, int numVisitedTreeStates);

private:

    struct LogEntry {

        LogEntry(int numOfConstraints) : isViolated(numOfConstraints), isActive(numOfConstraints),
                                         splittedConstraint(-1), numVisitedTreeStatesAtSplit(-1),
                                         numVisitedTreeStatesAtUnsplit(-1) {
            for (int i = 0; i < numOfConstraints; ++i)
            {
                isActive[i] = false;
                isViolated[i] = false;
            }
        }

        Vector<bool> isViolated;
        Vector<bool> isActive;
        int splittedConstraint;
        int numVisitedTreeStatesAtSplit;
        int numVisitedTreeStatesAtUnsplit;
    };

    IEngine *engine;

    int _numOfConstraints;

    List<PiecewiseLinearConstraint *> &_plConstraints;
    List<PiecewiseLinearConstraint *> &_violatedPlConstraints;

    std::List<LogEntry *> _log;

    std::Map<PiecewiseLinearConstraint *, LogEntry *> _constraint2OpenLogEntry;

    std::Map<PiecewiseLinearConstraint *, int> _constraint2index;

    std::default_random_engine _generator;
};


#endif //MARABOU_SPLITSELECTOR_H
