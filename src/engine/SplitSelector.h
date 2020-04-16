//
// Created by evyat on 10/16/2019.
//

#ifndef MARABOU_SPLITSELECTOR_H
#define MARABOU_SPLITSELECTOR_H

#include "TensorFlowSocket.h"
#include "PiecewiseLinearConstraint.h"
#include <map>
#include <random>
#include <vector>
#include <string>
#include <list>
#include <fstream>

enum SelectorMode {RANDOM, NN, NONE};
const SelectorMode DEFAULT_SELECTOR_MODE = NONE;

class SplitSelector
{
public:
    SplitSelector(
            List<PiecewiseLinearConstraint *> plConstraints );

    ~SplitSelector();

    PiecewiseLinearConstraint *getNextConstraint(List<PiecewiseLinearConstraint *> *plConstraintsOptions = nullptr);

    void logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates, List<PiecewiseLinearConstraint *> *plConstraintsOptions = nullptr );

    void logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates );

    void setSelectorMode(SelectorMode mode);

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

    PiecewiseLinearConstraint *getConstraintFromNN(List<PiecewiseLinearConstraint *> *plConstraintsOptions = nullptr);
    PiecewiseLinearConstraint *getRandomConstraint(List<PiecewiseLinearConstraint *> *plConstraintsOptions = nullptr);



    std::vector <PiecewiseLinearConstraint *> _plConstraints;

    int _numOfConstraints;

    std::map<PiecewiseLinearConstraint *, int> _constraint2index;

    std::map<PiecewiseLinearConstraint *, LogEntry *> _constraint2OpenLogEntry;

//    std::list<LogEntry *> _log;

    std::default_random_engine _generator;

    std::fstream _fout;

    std::string _csvPath;

    TensorFlowSocket _tensorFlowSocket;

    void writeHeadLine();
    void writeLogEntry(LogEntry* logEntry);

    SelectorMode _selectorMode;
};


#endif //MARABOU_SPLITSELECTOR_H
