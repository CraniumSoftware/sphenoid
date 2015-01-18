// Copyright (c) 2015 Cranium Software

#include "CommandParser.h"
#include "Connection.h"

int main( const int /*iArgumentCount*/, const char* const* const /*pszArguments*/ )
{
    if( Connection::Initiate( "Sphenoid", "#sphtest", "us.quakenet.org", "6667" ) == 0 )
    {
        CommandParser::Initialise();

        Connection::MainLoop();

        Connection::Terminate();
        CommandParser::Shutdown();
    }

    return 0;
}
