#ifndef INC_5042_SERVER_H
#define INC_5042_SERVER_H
#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <iterator>
#include <cstring>
#include <sstream>
#include <signal.h>
#include <random>
#define MAXCONNECTION 9999999
#define SERVER "SERVER>> "

class connection;
class game;

class server {
	
public:
    /**
     * This constructor creates a server on the hardcoded defalt port
     *
     * OUTPUT: server Object listening on default port
     */
    server();
	

    /**
     * This constructor creates a server on the provided port
     *
     * INPUT: int port #
     * OUTPUT: server Object listening on given port
     */
    server(int port);
	

    /**
     * This is a function to start-up the server instance
     *
     * Sets up the server and socket options. Also has logic
     * for running first time set-up options the first time the function
     * is called on an instance.
     *
     * INPUT: Reference to a sockaddr_in struct
     * INPUT: Reference to an int server_fd
     */
    void serverSetup(struct sockaddr_in &address, int &server_fd);
	

    /**
     * This is a function to assign sockets to clients
     *
     * sets up a new socket, verifies user by calling the login() function
     * then creates a connection object pointer for that user which contains
     * all the connection info.
     *
     * INPUT: Reference to a sockaddr_in struct
     * INPUT: Reference to an int server_fd
     */
    void createConnection(struct sockaddr_in &address, int &server_fd);
	

private:
    ///Private Variables:
	
    // Default port if server is instantiated using default constructor
    int PORT = 12105;

    // Needed only if activity outside the constructor is desired. Not used in this implementation
    bool bFirstRun = true; 

    // Map to use for authenticating users
    static std::map<std::string, std::string> mappify(std::string const& s);

    static std::random_device rd;

    // Holds the list of words from words.txt
    static std::vector<std::string> words;


    ///Private Functions:
	
    /**
     * Gets a random word from the words vector
     *
     * OUTPUT: string word
     */
    static std::string getRandomWord();

	
    /**
     * This is a function to split up replies from the server
     *
     * Uses the "tokens" struct, and <istream> + <iterator> libs.
     * splits on chars defined in the "tokens" struct into a
     * vector of strings
     *
     * Input: String with delimiters
     * Output: Vector of strings, broken on the set delimiters
     */
    static std::vector<std::string> splitBuffer(std::string buffer);

	
    /**
     * This is a function to handle logging into the server
     *
     * Reads RPC from the client, and performs actions based on it:
     * connect;<userName>;<password> calls authenticate() to authorize login
     * all others: report errors back to client
     *
     * INPUT: int socket which is set-up for client communication
     * OUTPUT: bool true if login is successful; else false
     */
    static bool login(int socket);

	
    /**
     * This is a function to check username and password
     *
     * Search given userName from userPasswordHut file and check the password accordingly
     *
     * INPUT: String userName
     * INPUT: String password
     * OUTPUT: bool true if authorized, else false
     */
    static bool authenticate(std::string userName, std::string password);

	
   /**
    * This is a function that formats the username and password to be added to the registration db
    *
    * INPUT: String userName
    * INPUT: String password
    * OUTPUT: void
    */
    static void registration(std::string username, std::string password);

	
    /**
     * This is a function to add new users to userpasswordHut file
     *
     * INPUT: String userName
     * INPUT: String password
     * OUTPUT: void
     */
    static void filePutContents(const std::string& name, const std::string& content, bool append);

	
    /**
     * This is a function to start the game
     *
     * pthreads won't accept member functions, so we force it to be a static "non-member" function.
     * Making this static is apparently bad practice, but we are hacking Object Oriented C++ into 
     * old school C threads, so it must be done.
     *
     * INPUT: connection pointer
     */
    static void startGame(connection *arg);

	
    /**
     * This is a function that handles multiple simultaneous connections
     *
     * This function modifies the serverside array of all existing connections
     * and assigns them unique threads to operate on.
     *
     * INPUT: connection object that has been cast to a void pointer
     */
    static void *createMultithreadInLogion(void *arg);
	
};

#endif //INC_5042_SERVER_H
