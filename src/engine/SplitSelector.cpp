//
// Created by evyat on 10/16/2019.
//

#include "SplitSelector.h"
#include "ReluConstraint.h"

#include <iostream>

#include <sys/time.h>


#include <list>
#include <vector>
#include <string>

#include "Debug.h"
#include <stdlib.h>

#define COMMA_REPLACEMENT '.'


#define CSV_FILE_PATH "SplitSelector_statistics/"

void constraint2String(std::string *s, PiecewiseLinearConstraint *constraint);

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
    char fmt[64];
    char buf[64];
    struct timeval tv;
    struct tm *tm;

    gettimeofday (&tv, NULL);
    tm = localtime (&tv.tv_sec);
    strftime (fmt, sizeof (fmt), "%F-%H-%M-%S-%%06u", tm);
    snprintf (buf, sizeof (buf), fmt, tv.tv_usec);
    _csvPath.append(CSV_FILE_PATH);
    _csvPath.append(buf);
    _csvPath.append(".csv");
    std::cout << '\n' << _csvPath << '\n';


    _fout.open(_csvPath, std::ios::out);
    _fout.close();
    _fout.open(_csvPath, std::ios::out | std::ios::app);
    writeHeadLine();
}

SplitSelector::~SplitSelector()
{
    std::cout << '\n' << "SplitSelector deleted" << '\n';
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
    std::string constraintName;
    for (auto constraint: _plConstraints)
    {
        constraint2String(&constraintName, constraint);
        _fout << ", " << constraintName;
    }

    _fout << "\n";
}

void SplitSelector::writeLogEntry(LogEntry* logEntry)
{
    int size = logEntry->numVisitedTreeStatesAtUnsplit - logEntry->numVisitedTreeStatesAtSplit;
    std::string constraintName;
    constraint2String(&constraintName, logEntry->splittedConstraint);
    _fout << constraintName << ", " << size;
    for (auto x: logEntry->isActive)
    {
        _fout << ", " << x;
    }

    _fout << "\n";
}

void constraint2String(std::string *s, PiecewiseLinearConstraint *constraint)
{
    ReluConstraint *relu = (ReluConstraint*) constraint;
    *s = relu->serializeToString().ascii();
    std::replace( s->begin(), s->end(), ',', COMMA_REPLACEMENT); // replace all ',' to COMMA_REPLACEMENT
}