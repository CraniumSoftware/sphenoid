// Copyright (c) 2015 Cranium Software

#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

class CommandParser
{

public:

    static bool Initialise();
    static bool Shutdown();

    static void RegisterChatCallback( void( *pfnChatCallback )( const char* const szSender, const char* const szRecipient, const char* const szMessage, const int iLength ) )
    {
        // SE - TODO: some std::vector list of these? std::function?
        spfnChatCallback = pfnChatCallback;
    }

private:

    static const char* AdvanceToUser( const char* const szResponse );
    static const char* AdvanceToCommand( const char* const szResponse );
    static const char* AdvanceToContent( const char* const szResponse );

    static bool MessageHandler( const char* const szWholeMessage, const int iLength );

    static void ( *spfnChatCallback )( const char* const szSender, const char* const szRecipient, const char* const szMessage, const int iLength );

};

#endif
