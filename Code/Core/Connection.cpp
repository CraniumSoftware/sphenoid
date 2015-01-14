#include "Connection.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>

#include <sys/types.h>

// SE - TODO: something better?
#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment( lib, "ws2_32" )

static WSAData gxWSAData;
#endif

long long Connection::sllSocket = -1;
bool( *Connection::spfnMessageCallback )( const char* const szWholeMessage, const int iLength );

// SE - TODO: something better. initiate should probably get you all the way into the channel
static const char* gszChannel = 0;

int Connection::Initiate( const char* const szNick, const char* const szChannel, const char* const szHost, const char* const szPort )
{
    addrinfo xHints;
    addrinfo* pxAddress = 0;

#ifdef WIN32

    WSAStartup( MAKEWORD( 2, 3 ), &gxWSAData );

#endif

    memset( &xHints, 0, sizeof( addrinfo ) );
    xHints.ai_family = AF_INET;
    xHints.ai_socktype = SOCK_STREAM;
    if( getaddrinfo( szHost, szPort, &xHints, &pxAddress ) == 0 )
    {
        sllSocket = socket( pxAddress->ai_family, pxAddress->ai_socktype, pxAddress->ai_protocol );
        connect( sllSocket, pxAddress->ai_addr, static_cast< int >( pxAddress->ai_addrlen ) );

        Send( "USER %s 0 0 :%s", szNick, szNick );
        Send( "NICK %s", szNick );

        gszChannel = szChannel;
    }
    else
    {
        return -1;
    }

    return 0;
}

void Connection::Terminate()
{
    closesocket( static_cast< SOCKET >( sllSocket ) );

#ifdef WIN32

    WSACleanup();

#endif
}

void Connection::MainLoop()
{
    // SE - NOTE: this is manifestly a single threaded thing.
    // should add some thread-safe queues for more flexible communications
    // both in and out of this loop.
    char szBuffer[ 1024 ];
    char szWorkBuffer[ 4096 ];
    int iLength = 0;
    int iWorkCursor = 0;
    bool bDiscard = false;
    while( iLength != SOCKET_ERROR )
    {
        iLength = recv( sllSocket, szBuffer, 1024, 0 );

        for( int i = 0; i < iLength; ++i )
        {
            szWorkBuffer[ iWorkCursor ] = szBuffer[ i ];
            ++iWorkCursor;

            // check for message is far too long.
            if( iWorkCursor >= 4095 )
            {
                printf( "Error: Recieved excessively long message - it will be discarded" );
                bDiscard = true;
                iWorkCursor = 0;
                continue;
            }

            // check for new lines ending our message
            if( szBuffer[ i ] == '\r' && szBuffer[ i + 1 ] == '\n' )
            {
                if( !bDiscard )
                {
                    bDiscard = false;

                    // this is the end of a message
                    szWorkBuffer[ iWorkCursor ] = szBuffer[ i + 1 ];
                    szWorkBuffer[ iWorkCursor + 1 ] = 0;
                    printf( "RECIEVE: %s", szWorkBuffer );

                    // reply PING with PONG
                    if( !strncmp( szWorkBuffer, "PING", 4 ) )
                    {
                        szBuffer[ 1 ] = 'O';
                        Send( szBuffer );
                    }
                    else
                    {
                        // check if we should join the channel.
                        // 001 is the first response, so ask then...
                        const char* szSpaceCursor = szWorkBuffer;
                        while( *szSpaceCursor && ( *szSpaceCursor != ' ' ) )
                        {
                            ++szSpaceCursor;
                        }

                        if( !strncmp( szSpaceCursor + 1, "001", 3 ) )
                        {
                            Send( "JOIN %s", gszChannel );
                        }
                        else if( spfnMessageCallback )
                        {
                            spfnMessageCallback( szWorkBuffer, iWorkCursor );
                        }
                    }
                }

                iWorkCursor = 0;
                ++i;
            }
        }
    }
}

int Connection::Send( const char* const szFormatString, ... )
{
    char szMessageBuffer[ 1024 ];
    va_list xVAList;
    va_start( xVAList, szFormatString );
    const int iReturnValue = vsnprintf( szMessageBuffer, 1024, szFormatString, xVAList );
    va_end( xVAList );

    strcat( szMessageBuffer, "\r\n" );

    printf( "SENDING: %s", szMessageBuffer );

    send( sllSocket, szMessageBuffer, static_cast< int >( strlen( szMessageBuffer ) ), 0 );

    return iReturnValue;
}
