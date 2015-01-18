// Copyright (c) 2015 Cranium Software

#include "CommandParser.h"

#include "Client/Connection.h"

#include <cstring>

void ( *CommandParser::spfnChatCallback )( const char* const szSender, const char* const szRecipient, const char* const szMessage, const int iLength );

// SE : to save the null termination gotcha from adding lots of little lines of code...
void strncpy_notshit( char* const szDestination, const char* const szSource, const int iStringLength )
{
    strncpy( szDestination, szSource, iStringLength );
    szDestination[ iStringLength ] = 0;
}

bool CommandParser::Initialise()
{
    // SE - TODO: some more flexible architecture?
    // make this the message handler for the connection

    Connection::RegisterCallback( MessageHandler );

    return true;
}

bool CommandParser::Shutdown()
{
    return true;
}

const char* CommandParser::AdvanceToUser( const char* const szResponse )
{
    if( strncmp( szResponse, ":", 1 ) == 0 )
    {
        return szResponse + 1;
    }

    return 0;
}

const char* CommandParser::AdvanceToCommand( const char* const szResponse )
{
    if( strncmp( szResponse, "NOTICE", 6 ) == 0 )
    {
        // SE: don't think we care about notice...
        return szResponse;
    }

    if( strncmp( szResponse, ":", 1 ) == 0 )
    {
        // SE - TODO: make safe against overrun
        // skip past the user to the command
        const char* szCursor = szResponse + 1;
        while( *szCursor != ' ' )
        {
            ++szCursor;
        }
        ++szCursor;

        return szCursor;
    }
    
    return 0;
}

const char* CommandParser::AdvanceToContent( const char* const szResponse )
{
    const char* const szCommand = AdvanceToCommand( szResponse );
    if( szCommand == 0 )
    {
        return 0;
    }
    else if( strncmp( szCommand, "PRIVMSG", 7 ) == 0 )
    {
        // this covers queries and channel messages...
        return szCommand + 8;
    }

    return 0;
}

bool CommandParser::MessageHandler( const char* const szWholeMessage, const int /*iLength*/ )
{
    const char* const szCommand = AdvanceToCommand( szWholeMessage );

    if( strncmp( szCommand, "NOTICE", 6 ) == 0 )
    {
        // SE: don't think we care about notice...
        return false;
    }

    if( strncmp( szCommand, "PRIVMSG", 7 ) == 0 )
    {
        if( spfnChatCallback )
        {
            const char* szUser = AdvanceToUser( szWholeMessage );
            const char* szContent = AdvanceToContent( szWholeMessage );

            const int iUserLength = static_cast< int >( szUser - szCommand ) - 1;
            int iTargetLength = 0;

            // so the content is the target then a colon and a message.
            const char* szTarget = szContent;
            while( szTarget[ iTargetLength ] != ':' )
            {
                ++iTargetLength;
            }

            const char* szMessage = szTarget + iTargetLength + 1;
            const int iMessageLength = static_cast< int >( strlen( szTarget ) ) - 2; // drop \r\n
        
            // copy everything for the callback to be called...

            char* szUserCopy = new char[ iUserLength + 1 ];
            char* szTargetCopy = new char[ iTargetLength + 1 ];
            char* szMessageCopy = new char[ iMessageLength + 1 ];

            strncpy_notshit( szUserCopy, szUser, iUserLength );
            strncpy_notshit( szTargetCopy, szTarget, iTargetLength );
            strncpy_notshit( szMessageCopy, szMessage, iMessageLength );

            spfnChatCallback( szUser, szTarget, szMessage, iMessageLength );
        }
    }

    return false;
}
