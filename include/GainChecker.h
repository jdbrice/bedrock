#ifndef GAIN_CHECKER_H
#define GAIN_CHECKER_H

#include "TreeAnalyzer.h"

class GainChecker : public TreeAnalyzer
{
public:
	GainChecker(  );
	~GainChecker();

	virtual void make();
	virtual void analyzeEvent();
	virtual void postEventLoop();
	
};
#endif