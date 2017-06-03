#include "GainChecker.h"
#include "RooPlotLib.h"

GainChecker::GainChecker(  ){

}

GainChecker::~GainChecker(){

}


void GainChecker::make(){
	TreeAnalyzer::make();

}

void GainChecker::analyzeEvent(){
	TreeAnalyzer::analyzeEvent();
	

	for (int i = 0; i < 16; i++){

		//cout << ds->get( "numberOfVpdEast" ) << endl;
		double adcEast = ds->get( "vpdBbqAdcEast", i);
		double adcWest = ds->get( "vpdBbqAdcWest", i);

		if ( adcEast > 0  && adcEast < 500 )
			book->fill( "bbqAdcEast", i, adcEast );
		if ( adcWest > 0  && adcWest < 500 )
			book->fill( "bbqAdcWest", i, adcWest );

	}

	for (int i = 0; i < 19; i++){

		//cout << ds->get( "numberOfVpdEast" ) << endl;
		double le = ds->get( "vpdLeWest", i);
		double tot = ds->get( "vpdTotWest", i);

		if ( tot > 10  && tot < 40 ){
			book->fill( "leWest", i, le );
			book->fill( "totWest", i, tot );	
		}

		le = ds->get( "vpdLeEast", i);
		tot = ds->get( "vpdTotEast", i);

		if ( tot > 10  && tot < 40 ){
			book->fill( "leEast", i, le );
			book->fill( "totEast", i, tot );	
		}
		

	}
}


void GainChecker::postEventLoop(){

	vector<int> maskEast = config.getIntVector( nodePath + "MaskChannelsEast" );
	vector<int> maskWest = config.getIntVector( nodePath + "MaskChannelsWest" );

	for ( int c : maskEast ){
		cout << "Masking East : " << c << endl;
	}
	for ( int c : maskWest ){
		cout << "Masking West : " << c << endl;
	}

	TH1D * px = (TH1D*)(book->get2D( "totWest" )->ProfileX());
	double mean = 0;
	for ( int i = 0; i < 19; i++ ){
		if ( find( maskWest.begin(), maskWest.end(), i ) != maskWest.end() ) continue;
		mean += px->GetBinContent( i+1);
	}
	mean = mean / ( 19 - maskWest.size() );
	for ( int i = 0; i < 19; i++ ){
		double bc = px->GetBinContent( i+1 );
		book->get( "pDiffTotWest" )->SetBinContent( i+1, (( mean - bc ) / mean) * 100 );
	}


	px = (TH1D*)(book->get2D( "totEast" )->ProfileX());
	mean = 0;
	for ( int i = 0; i < 19; i++ ){
		if ( find( maskEast.begin(), maskEast.end(), i ) != maskEast.end() ) {continue; cout << "masked : " << i << endl;}
		mean += px->GetBinContent( i+1);
	}
	mean = mean / (19 - maskEast.size());
	for ( int i = 0; i < 19; i++ ){

		double bc = px->GetBinContent( i+1 );
		book->get( "pDiffTotEast" )->SetBinContent( i+1, (( mean - bc ) / mean) * 100 );
	}



	gStyle->SetOptStat(0);

	/**
	 * Generate a quick report
	 */
	RooPlotLib rpl;
	reporter->newPage(1, 2);
	rpl.style( book->get("pDiffTotEast") ).set( "draw", "p" )
	.set( "markerstyle", 2 ).set("range", -15, 15)
	.set( "x", "Channel #" ).set( "y", "% diff" ).draw();
	reporter->cd( 1, 2 );
	rpl.style( book->get("pDiffTotWest") ).set( "draw", "p" )
	.set( "markerstyle", 2 ).set("range", -15, 15)
	.set( "x", "Channel #" ).set( "y", "% diff" ).draw();
	reporter->savePage();



/*
	px = (TH1D*)(book->get2D( "bbqAdcEast" )->ProfileX());
	mean = 0;
	for ( int i = 0; i < 16; i++ ){
		mean += px->GetBinContent( i+1);
	}
	mean = mean / 16;
	for ( int i = 0; i < 16; i++ ){
		double bc = px->GetBinContent( i+1 );
		book->get( "pDiffBbqEast" )->SetBinContent( i+1, (( mean - bc ) / mean) * 100  );
	}

	px = (TH1D*)(book->get2D( "bbqAdcWest" )->ProfileX());
	mean = 0;
	for ( int i = 0; i < 16; i++ ){
		mean += px->GetBinContent( i+1);
	}
	mean = mean / 16;
	for ( int i = 0; i < 16; i++ ){
		double bc = px->GetBinContent( i+1 );
		book->get( "pDiffBbqWest" )->SetBinContent( i+1, (( mean - bc ) / mean) );
	}
	*/


}






