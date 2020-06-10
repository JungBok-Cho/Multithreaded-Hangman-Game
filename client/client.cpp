#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <istream>
#include <vector>
#include <iterator> //for spliting
#include <cstring> //for spliting
#include <sstream> //for spliting
#define DEFAULTHOSTNAME "127.0.0.1"
#define DEFAULTPORT "12105"
#define USER "USER>> "


//Function Declarations
int connectToServer(const char *,const char *, int &);
bool startClient(int);
std::string loginRPC();
std::string registrationRPC();
void startGame(int);
std::string onGoingGame(std::vector<std::string>);
std::string endGame(std::vector<std::string>);
bool checkUsedChar(std::string, char);
bool checkPwdFormat(std::string);
std::string sendRPC(int, std::string);
std::string disconnectRPC(int &);
std::vector<std::string> splitBuffer(std::string);


/**
 * Main method to start this client program
 *
 * @param argc  IP Address
 * @param argv  Port Number
 * @return Return 0 when it ends program
 */
int main(int argc, char **argv) {
    int mySocket = 0;

    // Default IP Address and Port Number
    std::string host=DEFAULTHOSTNAME;
    std::string port=DEFAULTPORT;

    // If User wants different ones, assign those
    if(argc>=3){
        host=argv[1];
        port=argv[2];
    }

    printf("%sConnecting to IP: %s | Port: %s\n", USER,host.c_str(), port.c_str());

    // Try to connect to the client
    if (connectToServer(host.c_str(), port.c_str(), mySocket) != -1) {
        // Start the client
        if(startClient(mySocket)) {
            std::cout << USER << "Login accepted!\n";
            startGame(mySocket);
        }
    }

    return 0;
}


/**
 * Struct to allow tokenized STL splitting of strings by multiple delimiters
 */
struct tokens : std::ctype<char> {
    tokens() : std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table() {
        typedef std::ctype<char> cctype;
        static const cctype::mask *const_rc = cctype::classic_table();

        static cctype::mask rc[cctype::table_size];
        std::memcpy(rc, const_rc, cctype::table_size * sizeof(cctype::mask));

        // Splits on = and ;, follow same format for additional delimiters.
        rc[static_cast<int>('=')] = std::ctype_base::space;
        rc[static_cast<int>(';')] = std::ctype_base::space;

        return &rc[0];
    }
};


/**
 * This is a function to start a connection with the Server.
 *
 * @param hostName  IP Address to connect
 * @param port      Port number to connect
 * @param mySocket  Socket number
 * @return Return socket number if connected, otherwise return -1
 */
int connectToServer(const char *hostName, const char *port, int &mySocket) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t)atoi(port));
    serv_addr.sin_family = AF_INET;

    // Create a socket
    if ((mySocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\nERROR: Socket creation \n";
        return -1;
    }

    // Converts the character string src into a network address structure,
    // then copies the network address structure to dst
    if (inet_pton(AF_INET, hostName, &serv_addr.sin_addr) <= 0) {
        std::cout << "\nERROR: Invalid address/Address not supported \n";
        return -1;
    }

    // Connects the socket to the address specified by addr
    if (connect(mySocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nERROR: Connection Failed \n");
        return -1;
    }

    return mySocket;
}


/**
 * Check if user wants to login or create a new account
 *
 * @param mySocket Socket number to use
 * @return  Return true if the user successfully login or created a new account.
 *          Otherwise return false
 */
bool startClient(int mySocket) {
    bool choice = true;
    int loginLimit = 3;
    std::string serverReply;
    std::string userInput;

    do {
        std::cout << USER << "Enter 1 if you want to add a new account" << std::endl;
        std::cout << USER << "Enter 2 if you already have an account" << std::endl;
        std::cout << USER;
        getline(std::cin, userInput);

        // Create a new account
        if(userInput == "1") {
            serverReply = sendRPC(mySocket, registrationRPC());
            choice = false;

        // Try to login
        } else if(userInput == "2") {
            do {
                serverReply = sendRPC(mySocket, loginRPC());
                std::vector<std::string> reply = splitBuffer(serverReply);
                if(reply[0] == "reply" && reply[1] == "success") {
                    return true;
                } else {
                    choice = reply[0] != "reply" || reply[1] != "success";
                }

                if(reply[1] != "success" && (loginLimit <= 3 && loginLimit > 1)) {
                    std::cout << USER << "Login Failed. Try again." << std::endl;
                    std::cout << std::endl;
                }

                loginLimit--;

                // If user keeps fail to login, Ask the user if the person wants to create a new account
                if(loginLimit <= 0) {
                    std::cout << USER << "Do you wish to Register as an new user? (y/n)" << std::endl;
                    std::string answer;
                    std::cout << USER;
                    std::cin >> answer;

                    // Create a new account if user wants
                    if (answer == "y") {
                        serverReply = sendRPC(mySocket, registrationRPC());
                        choice = false;

                    // Disconnect if user does not want
                    } else {
                        disconnectRPC(mySocket);
                        return false;
                    }
                }
            } while(choice);
        }
    } while(choice);

    return true;
}


/**
 * This is a function to generate the Registration RPC.
 * Generated RPC format="rpc=connect;username=!<Your user>;password=<Your password>;"
 *
 * @return Return the generated RPC
 */
std::string registrationRPC() {
    std::string RPC = "rpc=connect;";
    std::string username, password;

    // Ask user to enter a new username
    std::cout << "\n" << USER << "Enter username with minimum length of 6: ";
    std::cin >> username;

    // Check if the username is valid
    while(username.size() < 6) {
        std::cout << USER << "Enter valid username: ";
        std::cin >> username;
    }

    // Ask user to enter a new password
    std::cout << USER << "Enter password with minimum length of 8 \n      "
                         " including an upper case letter, \n      "
                         " a lower case letter, and a number.\n";
    getline(std::cin, password);

    // Check if the password is valid
    while(!checkPwdFormat(password)) {
        std::cout << USER << "Enter valid password: ";
        getline(std::cin, password);
    }

    // Generate the Registration RPC
    RPC += "username=!" + username + ";password=" + password + ";";

    return RPC;
}


/**
 * This is a function to generate the login RPC.
 * Generated RPC format="rpc=connect;username=<Your user>;password=<Your password>;"
 *
 * @return  Return the generated RPC
 */
std::string loginRPC() {
    std::string RPC = "rpc=connect;";
    std::string username, password;

    // Ask user to input the username and password
    std::cout << USER << "Enter username: ";
    getline(std::cin, username);
    std::cout << USER << "Enter password: ";
    getline(std::cin, password);

    // Generate the login RPC
    RPC += "username=" + username + ";password=" + password + ";";

    return RPC;
}


/**
 * This is a function to send RPCs to the server and read its reply
 * Output format = "reply=<fail or success>;error=<error or null>
 *
 * @param socket  The Socket to send and receive messages
 * @param RPC     The RPC message to send
 * @return        Return the message received from the server
 */
std::string sendRPC (int socket, std::string RPC) {
    char reply[1024] = { 0 };
    ssize_t status; // To check the status of the send and read functions

    // Send a message to the server
    status = send(socket, RPC.c_str(), RPC.size() + 1, 0);
    if(status == -1) {
        std::cout << "ERROR: Send Failed" << std::endl;
    }

    // Read a message from the server
    status = read(socket, reply, 1024);
    if(status == -1){
        std::cout << "ERROR: Read Failed" << std::endl;
    }

    return std::string(reply);
}


/**
 * This is a function that the user can play a game.
 *
 * @param mySocket
 */
void startGame(int mySocket) {
    // Welcome message
    std::cout << "\n" << USER << "Welcome to Hangman! "
                                 "\n       Guess the letters of hidden words, "
                                 "\n       or guess the whole word! "
                                 "\n       Get the word right and you get points! "
                                 "\n       Run out of points and it's game over.\n\n";

    char reply[1024] = { 0 };   // To check the reply from the server
    ssize_t status;             // To check the status of sending and receiving functions
    std::string message;        // To send message to the server

    while(true) {
        message = "None"; // Default message

        // Read the message from the server
        status = read(mySocket, reply, 1024);
        if(status == -1){
            std::cout << "ERROR: Read Failed" << std::endl;
            break;
        }

        // Disassemble the message
        auto rsp = splitBuffer(reply);

        // Interpret the message from the server
        if(rsp[0] == "type") {

            // Ongoing Game
            if(rsp[1] == "gameload" || rsp[1] == "gaming") {
                message = onGoingGame(rsp);
            // Game End
            } else if(rsp[1] == "gameend") {
                message = endGame(rsp);
            }

        // End Game Message
        } else {
            if(rsp[1] == "0") {
                std::cout << USER << "Next time, try to make the highest score!" << std::endl;
            } else {
                std::cout << USER << "You made New Highest Score, " << rsp[1] << "!" << std::endl;
            }
            std::cout << USER << "Thank you for playing this game! Goodbye :)\n" << std::endl;
            break;
        }

        // Send the Game RPC to the Server
        status = send(mySocket, message.c_str(), message.size() + 1, 0);
        if(status == -1) {
            std::cout << "ERROR: Send Failed" << std::endl;
            break;
        }
    }
}

/**
 * This function deals with the ongoing game.
 *
 * @param rsp Splitted message from the server
 * @return  Return the Ongoing Game RPC
 */
std::string onGoingGame(std::vector<std::string> rsp) {
    if (rsp[3][0] == '!') {
        std::cout << USER << "That's wrong guess. Try again!" << std::endl;
        std::cout << USER << "Mask word: " << rsp[3].substr(1, rsp[3].size() - 1)
                  << ", you have " << rsp[5] << " points left!" << std::endl;
    } else {
        std::cout << USER << "Mask word: " << rsp[3] << ", you have "
                  << rsp[5] << " points left!" << std::endl;
    }

    // Ask user to input a character
    std::string c;
    std::cout << USER;
    std::getline(std::cin, c);

    // Check the character
    if (c.length() == 1) {
        do {
            if (checkUsedChar(rsp[3], c[0])) {
                std::cout << USER << "You already found the character.\n";
                std::cout << "       Guess a different character: ";
                std::getline(std::cin, c);
            } else {
                break;
            }
        } while (checkUsedChar(rsp[3], c[0]));
    }

    // Create the Game RPC to send to the server
    if (c.length() == 1) {
        return "type=gaming;char=" + c;
    } else {
        return "type=gaming;word=" + c;
    }
}


/**
 * This function deals with the ended game.
 *
 * @param rsp  Splitted message from the server
 * @return  Return the End Game RPC
 */
std::string endGame(std::vector<std::string> rsp) {
    // When the user wins the game
    if(rsp[7] == "win") {
        std::cout << USER << "Game over! \n"
                  << USER <<  "You win, score: " << rsp[3] << ", the word was ["
                  << rsp[5] << "].\n" << USER << "Your total score for this session is "
                  << rsp[9] << " points.\n" << USER << "Would you like to play again? (y/n): " << std::endl;

        // When the user loses the game
    } else {
        std::cout << USER << "Game over! \n"
                  << USER << "You fail, the word was ["
                  << rsp[5] << "].\n" << USER << "Your total score for this session is "
                  << rsp[9] << " points.\n" << USER << "Would you like to play again? (y/n): " << std::endl;
    }

    // Ask if user wants to play again
    std::string c;
    while(true) {
        std::cout << USER;
        std::getline(std::cin,c);
        if(c.length() == 1) {
            break;
        }
        std::cout << USER << "Would you like to play again? (y/n): ";
    }
    return "type=gameend;char=" + c;
}


/**
 * Check if the word has the character
 *
 * @param word  The word to check
 * @param c     The character to check
 * @return  Return true if the word has the character. Otherwise return false
 */
bool checkUsedChar(std::string word, char c) {
    for(char i :word) {
        if(i == c) {
            return true;
        }
    }
    return false;
}


/**
 * Check the password format
 *
 * @param input Password to check
 * @return Return true if the password is in a valid format, otherwise return false
 */
bool checkPwdFormat(std::string input) {
    bool lowerLetter = false;
    bool upperLetter = false;
    bool digit = false;

    if (input.size() < 8) {
        return false;
    }

    for (char c : input) {
        if (c >= 97 && c <= 122) {
            lowerLetter = true;
        } else if (c >= 65 && c <= 90) {
            upperLetter = true;
        } else if (c >= 48 && c <= 57) {
            digit = true;
        } else{
            return false;
        }
    }

    return lowerLetter && upperLetter && digit;
}


/**
 * This is a function to disconnect from the Server and close the socket.
 *
 * @param sock  The Socket to cose
 * @return  Return the message from the server
 */
std::string disconnectRPC(int & sock) {
    char reply[1024] = { 0 };

    // Disconnect RPC
    std::string disconnectMessage = "rpc=disconnect;";

    //Send the disconnect rpc and receive a message from the Server
    ssize_t status;
    status = send(sock, disconnectMessage.c_str(), disconnectMessage.size() + 1, 0);
    if(status == -1) {
        std::cout << "ERROR: Send Failed" << std::endl;
    }

    // Close the client socket
    if (true) { //optional logic for reading server reply and performing additional actions before closing socket
        std::cout << USER << "Disconnected, Socket closed.\n\n";
        close(sock);
    }

    return std::string(reply);
}


/**
 * This is a function to split up replies from the server.
 * It uses the "tokens" struct, and <istream> + <iterator> libs
 * splits on chars defined in the "tokens" struct into a vector of strings.
 *
 * @param buffer The buffer to split up
 * @return  Return a vector that contains the splitted buffer
 */
std::vector<std::string> splitBuffer(std::string buffer) {
    std::stringstream ss(buffer);
    ss.imbue(std::locale(std::locale(), new tokens()));
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vec(begin, end);
    return vec;
}
