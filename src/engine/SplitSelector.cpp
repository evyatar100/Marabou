//
// Created by evyat on 10/16/2019.
//

#include "SplitSelector.h"
#include "ReluConstraint.h"

#include <iostream>

#include <ctime>


#include <list>
#include <vector>
#include <string>

#include "Debug.h"
#include <stdlib.h>


#define CSV_FILE_PATH "SplitSelector_statistics"

SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> plConstraints )
        :
        _plConstraints( plConstraints )
        , _numOfConstraints( plConstraints.size() )
        , _constraint2index()
        , _constraint2OpenLogEntry()
        , _log()
        , _generator()
        , _fout()
        , _csvPath()
{
    std::cout << "start SS constructor" << '\n';
    int i = 0;
    for ( auto constraint: _plConstraints )
    {
        _constraint2index[constraint] = i;
        _constraint2OpenLogEntry[constraint] = nullptr;
        ++i;
    }
//    std::time_t t = std::time(0);
//    std::tm* now = std::localtime(&t);
//    _csvPath << CSV_FILE_PATH << "-" <<now->tm_hour<< "-" << now->tm_min << "-"<< now->tm_sec << ".csv";


    _fout.open(CSV_FILE_PATH, std::ios::out);
    writeHeadLine();
}

SplitSelector::~SplitSelector()
{
    std::cout << '\n' << "SplitSelector statistics:" << '\n';
//    for ( auto logEntry: _log )
//    {
//        int size = logEntry->numVisitedTreeStatesAtUnsplit - logEntry->numVisitedTreeStatesAtSplit;
//        std::cout << "splittedConstraint: " << logEntry->splittedConstraint << '\t';
//        std::cout << "size of subtree: " << size << '\n';
//        delete logEntry;
//    }
    _fout.close();
}

PiecewiseLinearConstraint *SplitSelector::getNextConstraint()
{
    std::cout << "start SS getNextConstraint" << '\n';

    std::vector<PiecewiseLinearConstraint *> activeConstraints(_numOfConstraints);
    for (auto constraint: _plConstraints)
    {
        if ( constraint->isActive() )
        {
            activeConstraints.push_back(constraint);
        }
    }

    std::cout << "activeConstraints.size() = " << activeConstraints.size() << " SS 2 getNextConstraint" << std::endl;
    if (activeConstraints.size() == 0)
    {
        return nullptr;
    }
//    PiecewiseLinearConstraint *constraint = nullptr;
    std::uniform_int_distribution<int> distribution( 0, activeConstraints.size() - 1 );
    int i = distribution( _generator );

    std::cout << "i = " << i << " SS 3 getNextConstraint" << std::endl;
//    auto it = activeConstraints.begin();

//    std::advance( it, i );
    return activeConstraints[i];

//    return constraint;
}

void SplitSelector::logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintSplit" << '\n';

    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end() );
    ASSERT( _constraint2OpenLogEntry[constraintForSplitting] == nullptr );


    LogEntry *logEntry = new LogEntry( _numOfConstraints );
    logEntry->splittedConstraint = constraintForSplitting;
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    for ( auto constraint: _plConstraints )
    {
        int i = _constraint2index[constraint];
        logEntry->isActive[i] = constraint->isActive();
    }


    _constraint2OpenLogEntry[constraintForSplitting] = logEntry;
//    _log.push_back( logEntry );
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

    delete logEntry;
}

void SplitSelector::writeHeadLine()
{
    _fout << "current constraint" << ", " << "sub-tree size";
    ReluConstraint *tempRelu;
    for (auto constraint: _plConstraints)
    {
        tempRelu = (ReluConstraint*) constraint;
        _fout << ", " << tempRelu->serializeToString().ascii();
    }

    _fout << "\n";
}

void SplitSelector::writeLogEntry(LogEntry* logEntry)
{
    int size = logEntry->numVisitedTreeStatesAtUnsplit - logEntry->numVisitedTreeStatesAtSplit;
    ReluConstraint *tempRelu = (ReluConstraint*) logEntry->splittedConstraint;
    _fout << tempRelu->serializeToString().ascii() << ", " << size;
    for (auto x: logEntry->isActive)
    {
        _fout << ", " << x;
    }

    _fout << "\n";
}