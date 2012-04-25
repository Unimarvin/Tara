/*!
\file Handlers.cc
\author Niels
\status new

\brief Implementation of exit and termination handlers.
*/

#include <config.h>
#include <string>
#include <pthread.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "cmdline.h"
#include "Core/Dimensions.h"
#include "Core/Handlers.h"
#include "Net/Net.h"
#include "Net/Place.h"
#include "Net/Marking.h"
#include "Net/Transition.h"
#include "InputOutput/Socket.h"
#include "InputOutput/Reporter.h"
#include "InputOutput/SimpleString.h"

extern Reporter* rep;
extern gengetopt_args_info args_info;

pthread_t Handlers::terminationHandler_thread;


/*!
\param signum The signal to react to.
\note This function is only called for those signals that have been registered
      by calling signal() first - see Handlers::installTerminationHandlers().
*/
__attribute__((noreturn)) void Handlers::signalTerminationHandler(int signum)
{
    rep->message("caught termination signal: '%s'", strsignal(signum));
    exit(EXIT_TERMINATION);
}


/*!
The termination handler allows to terminate LoLA by sending a predefined
secret via socket. Once the message is received, LoLA's execution is
terminated by calling sending the SIGUSR1 signal which is processed in
Handlers::signalTerminationHandler().
*/
void* Handlers::remoteTerminationHandler(void*)
{
    Socket listener_socket(args_info.inputport_arg);
    char* sender = listener_socket.waitFor(args_info.remoteTermination_arg);
    assert(sender);
    rep->message("received %s packet (%s) from %s - shutting down", rep->markup(MARKUP_BAD, "KILL").str(), rep->markup(MARKUP_IMPORTANT, args_info.remoteTermination_arg).str(), rep->markup(MARKUP_FILE, sender).str());
    free(sender);

    // abort LoLA by sending SIGUSR1 signal
    kill(getpid(), SIGUSR1);
    return NULL;
}


/*!
The exit handler allows to organize the termination of LoLA. This includes
closing files, reporting exit, and releasing memory.
*/
void Handlers::exitHandler()
{
    // print statistics
    if (args_info.stats_flag) {
        std::string call = std::string("ps -o rss -o comm | ") + TOOL_GREP + " " + PACKAGE + " | " + TOOL_AWK + " '{ if ($1 > max) max = $1 } END { print max \" KB\" }'";
        FILE* ps = popen(call.c_str(), "r");
        unsigned int memory;
        int res = fscanf(ps, "%u", &memory);
        assert(res != EOF);
        pclose(ps);
        rep->message("memory consumption: %u KB", memory);
    }

    // release memory from command line parser
    cmdline_parser_free(&args_info);

    // shut down killer thread
    if (args_info.remoteTermination_given)
    {
        const int ret = pthread_cancel(terminationHandler_thread);
        if (ret == 0)
        {
            rep->status("killed listener thread");
        }
    }

    // tidy up
    Net::deleteNodes();
    Place::deletePlaces();
    Transition::deleteTransitions();
    Marking::deleteMarkings();

    // should be the very last call
    delete rep;
}


void Handlers::installExitHandler()
{
    // set the function to call on normal termination
    atexit(exitHandler);
}


void Handlers::installTerminationHandlers()
{
    // listen to software termination signal (kill)
    signal(SIGTERM, signalTerminationHandler);
    // listen to interrupt program (ctrl+c)
    signal(SIGINT, signalTerminationHandler);
    // listen to user-defined signal 1 (used for remote termination)
    signal(SIGUSR1, signalTerminationHandler);

    // start up listener thread
    if (args_info.remoteTermination_given)
    {
        rep->status("setting up listener socket at port %s - secret is %s", rep->markup(MARKUP_FILE, "%d", args_info.inputport_arg).str(), rep->markup(MARKUP_IMPORTANT, args_info.remoteTermination_arg).str());

        const int ret = pthread_create(&terminationHandler_thread, NULL, remoteTerminationHandler, NULL);
        if (UNLIKELY(ret != 0))
        {
            rep->status("thread could not be created");
            rep->abort(ERROR_THREADING);
        }
    }
}