#ifndef TRIGGER_CHECKER_H
#define TRIGGER_CHECKER_H


#include "TreeAnalyzer.h"
#include "XmlFunction.h"

#include <map>
#include <vector>
using namespace std;

class TriggerChecker : public TreeAnalyzer
{
protected:
	
	int eBinEdges[19][10];
	int eCorrs[19][10];

	int wBinEdges[19][10];
	int wCorrs[19][10];

	int numBins;

	XmlFunction *fRes;

	vector< int > refMeanE, refMeanW;
	map<int, int> refMap;

	vector<int> tacOffEast, tacOffWest;

public:
	TriggerChecker( );
	~TriggerChecker();

	virtual void initialize();

	virtual void make();
	virtual void analyzeEvent();
	virtual void postEventLoop();


	void readParams();

	int corrEast( int iCh, int adc, int tac );
	int corrWest( int iCh, int adc, int tac );

	int corrBin( int iCh, int adc, int *bins );

	int refCorrect( int iCh, int tac, string crate, string side );
	
};
#endif