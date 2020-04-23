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
#include <set>
#include <unordered_set>
#include <sstream>

#include "Debug.h"
#include <stdlib.h>

#define COMMA_REPLACEMENT '.'
#define IS_CHOSEN_HEADLINE_SUFFIX "_is_chosen"
#define IS_ACTIVE_HEADLINE_SUFFIX "_is_active"
#define IS_VIOLATED_HEADLINE_SUFFIX "_is_violated"


#define CSV_FILE_PATH "splitSelector_statistics/"

void constraint2String( std::string *s, PiecewiseLinearConstraint *constraint );

std::string generateCSVPath();


SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> plConstraints )
        :
        _plConstraints( plConstraints.begin(), plConstraints.end()), _numOfConstraints( plConstraints.size()), _constraint2index(),
        _constraint2OpenLogEntry(), _generator(), _fout(), _csvPath(), _tensorFlowSocket(), _selectorMode( DEFAULT_SELECTOR_MODE )
{
    ASSERTM(_tensorFlowSocket.isSocketReady(), "Error: Socket is not ready. maybe the server is not running.")
    struct timeval tv;
    gettimeofday( &tv, NULL );
    _generator = std::default_random_engine( static_cast<long unsigned int>(time( 0 )) + tv.tv_usec );

    std::cout << "start SS constructor" << '\n';
    int i = 0;
    for ( auto constraint: _plConstraints )
    {
        _constraint2index[constraint] = i;
        _constraint2OpenLogEntry[constraint] = nullptr;
        ++i;
    }

    _csvPath = generateCSVPath();

    _fout.open( _csvPath, std::ios::out );
    _fout.close();
    _fout.open( _csvPath, std::ios::out | std::ios::app );
    writeHeadLine();
}

SplitSelector::~SplitSelector()
{
    std::cout << '\n' << "SplitSelector deleted" << '\n';
    _fout.close();
}


PiecewiseLinearConstraint *SplitSelector::getConstraintFromNN( List<PiecewiseLinearConstraint *> *plViolatedConstraints )
{

    ASSERT( plViolatedConstraints != nullptr );
    std::unordered_set<PiecewiseLinearConstraint *> plViolatedConstraintsSet( plViolatedConstraints->begin(), plViolatedConstraints->end());
    bool isViolated;

    // create network_state string
    std::stringstream network_state;
    network_state << _plConstraints[0]->isActive();
    for ( int i = 1; i < _numOfConstraints; ++i )
    {
        network_state << ',' << _plConstraints[i]->isActive();
    }
    for ( int i = 0; i < _numOfConstraints; ++i )
    {
        isViolated = plViolatedConstraintsSet.find( _plConstraints[i] ) != plViolatedConstraintsSet.end();
        network_state << ',' << isViolated;
    }

    // get the result
    std::cout << "getting the results from the server..." << '\n';
    std::stringstream estimatorResultSS( _tensorFlowSocket.runModel( network_state.str()));
    std::cout << "got the results!" << '\n';

    std::vector<int> estimatorResult;

    // parse result
    for ( int i; estimatorResultSS >> i; )
    {
        estimatorResult.push_back( i );
        if ( estimatorResultSS.peek() == ',' )
            estimatorResultSS.ignore();
    }

    // generate a set of relevant constraints
    std::set<int> relevantConstraint;
    for ( auto constraint: *plViolatedConstraints )
    {
        if ( constraint->isActive())
        {
            relevantConstraint.insert( _constraint2index[constraint] );
        }
    }

    // return the best legal constraint
    for ( int constraintIndex: estimatorResult )
    {
        if ( relevantConstraint.find( constraintIndex ) != relevantConstraint.end())
        {
            return _plConstraints[constraintIndex];
        }
    }

    return nullptr;
}

PiecewiseLinearConstraint *SplitSelector::getRandomConstraint( List<PiecewiseLinearConstraint *> *plViolatedConstraints )
{
    std::vector<PiecewiseLinearConstraint *> activeConstraints;
    if ( plViolatedConstraints == nullptr )
    {
        for ( auto constraint: _plConstraints )
        {
            if ( constraint->isActive())
            {
                activeConstraints.push_back( constraint );
            }
        }
    } else
    {
        for ( auto constraint: *plViolatedConstraints )
        {
            if ( constraint->isActive())
            {
                activeConstraints.push_back( constraint );
            }
        }
    }

    std::cout << "activeConstraints.size() = " << activeConstraints.size() << " SS 2 getNextConstraint" << std::endl;
    if ( activeConstraints.size() == 0 )
    {
        return nullptr;
    }
    std::uniform_int_distribution<int> distribution( 0, activeConstraints.size() - 1 );
    int i = distribution( _generator );

    std::cout << "i = " << i << " SS 3 getNextConstraint" << std::endl;
    return activeConstraints[i];

}

PiecewiseLinearConstraint *SplitSelector::getNextConstraint( List<PiecewiseLinearConstraint *> *plViolatedConstraints )
{
    std::cout << "start SS getNextConstraint. mode = " << _selectorMode << '\n';

    if ( _selectorMode == NN )
    {
        return getConstraintFromNN( plViolatedConstraints );
    } else if ( _selectorMode == RANDOM )
    {
        return getRandomConstraint( plViolatedConstraints );
    }

    return nullptr;

}

void SplitSelector::logPLConstraintSplit( PiecewiseLinearConstraint *constraintForSplitting, int numVisitedTreeStates,
                                          List<PiecewiseLinearConstraint *> *plConstraintsOptions )
{
    std::cout << "start SS logPLConstraintSplit" << '\n';

    ASSERT( plConstraintsOptions != nullptr )
    ASSERT( _constraint2OpenLogEntry.find( constraintForSplitting ) != _constraint2OpenLogEntry.end());
    ASSERT( _constraint2OpenLogEntry[constraintForSplitting] == nullptr );


    LogEntry *logEntry = new LogEntry( _numOfConstraints );
    logEntry->splittedConstraint = constraintForSplitting;
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    for ( auto constraint: _plConstraints )
    {
        int i = _constraint2index[constraint];
        logEntry->isActive[i] = constraint->isActive();

    }

    if ( plConstraintsOptions != nullptr )
    {
        for ( auto constraint: *plConstraintsOptions )
        {
            int i = _constraint2index[constraint];
            logEntry->isViolated[i] = true;
        }
    } else
    {
        for ( auto constraint: _plConstraints )
        {
            int i = _constraint2index[constraint];
            logEntry->isViolated[i] = -1;
        }
    }

    _constraint2OpenLogEntry[constraintForSplitting] = logEntry;
}

void SplitSelector::logPLConstraintUnsplit( PiecewiseLinearConstraint *constraintForUnsplitting, int numVisitedTreeStates )
{
    std::cout << "start SS logPLConstraintUnsplit" << '\n';
    ASSERT( _constraint2OpenLogEntry.find( constraintForUnsplitting ) != _constraint2OpenLogEntry.end());

    LogEntry *logEntry = _constraint2OpenLogEntry[constraintForUnsplitting];
    ASSERT( logEntry != nullptr );

    _constraint2OpenLogEntry[constraintForUnsplitting] = nullptr;

    logEntry->numVisitedTreeStatesAtUnsplit = numVisitedTreeStates;

    writeLogEntry( logEntry );

    delete logEntry;
}

void SplitSelector::writeHeadLine()
{
    _fout << "sub-tree_size";
    std::string constraintName;
    for ( auto constraint: _plConstraints )
    {
        constraint2String( &constraintName, constraint );
        _fout << "," << constraintName << IS_ACTIVE_HEADLINE_SUFFIX;
    }
    for ( auto constraint: _plConstraints )
    {
        constraint2String( &constraintName, constraint );
        _fout << "," << constraintName << IS_VIOLATED_HEADLINE_SUFFIX;
    }
    for ( auto constraint: _plConstraints )
    {
        constraint2String( &constraintName, constraint );
        _fout << "," << constraintName << IS_CHOSEN_HEADLINE_SUFFIX;
    }

    _fout << "\n";
}

void SplitSelector::writeLogEntry( LogEntry *logEntry )
{
    int j = _constraint2index[logEntry->splittedConstraint];
    int size = logEntry->numVisitedTreeStatesAtUnsplit - logEntry->numVisitedTreeStatesAtSplit;
    std::string constraintName;
    constraint2String( &constraintName, logEntry->splittedConstraint );
//    _fout << constraintName << ", ";
    _fout << size;
    for ( int i = 0; i < _numOfConstraints; ++i )
    {
        _fout << "," << logEntry->isActive[i];
    }
    for ( int i = 0; i < _numOfConstraints; ++i )
    {
        _fout << "," << logEntry->isViolated[i];
    }
    for ( int i = 0; i < _numOfConstraints; ++i )
    {
        if ( i == j )
        {
            _fout << "," << 1;
        } else
        {
            _fout << "," << 0;
        }
    }

    _fout << "\n";
}

void SplitSelector::setSelectorMode( SelectorMode mode )
{
    _selectorMode = mode;
}

void constraint2String( std::string *s, PiecewiseLinearConstraint *constraint )
{
    ReluConstraint *relu = (ReluConstraint *) constraint;
    *s = relu->serializeToString().ascii();
    std::replace( s->begin(), s->end(), ',', COMMA_REPLACEMENT ); // replace all ',' to COMMA_REPLACEMENT
}

std::string generateCSVPath()
{
    char fmt[64];
    char buf[64];
    struct timeval tv;
    struct tm *tm;

    gettimeofday( &tv, NULL );
    tm = localtime( &tv.tv_sec );
    strftime( fmt, sizeof( fmt ), "%F-%H-%M-%S-%%06u", tm );
    snprintf( buf, sizeof( buf ), fmt, tv.tv_usec );
    std::string path = "";
    path.append( CSV_FILE_PATH );
    path.append( buf );
    path.append( ".csv" );

    std::cout << '\n' << path << '\n';

    return path;
}