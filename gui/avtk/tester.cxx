/*
 * Copyright(c) 2016, OpenAV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENAV BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifdef AVTK_TESTER

#include "tester.hxx"

#include "ui.hxx"
//#include "picojson.hxx"

// for converting events -> JSON
#include "pugl/pugl.h"

#include <list>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


namespace Avtk
{

Tester::Tester( Avtk::UI* ui_ ) :
	ui( ui_ ),
	playing_( false ),
	recording_( false ),
	startRecTime( 0 ),
	startPlayTime( 0 ),
	playEventNum( 0 )
{
}

void Tester::record( const char* n )
{
	playing_ = false;
	recording_ = true;
	name = n;
	startRecTime = getTime();
	printf("%s , %lf\n", __PRETTY_FUNCTION__, startRecTime );
	events.clear();
}

void Tester::handle( const PuglEvent* event )
{
	if( !playing_ ) {
		if( event->type != PUGL_MOTION_NOTIFY ) {
			events.push_back( AvtkEvent( event, getTime() - startRecTime ) );
		}
	}
}

void Tester::writeTest( const char* filename )
{
	printf("%s %s : STUB\n", __FILE__, __PRETTY_FUNCTION__ );
	/*
	picojson::object test;
	picojson::object event;

	test["name"] = picojson::value("Test name");
	test["avtkVersion"] = picojson::value( AVTK_VERSION_STRING );

	picojson::array list = picojson::array();

	AvtkEvent* e = &events.at(0);
	for(int i = 0; i < events.size(); i++ )
	{
	  event["eventType"] =  picojson::value( puglEventGetTypeString( e->event.type ) );
	  event["integer"] =  picojson::value( 1.0 );


	  list.push_back( picojson::value(event) );

	  //printf("Event %i, type %i\n", i, e->event.type );
	  e++;
	}

	test["events"] =  picojson::value( list );

	std::string str = picojson::value(test).serialize();
	//printf("Tester::writeTest() %s\n", str.c_str() );

	std::ofstream out;
	out.open ( filename );
	out << str << "\n";
	out.close();
	*/
}

void Tester::recordStop()
{
	if( !events.size() ) {
		printf("No events recorded.\n" );
		return;
	}
	AvtkEvent* e = &events.at(0);

	for(int i = 0; i < events.size(); i++ ) {
		printf("Event %i, type %i\n", i, e->event.type );
		e++;
	}

	writeTest( "avtk_test.json" );

	printf("%s stopping AVTK TEST %s\n", __PRETTY_FUNCTION__, name.c_str() );
	recording_ = false;
}

void Tester::process()
{
	if( playing_ ) {
		if ( playEventNum < events.size() ) {
			if( events.at( playEventNum ).timestamp < getTime() - startPlayTime ) {
				ui->event( &events.at( playEventNum ).event );
				playEventNum++;
			}
		} else {
			playing_   = 0;
			recording_ = 0;
		}
	}
}

int Tester::runTest( const char* testName, bool ignoreTimestamps )
{
	playing_ = true;

	// get the time of the play start
	startPlayTime = getTime();

	if( recording_ )
		recordStop();

	playEventNum = 0;

	/*
	printf("%s running test %s : num events %i\n", __PRETTY_FUNCTION__, testName, events.size() );

	if( events.size() > 0 )
	{
	  //AvtkEvent* e = &events.at(0);
	  for(int i = 0; i < events.size(); i++ )
	  {
	    //printf("Event %i, type %i\n", i, events.at(0).event.type );


	  }
	}
	*/

	return 0;

	/*
	try
	{
	  std::ifstream ifs;
	  ifs.open ( testName, std::ifstream::in);

	  picojson::value v;
	  ifs >> v;

	  if( ifs.fail() )
	  {
	    printf("Theme::load() %s : File doesn't exist, abort.\n", testName );
	    //std::cerr << picojson::get_last_error() << std::endl;
	    return -1;
	  }

	  const char* items[5] =
	  {
	    "bg",
	    "bg-dark",
	    "fg",
	    "fg-dark",
	    "highlight"
	  };

	  for( int i = 0; i < 5; i++ )
	  {
	    // extract the 3 ints from the array, and store into Color array
	    int colNum = 0;
	    picojson::array list = v.get( items[i] ).get<picojson::array>();
	    for (picojson::array::iterator iter = list.begin(); iter != list.end(); ++iter)
	    {
	      int tmp = (int)(*iter).get("c").get<double>();
	      //printf("%s = %lf\r\n", items[i], tmp );
	      ///colors[i].c[colNum++] = tmp;
	    }
	  }
	}
	catch( ... )
	{
	  printf("Theme::load() Error loading theme from %s : falling back to default.Double check file-exists and JSON contents valid.\n", testName );
	  // *any* error, and we don't use the theme
	  return -1;
	}
	*/
}

double Tester::getTime()
{
#ifdef _WIN32
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	double time = count.QuadPart * m_reciprocalFrequency;
#else
	struct timespec interval;
	clock_gettime(CLOCK_MONOTONIC, &interval);
	double time = interval.tv_sec + interval.tv_nsec * 0.000000001f;
#endif

	return time;
};

}; // Avtk

#endif // AVTK_TESTER
