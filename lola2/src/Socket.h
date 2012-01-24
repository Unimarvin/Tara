/*!
\file Socket.h
\author Niels
\status new
\ingroup g_reporting

\brief Socket class definition
*/

#pragma once

#include <config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*!
\brief class encapsulating Berkely Sockets (using UDP datagrams)

\author Niels
\status new
\ingroup g_reporting
*/
class Socket
{
    private:
        /// the buffer size for incoming messages
        static size_t bufferSize;

        /// the length of the address struct
        static socklen_t addressLength;

    private:
        /// the socket
        int sock;

        /// the address
        sockaddr_in address;

        /// whether we are listening (server) or sending (client)
        bool listening;

        /// a buffer for incoming messages
        char* buffer;

    public:
        /// create a socket - port is mandatory, destination address optional
        Socket(u_short port, const char* destination = NULL);

        /// close the socket
        ~Socket();

        /// receive incoming messages (does not return)
        void receive();

        /// send a message
        void send(const char* message) const;
};