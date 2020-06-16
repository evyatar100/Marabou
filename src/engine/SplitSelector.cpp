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
#include "AutoTableau.h"

#define COMMA_REPLACEMENT '.'
#define IS_CHOSEN_HEADLINE_SUFFIX "_is_chosen"
#define IS_ACTIVE_HEADLINE_SUFFIX "_is_active"
#define IS_VIOLATED_HEADLINE_SUFFIX "_is_violated"
#define PHASE_STATUS_HEADLINE_SUFFIX "_phase_status"

#define B_VAL_SUFFIX "_B_val"
#define B_LOWER_SUFFIX "_B_lower"
#define B_UPPER_SUFFIX "_B_upper"
#define F_VAL_SUFFIX "_F_val"
#define F_LOWER_SUFFIX "_F_lower"
#define F_UPPER_SUFFIX "_F_upper"


#define CSV_FILE_PATH "/cs/labs/guykatz/evyatar100/Marabou/splitSelector_statistics/"

void constraint2String( std::string *s, PiecewiseLinearConstraint *constraint );

std::string generateCSVPath();

SplitSelector::SplitSelector( List<PiecewiseLinearConstraint *> plConstraints, AutoTableau &tableau ):

        _plConstraints( plConstraints.begin(), plConstraints.end()), _tableau( tableau ), _numOfConstraints( plConstraints.size()), _constraint2index(),
        _constraint2OpenLogEntry(), _generator(), _fout(), _csvPath(), _tensorFlowSocket(), _selectorMode( DEFAULT_SELECTOR_MODE )
{
    std::cout << "start SS constructor" << '\n';
	if (DEFAULT_SELECTOR_MODE == NN)
	{
		_tensorFlowSocket.reInitiateSocket();
		if (!_tensorFlowSocket.isSocketReady())
		{
			std::cout << "warning: Socket is not ready. maybe the server is not running." << '\n';
		}
	}
    struct timeval tv;
    gettimeofday( &tv, NULL );
    _generator = std::default_random_engine( static_cast<long unsigned int>(time( 0 )) + tv.tv_usec );

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
    _fout.close();
    _fout.open( _csvPath, std::ios::out | std::ios::app );
}

SplitSelector::~SplitSelector()
{
    std::cout << '\n' << "SplitSelector deleted" << '\n';
    _fout.close();
}


PiecewiseLinearConstraint *SplitSelector::getConstraintFromNN( List<PiecewiseLinearConstraint *> *plViolatedConstraints )
{
    ASSERTM(_tensorFlowSocket.isSocketReady(), "Error: Socket is not ready. maybe the server is not running.")
    ASSERT( plViolatedConstraints != nullptr );
    string networkStateStr = getNetworkStateStr( plViolatedConstraints );

	// get the result
    std::cout << "getting the results from the server..." << '\n';
    std::stringstream estimatorResultSS( _tensorFlowSocket.runModel( networkStateStr ));
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

string SplitSelector::getNetworkStateStr( List<PiecewiseLinearConstraint *> *plViolatedConstraints) const
{
    unordered_set<PiecewiseLinearConstraint *> plViolatedConstraintsSet( plViolatedConstraints->begin(), plViolatedConstraints->end());
    bool isViolated;
    ReluConstraint* reluConstraintPtr;
    unsigned b, f;

    // create network_state string
    stringstream network_state;
    for ( auto constraint: _plConstraints )
    {
        network_state << constraint->isActive();

        isViolated = plViolatedConstraintsSet.find( constraint ) != plViolatedConstraintsSet.end();
        network_state << ',' << isViolated;

        reluConstraintPtr = dynamic_cast<ReluConstraint *>( constraint );
        network_state << ',' << reluConstraintPtr->getPhaseStatus();

        b = reluConstraintPtr->getB();
        network_state << ',' << _tableau->getValue(b);
        network_state << ',' << _tableau->getLowerBound(b);
        network_state << ',' << _tableau->getUpperBound(b);

        f = reluConstraintPtr->getB();
        network_state << ',' << _tableau->getValue(f);
        network_state << ',' << _tableau->getLowerBound(f);
		network_state << ',' << _tableau->getUpperBound(f);

		if (_plConstraints.back() != constraint)
		{
			network_state << ',';  // we don't want this comma in the end.
		}
    }

    string network_state_str = network_state.str();

	size_t n = std::count(network_state_str.begin(), network_state_str.end(), ',') + 1;
	ASSERT(n == 2016)  // TODO N_CONSTRAINS * N_FEATURES_PER_CONSTRAINS

	// length ~~ 13001
	return network_state_str;;
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


    LogEntry *logEntry = new LogEntry();
    logEntry->splittedConstraint = constraintForSplitting;
    logEntry->numVisitedTreeStatesAtSplit = numVisitedTreeStates;

    logEntry->networkState = getNetworkStateStr( plConstraintsOptions );

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
        _fout << "," << constraintName << IS_VIOLATED_HEADLINE_SUFFIX;
        _fout << "," << constraintName << PHASE_STATUS_HEADLINE_SUFFIX;

        _fout << "," << constraintName << B_VAL_SUFFIX;
        _fout << "," << constraintName << B_LOWER_SUFFIX;
        _fout << "," << constraintName << B_UPPER_SUFFIX;
        _fout << "," << constraintName << F_VAL_SUFFIX;
        _fout << "," << constraintName << F_LOWER_SUFFIX;
        _fout << "," << constraintName << F_UPPER_SUFFIX;
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

    _fout << size << ',';
    _fout << logEntry->networkState;

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

    std::cout << '\n' << "csv path = "<< path << '\n';

    return path;
}
