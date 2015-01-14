// Copyright (c) 2015 Cranium Software

#ifndef CONNECTION_H
#define CONNECTION_H

class Connection
{

public:

    static int Initiate( const char* const szNick, const char* const szChannel, const char* const szHost, const char* const szPort );
    static void Terminate();

    static void MainLoop();

    // SE - TODO: some std::vector list of these? std::function?
    static void RegisterCallback( bool( *const pfnMessageCallback )( const char* const szWholeMessage, const int iLength ) )
    {
        spfnMessageCallback = pfnMessageCallback;
    }

    static int Send( const char* const szFormatString, ... );

private:

    static long long sllSocket;
    static bool( *spfnMessageCallback )( const char* const szWholeMessage, const int iLength );
};

#endif
