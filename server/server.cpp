#include "server.h"
#include "connection.h"
#include "game.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;


static pthread_t threads[MAXCONNECTION];
volatile int i = 0;
volatile int HIGHSCORE = 0;
pthread_mutex_t mutex;

std::random_device server::rd;
std::vector<std::string> server::words;
#define WORDS_FILE "words.txt"


server::server() {
    if(words.size()==0) {
        ifstream file(WORDS_FILE);
        if (file.is_open()) {
            string line;
            while (std::getline(file, line)) {
                if (line.length() == 0) {
                    continue;
                }
                words.push_back(line);
            }
            file.close();
        } else {
            cout << "Error on open words file " << WORDS_FILE << endl;
            exit(EXIT_FAILURE);
        }
    }
}


server::server(int port) {
    this->PORT = port;
    if(words.size()==0) {
        ifstream file(WORDS_FILE);
        if (file.is_open()) {
            string line;
            while (std::getline(file, line)) {
                if (line.length() == 0) {
                    continue;
                }
                words.push_back(line);
            }
            file.close();
        } else {
            cout << "Error on open words file " << WORDS_FILE << endl;
            exit(EXIT_FAILURE);
        }
    }
}


std::string server::getRandomWord() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, words.size() - 1);
    auto s = words[dis(gen)];
    return s;
}


struct tokens : std::ctype<char> {
    tokens() : std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table() {
        typedef std::ctype<char> cctype;
        static const cctype::mask *const_rc = cctype::classic_table();

        static cctype::mask rc[cctype::table_size];
        std::memcpy(rc, const_rc, cctype::table_size * sizeof(cctype::mask));

        //splits on = and ;, follow same format for additional delimiters.
        rc[static_cast<int>('=')] = std::ctype_base::space;
        rc[static_cast<int>(';')] = std::ctype_base::space;

        return &rc[0];
    }
};


map<string, std::string> server::mappify(string const& s) {
    std::map<std::string, std::string> m;

    std::string key, val;
    std::istringstream iss(s);

    while(std::getline(std::getline(iss, key, ':') >> std::ws, val)) {
        m[key] = val;
    }

    return m;
}


bool server::authenticate(std::string userName, std::string password) {
    const char *fileName="../userPasswordHut.txt";
    ifstream paramFile;
    paramFile.open(fileName);
    string line;
    string key;
    map <string, string> valueMap;

    if(paramFile.is_open()) {
        while(getline(paramFile, line)) {
            map <string, string> tempMap = mappify(line);
            for(auto const& s: tempMap) {
                valueMap[s.first] = s.second;
            }
        }
    }
    paramFile.close();

    // Check throughout the map
    for(auto const& s : valueMap) {
        if(s.first == userName) {
            if(s.second == password) {
                return true;
            }
        }
    }
    return false;
}


void server::filePutContents(const string& name, const string& content, bool append = false) {
    ofstream outfile;
    if (append) {
        outfile.open(name, ios_base::app);
    } else {
        outfile.open(name);
    }
     outfile << content<<endl;
}


void server::registration(string username, string password) {
    string line = username + ": " + password;

    filePutContents("../userPasswordHut.txt", line, true);
    cout << SERVER << "Registration Successful!" << endl;
}


bool server::login(int socket) {
    bool bLogin = false;
    char buffer[1024] = { 0 };
    ssize_t status;

    // Read messages from users
    status = read(socket, buffer, 1024);
    if(status == -1 || status == 0) {
        std::cout << SERVER << "User Disconnected" << std::endl;
        close(socket);
        return false;
    }
    printf("%sReply Recieved\n", SERVER);

    // Split the messages from the users
    std::vector<std::string> clientCommand = splitBuffer(buffer);

    // Interpret users' messages
    while (!bLogin) {
        if (clientCommand[0] == "rpc") {
            // Connect function
            if (clientCommand[1] == "connect") {
                // Registration function
                if (clientCommand[3].substr(0, 1) == "!") {
                    clientCommand[3] = clientCommand[3].substr(1, clientCommand[3].length());
                    registration(clientCommand[3], clientCommand[5]);
                    printf("%sAuto login successful\n", SERVER);
                    std::string reply = "reply=success;error=null";
                    status = send(socket, reply.c_str(), reply.size() + 1, 0);
                    if (status == -1 || status == 0) {
                        std::cout << SERVER << "User Disconnected" << std::endl;
                        close(socket);
                        return false;
                    }
                    bLogin = true;
                // Login function
                } else {
                    if (authenticate(clientCommand[3], clientCommand[5])) {
                        printf("%sLogin Credentials verified\n", SERVER);

                        std::string reply = "reply=success;error=null";
                        status = send(socket, reply.c_str(), reply.size() + 1, 0);
                        if (status == -1 || status == 0) {
                            std::cout << SERVER << "User Disconnected" << std::endl;
                            close(socket);
                            return false;
                        }
                        bLogin = true;
                    } else {
                        printf("%sUserID or Password is wrong\n", SERVER);
                        std::string reply = "reply=fail;error=UserID or Password is wrong";
                        status = send(socket, reply.c_str(), reply.size() + 1, 0);
                        if (status == -1 || status == 0) {
                            std::cout << SERVER << "User Disconnected" << std::endl;
                            close(socket);
                            return false;
                        }
                    }
                }
            // Disconnect function
            } else if(clientCommand[1] == "disconnect") {
                std::cout << SERVER << "User Disconnected" <<std::endl;
                return false;
            } else {
                printf("%sUnauthorized command submitted (unrecognized RPC)\n", SERVER);
                std::string reply = "reply=unauthorized;error=Unauthorized command, please log-in and retry";
                send(socket, reply.c_str(), reply.size() + 1, 0);
            }
        } else {
            printf("%sUnauthorized command submitted (not an RPC)\n", SERVER);
            std::string reply = "reply=unauthorized;error=unauthorized command, please log-in and retry";
            send(socket, reply.c_str(), reply.size() + 1, 0);
        }

        // Server ready to get new attempt of username/password
        if (!bLogin) {
            status  = read(socket, buffer, 1024);
            if(status == -1 || status == 0) {
                std::cout << SERVER << "User Disconnected" << std::endl;
                close(socket);
                return false;
            }
            printf("%sReply Recieved\n", SERVER);
            clientCommand = splitBuffer(buffer);
        }
    }
    return bLogin;
}


void server::serverSetup(struct sockaddr_in &address, int &server_fd){
    int opt = 1;

    if (this->bFirstRun == true) {
        // Do stuff for initial server set-up like read password file into a map, etc
        this->bFirstRun = false;
    }

    // Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("ERROR: socket failed");
        exit(EXIT_FAILURE);
    }
    printf("%sGot Socket\n", SERVER);

    // Forcefully attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("ERROR: setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set-up address struct
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attach socket to the port
    printf("%sAbout to bind\n", SERVER);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("ERROR: bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    printf("%sReady, listening for connections\n", SERVER);
    if (listen(server_fd, 3) < 0) {
        perror("ERROR: listen");
        return;
    }
}


void *server::createMultithreadInLogion(void *arg) {
    connection *myConnection = (connection *) arg;

    if(login(myConnection->getSocket())) {
        myConnection->setThreadNum(i);
        startGame(myConnection);
        printf("%sNew connection created\n", SERVER);
    } else {
        pthread_join(threads[myConnection->getThreadNum()], nullptr);
        myConnection->closeSocket();
        myConnection->end();
    }

    return NULL;
}


void server::createConnection(struct sockaddr_in &address, int &server_fd) {
    int newSocket; // might need to be dynamically allocated if for multiple users
    int addrlen = sizeof(address);

    connection *newConnection = new connection(0, server_fd, address);

    printf("\n%sWaiting\n", SERVER);
    if ((newSocket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *)&addrlen)) < 0) {
        perror("ERROR: accept");
        exit(EXIT_FAILURE);
    }

    // Accept a new client and generate a new socket for the client
    printf("%sAccepted Connection\n", SERVER);
    newConnection->setSocket(newSocket);
    pthread_mutex_lock(&mutex);
    newConnection->setThreadNum(i);
    pthread_mutex_unlock(&mutex);
    pthread_create(&threads[i++], nullptr, createMultithreadInLogion, (void *) newConnection);
}


std::vector<std::string> server::splitBuffer(std::string buffer) {
    std::stringstream ss(buffer);
    ss.imbue(std::locale(std::locale(), new tokens()));
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vec(begin, end);
    return vec;
}


void server::startGame(connection *myConnection) {
    std::cout << SERVER << "Running Game on Thread #"
              << std::to_string(myConnection->getThreadNum()) << std::endl;
    sleep(1);
    ssize_t status;  // To check the status of send and read functions
    game g(getRandomWord());
    std::string reply = "type=gameload;mask=" + g.getMask() + ";score="
                        + std::to_string(g.getScore()) + ";";

    // Send messages to clients
    status = send(myConnection->getSocket(), reply.c_str(), reply.size() + 1, 0);
    if(status == -1 || status == 0) {
        std::cout << SERVER << "User on Thread #"
                  << std::to_string(myConnection->getThreadNum())
                  << " Disconnected" << std::endl;
        myConnection->closeSocket();
        pthread_exit(NULL);
    }

    char buffer[1024] = { 0 };

    // Start the game
    while(true) {
        reply = "Error";
        status = read(myConnection->getSocket(), buffer, 1024);
        if(status == -1 || status == 0) {
            std::cout << SERVER << "User on Thread #"
                      << std::to_string(myConnection->getThreadNum())
                      << " Disconnected" << std::endl;
            myConnection->closeSocket();
            pthread_exit(NULL);
        } else {
            auto cmd=splitBuffer(buffer);

            // Deal with guessing a letter or a word
            if(cmd[0] == "type" && cmd[1] == "gaming") {
                if(cmd[3].length() == 1){
                    reply = g.step(cmd[3][0]);
                } else if(cmd[2] == "word") {
                    reply = g.checkGuess(cmd[3]);
                }
            }

            // Deal with ending a game
            if(cmd[0] == "type" && cmd[1] == "gameend") {
                if(cmd[3] == "y" || cmd[3] == "Y") {
                    g.reload(getRandomWord());
                    reply = "type=gameload;mask=" + g.getMask() + ";score="
                            + std::to_string(g.getScore()) + ";";
                } else {
                    reply = "goodbye;";
		    pthread_mutex_lock(&mutex);

		    if (g.getTotalScore() > HIGHSCORE) {
		 	HIGHSCORE = g.getTotalScore();
			std::cout << SERVER << "NEW HIGH SCORE!\nSERVER>> Server High Score is now: "
			  	  << std::to_string(HIGHSCORE) << " points!\n";
		 	reply += std::to_string(HIGHSCORE);
		    } else {
		        reply += "0";
		    }

		    /// Global variable
		    pthread_mutex_unlock(&mutex);
                    status = send(myConnection->getSocket(), reply.c_str(), reply.size() + 1, 0);
                    myConnection->closeSocket();
		    std::cout << SERVER << "Closing Game Thread #"
			      << std::to_string(myConnection->getThreadNum()) << std::endl;
                    pthread_exit(NULL);
                }
            }

            std::cout << SERVER << "Game on going " << std::endl;
            status = send(myConnection->getSocket(), reply.c_str(), reply.size() + 1, 0);

            if(status == -1 || status == 0) {
                std::cout << SERVER << "User disconnected" << std::endl;
                myConnection->closeSocket();
                pthread_exit(NULL);
            }
        }
    }
}
