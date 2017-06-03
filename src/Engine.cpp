
#include "XmlConfig.h"
using namespace jdb;


#include <iostream>
#include <exception>

#include "GainChecker.h"
#include "TriggerChecker.h"

int main( int argc, char* argv[] ) {

	Logger::setGlobalLogLevel( "all" );

	if ( argc >= 2 ){

		string fileList = "";
		string jobPrefix = "";

		// try block to load the XML config if given
		try{
			XmlConfig config( argv[ 1 ] );
			
			// if more arguments are given they will be used for 
			// parallel job running with examples of
			// argv[ 2 ] = fileList.lis
			// argv[ 3 ] = jobPrefix_ 
			if ( argc >= 4){
				fileList = (string) argv[ 2 ];
				jobPrefix = (string) argv[ 3 ];
			}
			string job = config.getString( "Job", "" );

			if ( "TriggerChecker" == job ){
				TriggerChecker tc;
				tc.init( config, "TriggerChecker." );
				tc.make();

			} else {
				GainChecker gc;
				gc.init( config, "GainChecker." );
				gc.make();	
			}


		} catch ( exception &e ){
			cout << e.what() << endl;
		}

	}
	return 0;
}
