//////////////////////////////////////////////////////////////////////
//
// Filename    : main.cpp
// Written By  : reiot@ewestsoft.com
// Description : Main function for the login server
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
    // any thread can receive one.
    registerSharedServerPacketHandlers();

    if (argc < 3) {
        cout << "Usage : sharedserver -f È¯°æÆÄÀÏ" << endl;
        exit(1);
    }

    // Convert the command-line parameters into strings.
    string* Argv;

    Argv = new string[argc];
    for (int i = 0; i < argc; i++)
        Argv[i] = argv[i];

    // Read the configuration file.
    // The executable must live in $VSHOME/bin and the configuration file in $VSHOME/conf.
    // Allow the configuration file to be given on the command line.

    try {
        if (Argv[1] != "-f") {
            throw Error("Usage : sharedserver -f config-file");
        }

        // When the first parameter is -f, the second is the path of the configuration file.
        g_pConfig = new Properties();
        g_pConfig->load(Argv[2]);

        cout << g_pConfig->toString() << endl;

    } catch (Error& e) {
        cout << e.toString() << endl;
    }

    // Create the log manager, initialize it and activate it.
    // The log manager has to catch even the errors that can occur while the login server
    // initializes, so it must not be initialized inside the login server.
    // It also has to be created and initialized before any other object is created
    // and initialized.

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
    // Create the login server object, initialize it and activate it.
    //
    try {
        struct rlimit rl;
        rl.rlim_cur = RLIM_INFINITY;
        rl.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_CORE, &rl);

        // Create the login server object.
        g_pSharedServer = new SharedServer();

        // Initialize the login server object.
        g_pSharedServer->init();

        // Activate the login server object.
        if (!ServerShutdown::isRequested())
            g_pSharedServer->start();
    } catch (Throwable& e) {
        // In case the server ends before logging is up
        ofstream ofile("../log/instant.log", ios::out);
        ofile << e.toString() << endl;
        ofile.close();

        // It means an exception or error not caught below occurred.
        // In that case it must be logged at LEVEL1 (that is, logged unconditionally).
        log(LOG_SHAREDSERVER_ERROR, "", "", e.toString());

        // Print it on standard output too.
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
