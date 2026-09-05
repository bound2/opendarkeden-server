//////////////////////////////////////////////////////////////////////
//
// Filename    : main.cpp
// Written By  : reiot@ewestsoft.com
// Description : ·Î±×ÀÎ ¼­¹ö¿ë ¸ÞÀÎ ÇÔ¼ö
//
//////////////////////////////////////////////////////////////////////

// include files
#include <stdlib.h>

#include <chrono>
#include <exception>
#include <new>

#include <sys/resource.h>

#include "Exception.h"
#include "LogClient.h"
#include "Properties.h"
#include "ServerShutdown.h"
#include "SharedPacketDispatch.h"
#include "SharedServer.h"
#include "StringStream.h"
#include "Types.h"

void memoryError() {
    cout << "CRITICAL ERROR! NOT ENOUGH MEMORY!" << endl;
    exit(0);
}

//////////////////////////////////////////////////////////////////////
//
// main()
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    // SIGTERM/SIGINT only store a lock-free request; the main heartbeat loop
    // and every worker observe it on their next turn.
    struct sigaction action {};
    action.sa_handler = ServerShutdown::request;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGTERM, &action, nullptr) != 0 || sigaction(SIGINT, &action, nullptr) != 0)
        return EXIT_FAILURE;
    // Armed before initialization so a startup that blocks still exits.
    ServerShutdown::Deadline shutdownDeadline(std::chrono::seconds(30), "sharedserver");

    // Set the out-of-memory handler.
    set_new_handler(memoryError);

    // Bind every packet id the sharedserver receives to its handler before
    // any thread can receive one (docs/RESTRUCTURING.md task 2.3).
    registerSharedServerPacketHandlers();

    if (argc < 3) {
        cout << "Usage : sharedserver -f È¯°æÆÄÀÏ" << endl;
        exit(1);
    }

    // command-line parameter¸¦ string À¸·Î º¯È¯ÇÑ´Ù. ^^;
    string* Argv;

    Argv = new string[argc];
    for (int i = 0; i < argc; i++)
        Argv[i] = argv[i];

    // È¯°æ ÆÄÀÏÀ» ÀÐ¾îµéÀÎ´Ù.
    // ´Ü ½ÇÇà ÆÄÀÏÀº $VSHOME/bin¿¡, È¯°æ ÆÄÀÏÀº $VSHOME/conf ¿¡ Á¸ÀçÇØ¾ß ÇÑ´Ù.½
    // command line ¿¡¼­ È¯°æ ÆÄÀÏÀ» ÁöÁ¤ÇÒ ¼ö ÀÖµµ·Ï ÇÑ´Ù.

    try {
        if (Argv[1] != "-f") {
            throw Error("Usage : sharedserver -f È¯°æÆÄÀÏ");
        }

        // Ã¹¹øÂ° ÆÄ¶ó¹ÌÅÍ°¡ -f ÀÏ °æ¿ì, µÎ¹øÂ° ÆÄ¶ó¹ÌÅÍ´Â È¯°æÆÄÀÏÀÇ À§Ä¡°¡ µÈ´Ù.
        g_pConfig = new Properties();
        g_pConfig->load(Argv[2]);

        cout << g_pConfig->toString() << endl;

    } catch (Error& e) {
        cout << e.toString() << endl;
    }

    // ·Î±× ¸Å´ÏÀú¸¦ »ý¼ºÇÏ°í ÃÊ±âÈ­ÇÑÈÄ È°¼ºÈ­½ÃÅ²´Ù.
    // ·Î±× ¸Å´ÏÀú´Â ·Î±×ÀÎ ¼­¹öÀÇ ÃÊ±âÈ­°úÁ¤¿¡¼­ ¹ß»ýÇÒ °¡´É¼ºÀÌ ÀÖ´Â ¿¡·¯±îÁöµµ
    // °ËÃâÇØ³»¾ß ÇÏ¹Ç·Î ·Î±×ÀÎ ¼­¹ö ³»ºÎ¿¡¼­ ÃÊ±âÈ­ÇØ¼­´Â ¾ÈµÈ´Ù.
    // ¶ÇÇÑ ´Ù¸¥ °´Ã¼¸¦ »ý¼ºÇÏ°í ÃÊ±âÈ­ÇÏ±âÀü¿¡ ·Î±×¸Å´ÏÀú°¡ ¿ì¼±ÀûÀ¸·Î »ý¼º,
    // ÃÊ±âÈ­µÇ¾î¾ß ÇÑ´Ù.

    try {
        string LogServerIP = g_pConfig->getProperty("LogServerIP");
        int LogServerPort = g_pConfig->getPropertyInt("LogServerPort");
        g_pLogClient = new LogClient(LogServerIP, LogServerPort);
        LogClient::setLogLevel(g_pConfig->getPropertyInt("LogLevel"));

        log(LOG_SHAREDSERVER, "", "", "Shared Server Start");
    } catch (Throwable& t) {
        cout << t.toString() << endl;
    }

    //
    // ·Î±×ÀÎ ¼­¹ö °´Ã¼¸¦ »ý¼ºÇÏ°í ÃÊ±âÈ­ÇÑ ÈÄ È°¼ºÈ­½ÃÅ²´Ù.
    //
    try {
        struct rlimit rl;
        rl.rlim_cur = RLIM_INFINITY;
        rl.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_CORE, &rl);

        // ·Î±×ÀÎ ¼­¹ö °´Ã¼¸¦ »ý¼ºÇÑ´Ù.
        g_pSharedServer = new SharedServer();

        // ·Î±×ÀÎ ¼­¹ö °´Ã¼¸¦ ÃÊ±âÈ­ÇÑ´Ù.
        g_pSharedServer->init();

        // ·Î±×ÀÎ ¼­¹ö °´Ã¼¸¦ È°¼ºÈ­½ÃÅ²´Ù.
        if (!ServerShutdown::isRequested())
            g_pSharedServer->start();
    } catch (Throwable& e) {
        // ·Î±×°¡ ÀÌ·ïÁö±â Àü¿¡ ¼­¹ö°¡ ³¡³¯ °æ¿ì¸¦ ´ëºñÇØ¼­
        ofstream ofile("../log/instant.log", ios::out);
        ofile << e.toString() << endl;
        ofile.close();

        // ÇÏÀ§¿¡¼­ Ä³Ä¡µÇÁö ¾ÊÀº ¿¹¿Ü ¶Ç´Â ¿¡·¯°¡ ¹ß»ýÇß´Ù´Â ¶æÀÌ´Ù.
        // ÀÌ °æ¿ì LEVEL1·Î ·Î±×ÇØ¾ß ÇÑ´Ù. (¹«Á¶°Ç ·Î±×ÇÑ´Ù´Â ¶æ)
        log(LOG_SHAREDSERVER_ERROR, "", "", e.toString());

        // Ç¥ÁØ Ãâ·ÂÀ¸·Îµµ Ãâ·ÂÇØÁØ´Ù.
        cout << e.toString() << endl;

        // Stop the shared server; every sub-manager has to stop with it.
        ServerShutdown::fail();
    } catch (...) {
        cout << "unknown exception..." << endl;
        ServerShutdown::fail();
    }

    // Both the signal-driven and the failed-startup paths reach the same
    // teardown: request the stop, then join every worker while the managers
    // it uses are still alive.
    ServerShutdown::request();
    bool drained = true;
    try {
        if (g_pSharedServer != NULL)
            g_pSharedServer->stop();
    } catch (Throwable& error) {
        drained = false;
        ServerShutdown::fail();
        cerr << "Shutdown failed: " << error.toString() << endl;
    } catch (const std::exception& error) {
        drained = false;
        ServerShutdown::fail();
        cerr << "Shutdown failed: " << error.what() << endl;
    } catch (...) {
        drained = false;
        ServerShutdown::fail();
        cerr << "Shutdown failed: unknown exception" << endl;
    }
    // The legacy singleton graph has no audited destruction order, so let the
    // OS reclaim it once every worker has joined.
    if (drained)
        cout << ">>> ALL SHARED WORKERS STOPPED." << endl;
    cout.flush();
    cerr.flush();
    std::_Exit(ServerShutdown::failed.load() ? EXIT_FAILURE : EXIT_SUCCESS);
}
