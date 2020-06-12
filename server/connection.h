#ifndef INC_5042_CONNECTION_H
#define INC_5042_CONNECTION_H
#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <iostream>

class connection {
	
public:
    /**
     * This default constructor is not used.
     *
     * INPUT: N/A
     * OUTPUT: N/A
     */
    connection();

	
    /**
     * This constructor creates a connection which stores all relevant info for the connection
     *
     * INPUT: int socket
     * INPUT: int reference to server_fd 
     * INPUT: sockaddr_in struct reference for address info
     * OUTPUT: connection object set up with connection information
     */
    connection(int mySocket, int &server_fd, struct sockaddr_in &address);

	
    /**
     * This sends a given string to the computer that the connection info points to
     * optionally it will then listen for a response.
     *
     * INPUT: string RPC to send
     * INPUT: bool should I wait for a response?
     * OUTPUT: response from recipient of message 
     *         (only if bRead is true, else it will just dump 
     *          the input buffer or return null)
     */
    std::string sendRPC(std::string RPC, bool bRead);

	
    /**
     * This function instructs the connection to wait and listen until it gets
     * a message from the connected computer.
     *
     * OUTPUT: string response message from connected computer
     */
    std::string listen();

	
    /**
     * This function instructs the connection to send a disconnect message
     * then delete itself, effectively closing the connection.
     */
    void end();

	
    /**
     * This function instructs the connection to close it's socket
     * It may be safely reassigned to a different socket at this point
     */
    void closeSocket();

	
    // Setter for socket number
    void setSocket(int newSocket);


    // Getter for socket number
    int getSocket();


    // Setter for thread number
    void setThreadNum(int num);

	
    // Getter for thread number
    int getThreadNum();

	
private:
    ///private variables

    //thread number the connection is operating on
    int threadNum;

    //socket the connection is operating on
    int mySocket;

    //server info for the connection
    int server_fd;

    //struct for holding the addressing info for the connection
    struct sockaddr_in address;
};

#endif //INC_5042_CONNECTION_H
