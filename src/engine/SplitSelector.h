//
// Created by evyat on 10/16/2019.
//

#ifndef MARABOU_SPLITSELECTOR_H
#define MARABOU_SPLITSELECTOR_H

#include "TensorFlowSocket.h"
#include "PiecewiseLinearConstraint.h"
#include "AutoTableau.h"

#include <map>
#include <random>
#include <vector>
#include <string>
#include <list>
#include <fstream>

#define N_CONSTRAINTS 300
#define N_FEATURES_PER_CONSTRAINT 9


enum SelectorMode {RANDOM, NN, NONE};
const SelectorMode DEFAULT_SELECTOR_MODE = RANDOM;

class SplitSelector
{
public:
    SplitSelector( List<PiecewiseLinearConstraint *> plConstraints, AutoTableau &tableau );

    ~SplitSelector();

    PiecewiseLinearConstraint *getNextConstraint(List<PiecewiseLinearConstraint *> *plViolatedConstraints = nullptr);

    void logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates, List<PiecewiseLinearConstraint *> *plConstraintsOptions = nullptr );

    void logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates );

    void setSelectorMode(SelectorMode mode);


private:

    struct LogEntry
    {

        LogEntry()
                : splittedConstraint( nullptr )
                , numVisitedTreeStatesAtSplit( -1 )
                , numVisitedTreeStatesAtUnsplit( -1 )
        {}

        std::string networkState;
        PiecewiseLinearConstraint* splittedConstraint;
        int numVisitedTreeStatesAtSplit;
        int numVisitedTreeStatesAtUnsplit;
    };

    PiecewiseLinearConstraint *getConstraintFromNN(List<PiecewiseLinearConstraint *> *plViolatedConstraints = nullptr);
    PiecewiseLinearConstraint *getRandomConstraint(List<PiecewiseLinearConstraint *> *plViolatedConstraints = nullptr);



    std::vector <PiecewiseLinearConstraint *> _plConstraints;

    AutoTableau &_tableau;

    int _numOfConstraints;

    std::map<PiecewiseLinearConstraint *, int> _constraint2index;

    std::map<PiecewiseLinearConstraint *, LogEntry *> _constraint2OpenLogEntry;

    std::default_random_engine _generator;

    std::fstream _fout;

    std::string _csvPath;

    TensorFlowSocket _tensorFlowSocket;

    SelectorMode _selectorMode;


    void writeHeadLine();
    void writeLogEntry(LogEntry* logEntry);
    string getNetworkStateStr( List<PiecewiseLinearConstraint *> *plViolatedConstraints ) const;

};


#endif //MARABOU_SPLITSELECTOR_H
