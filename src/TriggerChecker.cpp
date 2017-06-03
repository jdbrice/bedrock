#include "TriggerChecker.h"
#include "Tline.h"
#include "TProfile.h"

#include "XmlFunction.h"
#include "RooPlotLib.h"

TriggerChecker::TriggerChecker(  ) : TreeAnalyzer( ){
	gStyle->SetOptFit(1111);

	refMeanE = {  2082 ,  2385 ,  2350 ,  2082  };
	refMeanW = {  2440 ,  2329 ,  2290 ,  2172  };
	refMap = {{0,0}, { 1,0 }, {2,0}, {3,0}, {4,4}, {5,4}, {6,4}, {7,4}, {8,8}, {9,8}, {10,8}, {11,8}, {12,12}, {13,12}, {14,12}, {15,12} };
}

void TriggerChecker::initialize() {
	if ( config.exists( nodePath + ".Slewing" ) )
		readParams();

	fRes = new XmlFunction( &config, nodePath + ".XmlFunction" );
	gStyle->SetOptFit(1111);

	tacOffEast = config.getIntVector( nodePath + ".TacOffEast", 0, 16 );
	tacOffWest = config.getIntVector( nodePath + ".TacOffWest", 0, 16 );

	cout << "offsets East: " << endl;
	for ( int off : tacOffEast ){
		cout << off << ", ";
	}
	cout << endl;

	cout << "offsets West: " << endl;
	for ( int off : tacOffWest ){
		cout << off << ", ";
	}
	cout << endl;
}

TriggerChecker::~TriggerChecker(){

}


void TriggerChecker::make(){
	TreeAnalyzer::make();

}

void TriggerChecker::analyzeEvent(){
	TreeAnalyzer::analyzeEvent();
	
	int tacWest = -10;
	int tacEast = -10;

	int totalEast = 0;
	int totalWest = 0;

	int iEast = -1;
	int iWest = -1;

	int nEast = 0;
	int nWest = 0;

	int minTDC = 150;

	string crate = config.getString( nodePath + ".Crate", "Bbq" );
	bool applyCorr = config.getBool( nodePath + ".ApplyCorr", true );
	bool applyRefCorr = config.getBool( nodePath + ".RefCorrect", false );

	for (int i = 0; i < 16; i++){
		
		if ( applyRefCorr && ( 0 == i || 4 == i || 8 == i || 12 == i ) ) continue;
		Int_t tEast = (Int_t)ds->get( "vpd" + crate + "TdcEast", i);
		if ( tEast < 1  ) continue;

		Int_t aEast = (Int_t)ds->get( "vpd" + crate + "AdcEast", i);
		if ( aEast < 1 ) continue;

		


		if ( tEast > minTDC ){
			book->fill( "adcEast", i, aEast );
			book->fill( "tdcEast", i, tEast );
		}
		

		//cout << "[" << i << "] ADC=" << aEast << ", TAC=" << tEast << endl;
		

		if ( applyCorr )
			tEast = corrEast( i, aEast, tEast );

		if ( tEast > minTDC ){
			book->fill( "tdcEastPostSlew", i, tEast );
		}

		


		tEast = tEast - tacOffEast[ i ];
		// cout << "Offset [" << i << "] = " << tacOffEast[ i ] << endl;

		if ( tEast > minTDC ){
			book->fill( "tdcEastPostTacOff", i, tEast );
		}




		if ( applyRefCorr ){
			tEast = refCorrect( iEast, tEast, crate, "East" );
			if ( tEast >= minTDC )
				book->fill( "tdcEastPostNoise", i, tEast );
		}

		if ( tEast > minTDC && tEast > tacEast){
			tacEast = tEast;
			iEast = i;
		}

		totalEast += tEast;
		if ( tEast > 1  )
			nEast ++;		
	}


	for (int i = 0; i < 16; i++){

		if ( applyRefCorr && ( 0 == i || 1 == i || 4 == i || 8 == i  || 12 == i ) ){
			int refTac = (Int_t)ds->get( "vpd" + crate + "TdcWest", i );
			refTac -= refMeanW[ i / 4 ];
			book->fill( "tdcWestRef", i, refTac );
		}

		if ( applyRefCorr && ( 0 == i || 4 == i || 8 == i || 12 == i ) ) continue;

		if ( applyRefCorr && ( 0 == i || 4 == i || 8 == i || 12 == i ) ){
			int refTac = (Int_t)ds->get( "vpd" + crate + "TdcWest", i );
			int refCh = refMap[ i ];
			refTac -= refMeanW[ refCh / 4 ];
			book->fill( "tdcWestRef", refCh, refTac );
		}




		Int_t tWest = (Int_t)ds->get( "vpd" + crate + "TdcWest", i);

		if ( applyRefCorr ){
			tWest = refCorrect( iWest, tWest, crate, "West" );
			// if ( tWest >= minTDC )
				book->fill( "tdcWestPostNoise", i, tWest );
		}

		if (  tWest < 1 ) continue;

		Int_t aWest = (Int_t)ds->get( "vpd" + crate + "AdcWest", i);

		if (  aWest < 1 ) continue;
		
		if ( tWest > minTDC ){
			book->fill( "tdcWest", i, tWest );
			book->fill( "adcWest", i, aWest );		
		}
		
		if ( applyCorr )
			tWest = corrWest( i, aWest, tWest );

		if ( tWest > minTDC ){
			book->fill( "tdcWestPostSlew", i, tWest );
		}

		tWest = tWest - tacOffWest[ i ];

		if ( tWest > minTDC ){
			book->fill( "tdcWestPostTacOff", i, tWest );
		}

		

		
		


		if ( tWest > minTDC && tWest > tacWest){
			tacWest = tWest;
			iWest = i;
		}

		totalWest += tWest;
		if ( tWest > 1  )
			nWest ++;

	}

	if ( tacEast < minTDC || tacWest < minTDC )
		return;

	
	

	if ( -9999 ==tacEast || -9999 == tacWest ){
		cout << "invalid TAC " << tacEast << ", " << tacWest << endl;
		return;
	} else {

	}
	
	bool doResolution = config.getBool( nodePath + ".doResolution" );

	// TRACE( "fix @ 0 " << fix.eval( -5 ) );

	if ( nEast > 0  )
		tacEast = totalEast / nEast;
	if (nWest > 0 )
		tacWest = totalWest / nWest;


	double TACToNS = 0.019;
	double nsEast = tacEast * TACToNS;
	double nsWest = tacWest * TACToNS;
	
	if ( config.getBool( nodePath + ".Crate:avg", false ) ){
		nsEast = totalEast * TACToNS / nEast;
		nsWest = totalWest * TACToNS / nWest;	
	}


	double vZ = ds->get( "vertexZ" );
	double vpdVz = 29.979 * ( nsWest - nsEast) / 2.0;
	// if ( doFix ){
	// 	vpdVz = vpdVz - ( fix.eval(  ) )	
	// }


	
	if ( nEast > 0 && nWest > 0 /*&& tacEast > minTDC && tacWest > minTDC*/ ){

		// && abs(tacEast - tacWest + ( 4096 - fRes->eval( vZ ) )) < 50

		// cout << "TAC diff = " << (tacEast - tacWest + 4096) << endl;
		book->fill("TriggerVsTPC", vZ, tacEast - tacWest + 4096 );


		if ( doResolution )
			book->fill("TriggerVsTPCRes", vZ, tacEast - tacWest + ( 4096 - fRes->eval( vZ ) ) );


		book->fill( "vpdVz", 29.979 * (nsEast - nsWest) / 2.0 );
		book->fill( "vpdTACVz", tacWest - tacEast );
		book->fill( "tpcVz", vZ );
		book->fill( "deltaVz", vZ - vpdVz );
	}

}


void TriggerChecker::postEventLoop(){
	INFO( classname(), "" );

	// book->get2D( "tdcEastPostTacOff" )->FitSlicesY();
	// book->get2D( "tdcWestPostTacOff" )->FitSlicesY();

	// TH1 * hEast = (TH1*)gDirectory->Get( "tdcEastPostTacOff_1" );
	// TH1 * hWest = (TH1*)gDirectory->Get( "tdcWestPostTacOff_1" );

	// {
	// 	int total = 0;
	// 	int n = 0;
	// 	for ( int i = 0; i < 16; i++ ){
	// 		if ( 0 == i || 4 == i || 8 == i || 12 == i ) continue; 
	// 		total += hEast->GetBinContent( i+1 );
	// 		n++;
	// 	}

	// 	double mean = (double) total / n;
	// 	for ( int i = 0; i < 16; i++ ){
	// 		if ( 0 == i || 4 == i || 8 == i || 12 == i )
	// 			cout << "0, ";
	// 		else 
	// 			cout << (int)(hEast->GetBinContent( i+1 ) - mean + 0.5) << ", "; 
	// 	}
	// }
	// cout << endl << endl;
	// {
	// 	int total = 0;
	// 	int n = 0;
	// 	for ( int i = 0; i < 16; i++ ){
	// 		if ( 0 == i || 1 == i || 4 == i || 8 == i || 12 == i ) continue; 
	// 		total += hWest->GetBinContent( i+1 );
	// 		n++;
	// 	}

	// 	double mean = (double) total / n;
	// 	for ( int i = 0; i < 16; i++ ){
	// 		if ( 0 == i || 1==i|| 4 == i || 8 == i || 12 == i )
	// 			cout << "0, ";
	// 		else 
	// 			cout << (int)(hWest->GetBinContent( i+1 ) - mean + 0.5) << ", "; 
	// 	}
	// }
	// cout << endl << endl;
	// return;


	// {
	// 	int n = 0;
	// 	int total = 0;
	// 	int vals[16];
	// 	for ( int i = 0; i < 16; i++ ){
	
	// 		TH1* h1 = book->get2D( "tdcEastPostSlew" )->ProjectionY( ("temp"+ ts(i)).c_str(), i, i+1 );
			
	// 		int maxBin = h1->GetMaximumBin();
	// 		int maxVal = h1->GetBinCenter( maxBin );
	
	// 		//cout << "[" << i << "]" << maxVal << endl;
	// 		if ( 0 == i || 4 == i || 8 == i || 12 == i ) continue; 
	// 		vals[i] = maxVal;
			
	// 		total += maxVal;
	// 		n++;
	// 	}
	
	// 	cout << endl << endl;
	// 	double mean = total / (double)n;
	
	// 	cout << "<TacOffEast> ";
	// 	for ( int i = 0; i < 16; i++ ){
	// 		if ( 0 == i || 4 == i || 8 == i || 12 == i ) 
	// 			cout << 0 << ", ";
	// 		else 
	// 			cout << ((int)(vals[i] - mean + 0.5)) << ", ";
	// 	}
	// 	cout << "</TacOffEast> ";
	// }

	// {
	// 	int n = 0;
	// 	int total = 0;
	// 	int vals[16];
	// 	for ( int i = 0; i < 16; i++ ){
	
	// 		TH1* h1 = book->get2D( "tdcWestPostSlew" )->ProjectionY( ("temp"+ ts(i)).c_str(), i, i+1 );
			
	// 		int maxBin = h1->GetMaximumBin();
	// 		int maxVal = h1->GetBinCenter( maxBin );
	
	// 		// cout << "[" << i << "]" << maxVal << endl;
	// 		if ( 0 == i || 1 == i || 4 == i || 8 == i || 12 == i ) continue; 
	// 		vals[i] = maxVal;
			
	// 		total += maxVal;
	// 		n++;
	// 	}
	
	// 	cout << endl << endl;
	// 	double mean = total / (double)n;
	
	// 	cout << "<TacOffWest> ";
	// 	for ( int i = 0; i < 16; i++ ){
	// 		if ( 0 == i || 1 == i || 4 == i || 8 == i || 12 == i ) 
	// 			cout << 0 << ", ";
	// 		else 
	// 			cout << ((int)(vals[i] - mean + 0.5)) << ", ";
	// 	}
	// 	cout << "</TacOffWest> " << endl << endl;
	// }

	// return;


	
	gStyle->SetOptStat(0);

	reporter->newPage();

	/*book->style( "vpdVz" )->draw();
	reporter->savePage();

	book->style( "tpcVz" )->draw();
	reporter->savePage();
	*/

	RooPlotLib rpl;
	rpl.style( book->get("TriggerVsTPC")).set("draw", "colz").draw();
	TAxis * x = book->get("TriggerVsTPC")->GetXaxis();
	TAxis * y = book->get("TriggerVsTPC")->GetYaxis();
	TLine * tl = new TLine( x->GetXmin(), 4096, x->GetXmax(), 4096);
	tl->Draw("same");

	TLine * tl2 = new TLine( 0, y->GetXmin(), 0, y->GetXmax());
	tl2->Draw("same");

	reporter->savePage();

	gStyle->SetOptFit(1111);
	TProfile * tp = book->get2D("TriggerVsTPC")->ProfileX();

	TF1 * poly = new TF1( "poly", "pol1", -10, 10 );
	poly->SetParameters( 4096, -4.5 );



	// tp->GetXaxis()->SetRangeUser( -10, 10 );
	// tp->Draw();
	// tp->Fit( pol1 );
	// tp->Fit( pol1 );

	
	// tp->Draw( "same" );

	book->get2D("TriggerVsTPC")->FitSlicesY( 0, 0, -1, 5 );
	TH1 * hs = nullptr;

	gDirectory->GetObject( "TriggerVsTPC_1", hs );
	hs->GetXaxis()->SetRangeUser( -10, 10 );
	hs->Fit( poly );
	
	hs->GetXaxis()->SetRangeUser( -30, 30 );
	poly->SetRange( -30, 30 );
	rpl.style( book->get("TriggerVsTPC")).set("draw", "colz").draw();
	hs->Draw("same");
	poly->Draw( "same" );


	reporter->savePage();


	INFO( "Vpd +/- 5cm" )
	INFO( "" << poly->Eval( 5 ) << " -> " << poly->Eval( -5 ) );

	INFO( "Vpd +/- 10cm" )
	INFO( "" << poly->Eval( 10 ) << " -> " << poly->Eval( -10 ) );

	INFO( "Vpd +/- 30cm" )
	INFO( "" << poly->Eval( 30 ) << " -> " << poly->Eval( -30 ) );

	INFO( "Vpd +/- 50cm" )
	INFO( "" << poly->Eval( 50 ) << " -> " << poly->Eval( -50 ) );

	INFO( "Vpd +/- 70cm" )
	INFO( "" << poly->Eval( 70 ) << " -> " << poly->Eval( -70 ) );

	INFO( "Vpd +/- 100cm" )
	INFO( "" << poly->Eval( 100 ) << " -> " << poly->Eval( -100 ) );

	cout << "\n\n" << XmlFunction::toXml( poly ) << endl;


	






	// reporter->newPage();


	// book->style( "deltaVz" )->set( "domain", -175, 175 )->draw();
	// book->get( "deltaVz" )->Fit( "gaus" );

	// reporter->savePage();
	

	reporter->close();


	
}

int TriggerChecker::refCorrect( int iCh, int tac, string crate, string side ) {
	
	int refCh = refMap[ iCh ];
	if ( refCh <= -1 ) return -9999;
	int refTac = (Int_t)ds->get( "vpd" + crate + "Tdc" + side, refCh);
	if ( refTac <= 0 ) return -9999;

	if ( "East" == side ){
		refTac -= refMeanE[ refCh / 4 ];
	} else if ( "West" == side ){
		refTac -= refMeanW[ refCh / 4 ];
	}

	INFO( classname(), side );
	INFO( classname(), "[" << iCh << "] = " << tac );
	INFO( classname(), "refCh = " << refCh );
	INFO( classname(), "refTac = " << refTac );
	INFO( classname(), "Tac = " << tac - refTac );

	// book->fill( "tdc" + side + "Ref", iCh, refTac );

	return tac - refTac;
}



void TriggerChecker::readParams(){

	ifstream inf( config.getString( nodePath + ".Slewing:url" ).c_str() );

	for ( int i = 0; i < 19; i ++ ){
		for ( int j = 0; j < 10; j++){
			eBinEdges[ i ][ j ] = 0;
			wBinEdges[ i ][ j ] = 0;
			eCorrs[ i ][ j ] = 0;
			wCorrs[ i ][ j ] = 0;
		}
	}


	if ( !inf.good() ){
		ERROR( "", "BAD SLEWING" );
		return;
	}

	int iEast = 0;
	int iWest = 0;

	string line;
	while( getline( inf, line ) ){

		cout << line << endl;
		stringstream ss( line );

		string boardId;

		ss >> boardId;

		if ( "0x16" == boardId || "0x18" == boardId ){

			int channel;
			int nBins;
			int opt;

			ss >> channel >> nBins >> opt;
			cout << "nBins " << nBins << endl;

			numBins = nBins;
			
			if ( 0 == opt ) {// bin edges
				for ( int i = 0; i < nBins; i++ ){
					int be;
					ss >> be;
					cout << "[" << i << "] = " << be << endl;
					if ( "0x16" == boardId ) // east
						eBinEdges[iEast][i] = be;
					else if ("0x18" == boardId) // west
						wBinEdges[iWest][i] = be;
				}
			} else if ( 1 == opt ) {// bin corrs
				for ( int i = 0; i < nBins; i++ ){
					int bc;
					ss >> bc;
					cout << "[" << i << "] = " << bc << endl;
					if ( "0x16" == boardId ) // east
						eCorrs[iEast][i] = bc;
					else if ("0x18" == boardId) // west
						wCorrs[iWest][i] = bc;
				}
			}
			if ( "0x16" == boardId && 1 == opt ) // east
				iEast ++;
			else if ("0x18" == boardId && 1 == opt ) // west
				iWest ++;	
		}

	}

	inf.close();

}



int TriggerChecker::corrBin( int iCh, int adc, int *bins ){

	// find the bin
	for ( int iBin = 0; iBin < numBins; iBin++){

		if ( 0 == iBin && adc >= 0 && adc <= bins[ 0 ] ){
			return 0;
		} else {
			if ( adc > bins[ iBin - 1 ] && adc <= bins[ iBin ] )
				return iBin;
		}
	}
	return -1;
}


int TriggerChecker::corrEast( int iCh, int adc, int tac ){
	int bin = corrBin( iCh, adc, eBinEdges[ iCh ] );
	// cout << "East CORR = " << eCorrs[ iCh ][ bin ] << endl;
	return tac + eCorrs[ iCh ][ bin ];
}

int TriggerChecker::corrWest( int iCh, int adc, int tac ){
	int bin = corrBin( iCh, adc, wBinEdges[ iCh ] );
	// cout << "West CORR = " << wCorrs[ iCh ][ bin ] << endl;
	return tac + wCorrs[ iCh ][ bin ];
}




