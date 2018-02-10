#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <termios.h>
#include <unistd.h>

using namespace std;

#define STDIN 0 // stdin

int getch()
{
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
} // getch() and getpass() functions are taken from http://www.cplusplus.com/articles/E6vU7k9E/ for hiding password

string getpass(const char* prompt, bool show_asterisk = true)
{
    const char BACKSPACE = 127;
    const char RETURN = 10;

    string password;
    unsigned char ch = 0;

    cout << prompt;

    while ((ch = getch()) != RETURN) {
        if (ch == BACKSPACE) {
            if (password.length() != 0) {
                if (show_asterisk)
                    cout << "\b \b";
                password.resize(password.length() - 1);
            }
        }
        else {
            password += ch;
            if (show_asterisk)
                cout << '*';
        }
    }
    cout << endl;
    return password;
}

int main(int argc, char* argv[])
{

    if (argc < 3) {
        cout << "Warning : executefile<space>server<space>port number\n";
        return 0;
    }

    int port = atoi(argv[2]);
    // end of taking command line argument

    int sock, activity; // main socket and activity
    struct sockaddr_in server;
    //struct hostent *servername;
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    //    timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
    // timeout.tv_usec = 0;

    string temp1;
    char server_reply[1024];
    int x;

    //cout << "Socket created\n";

    bzero((char*)&server, sizeof(server));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    char inp;
    bool val_check = 1;
    bool isAuthenticated = false;
    bool logged_out = true;
    //bool

    string username, name, password, confpassword;

    while (true) {
        if (!isAuthenticated) // if isAuthenticated is true login begins
        {
            if (logged_out) {
                //Create socket for first time login
                sock = socket(AF_INET, SOCK_STREAM, 0);
                //cout << sock << endl;
                if (sock == -1) {
                    cout << "Could not create socket\n";
                }
                if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
                    cout << "Error in Connecting\n";
                    return 0;
                }
                logged_out = false;
            }
            cout << "\nPress \"l\" for login or \"r\" for registration or \"e\" to exit program\n";
            cin >> inp;
            if (inp == 'l' || inp == 'L') // code for login
            {
                cout << "Enter username : ";
                cin.ignore();
                getline(cin, username);
                password = getpass("\nPlease enter the password: ", true);
                temp1 = string("l;") + username + string(";") + password;

                if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                    cout << "\nlogin failed...Try Again !!!!";
                }

                memset(server_reply, 0, 1024);
                //Receive a reply from the server

                if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                    cout << "login failed\n";
                }
                else {
                    server_reply[x] = '\0';

                    //cout << server_reply << endl;
                    //cout << strlen(server_reply);
                    if (server_reply[0] == 'A') //// acknowledging from server for login
                    {
                        isAuthenticated = true;
                        cout << "\nYou have successfully logged in---\n";

                        cout << "\nPress w for help any time---\n";
                    }
                    else
                        cout << "\n Invalid credentials !!!";
                }
                // if (server_reply == "s")
            }
            else if (inp == 'r' || inp == 'R') // code for registration
            {
                cout << "Enter your name : ";
                cin.ignore();
                getline(cin, name);
                cout << "\nEnter your username : ";
                getline(cin, username);
                password = "a";
                confpassword = "b";
                while (true) {
                    password = getpass("\nPlease enter the password: ", true);
                    confpassword = getpass("\nPlease re-enter the password: ", true);
                    if (password == confpassword)
                        break;
                    else
                        cout << "\nPasswords did not match . Please re-enter password\n";
                }

                temp1 = string("r;") + name + string(";") + username + string(";") + password;

                if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                    cout << "\nRegistration failed...Try Again !!!!";
                }

                if ((x = recv(sock, server_reply, 1024, 0)) < 0) // acknowledging from server for registration
                {
                    cout << "registration failed\n";
                }
                else {
                    server_reply[x] = '\0';
                    cout << server_reply << endl;
                }
            }
            else if (inp == 'e' || inp == 'E')
                return 0;
            else
                cout << "\033[1;31m\nInvalid option\033[0m\n";
        }

        else {

            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            FD_SET(STDIN, &readfds);
            //max_sd = sock;

            activity = select(sock + 1, &readfds, NULL, NULL, NULL);

            if ((activity < 0) && (errno != EINTR)) {
                cout << "\033[1;31mSelect error\033[0m\n";
            }

            if (FD_ISSET(STDIN, &readfds)) // code stdin socket
            {

                inp = '-';
                //cout << "input : ";
                cin >> inp;

                if (inp == 'u' || inp == 'U') {
                    // code for unseen messages

                    temp1 = string("u");

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\033[1;31mnConnection failed...Try Again !!!!\033[0m\n";
                    }

                    if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                        cout << "connection failed\n";
                    }
                    else {
                        server_reply[x] = '\0';
                        cout << server_reply << endl;
                    }
                }
                else if (inp == 'h' || inp == 'H') {
                    // code for conversation history with friend

                    string temp_name;
                    cout << "\n Enter friend's name to view your conversation with him : \n\t";
                    cin >> temp_name;

                    temp1 = string("h;") + temp_name;

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\nConnection failed...Try Again !!!!";
                    }

                    if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                        cout << "registration failed\n";
                    }
                    else {
                        server_reply[x] = '\0';
                        cout << server_reply << endl;
                    }
                }
                else if (inp == 'w' || inp == 'W') {
                    cout << "\n Press w to send messages to multiple friends ";
                    cout << "\n Press m to send messages to multiple friends ";
                    cout << "\n Press p to send messages to a particular friend";
                    cout << "\n Press u for unseen messages";
                    cout << "\n Press h for chat history";
                    cout << "\n Press o to list Active friends";
                    cout << "\n Press f to list all friends";
                    cout << "\n Press l to Logout";
                    cout << "\n Press e to exit program \n";
                }
                else if (inp == 'o' || inp == 'O') {
                    // code for listing online friends
                    temp1 = string("o");

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\nConnection failed...Try Again !!!!";
                    }

                    if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                        cout << "registration failed\n";
                    }
                    else {
                        server_reply[x] = '\0';
                        cout << server_reply << endl;
                    }
                }
                else if (inp == 'f' || inp == 'F') {
                    // code for listing all registered friends
                    temp1 = string("f");

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\nConnection failed...Try Again !!!!";
                    }

                    if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                        cout << "registration failed\n";
                    }
                    else {

                        server_reply[x] = '\0';
                        cout << server_reply << endl;
                    }
                }
                else if (inp == 'l' || inp == 'L') {
                    // code for logout
                    temp1 = string("e");

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\nConnection failed...Try Again !!!!";
                    }
                    else {
                        close(sock);
                        isAuthenticated = false;
                        logged_out = true;
                    }
                }
                else if (inp == 'e' || inp == 'E') // code for exit
                {
                    temp1 = string("e");

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\nConnection failed...Try Again !!!!";
                    }
                    else {
                        close(sock);
                        cout << "\n Program is closing......\n";
                        return 0;
                    }
                }
                else if (inp == 'm' || inp == 'M') {
                    // code for multiple chatting room
                    int z; //
                    cout << "\n Enter number of friends you want to message : \n\t";
                    cin >> z;
                    string temp_name[z], temp_message;
                    temp1 = string("m;") + to_string(z);
                    cout << "\n Enter names of friends you want to message : \n\t";
                    for (int i = 0; i < z; ++i) {
                        cin >> temp_name[i];
                        temp1 += (string(";") + temp_name[i]);
                    }

                    cout << "\n Enter message : \n\t";
                    cin >> temp_message;
                    temp1 = temp1 + string(";") + temp_message;

                    if (send(sock, temp1.c_str(), temp1.length(), 0) < 0) {
                        cout << "\nConnection failed...Try Again !!!!";
                    }
                    else {
                        cout << "\nYour message has been sent\n";
                    }
                }
                else if (inp == 'p' || inp == 'P') {
                    // code for two person chatting room
                    int z = 1;
                    cout << "\n press \"b\" to go back to previous menu any time during conversation ";
                    cout << "\n Enter name of friends want to chat with : \n\t";
                    string temp_name, temp_message;
                    temp1 = string("m;") + to_string(z);

                    cin >> temp_name;
                    temp1 += (string(";") + temp_name);

                    //cout << "\n Enter message : \n\t";
                    //cin >> temp_message;
                    temp1 = temp1 + string(";") + temp_message;

                    while (true) {
                        FD_ZERO(&readfds);
                        FD_SET(sock, &readfds);
                        FD_SET(STDIN, &readfds);
                        //max_sd = sock;

                        activity = select(sock + 1, &readfds, NULL, NULL, NULL);

                        if ((activity < 0) && (errno != EINTR)) {
                            cout << "\033[1;31mSelect error\033[0m\n";
                        }

                        if (FD_ISSET(STDIN, &readfds)) {

                            //printf("Enter message : ");
                            //scanf("%s" , message);
                            getline(cin, temp_message);
                            //cin >> temp_message;
                            if (temp_message == "b" || temp_message == "B")
                                break;
                            string temp2;
                            temp2 = temp1 + string(";") + temp_message;
                            //Send some data
                            if (send(sock, temp2.c_str(), temp2.length(), 0) < 0) {
                                cout << "\nConnection failed...Try Again !!!!";
                            }
                        }

                        if (FD_ISSET(sock, &readfds)) {
                            if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                                cout << "registration failed\n";
                            }
                            else {
                                server_reply[x] = '\0';
                                cout << server_reply << endl;
                            }
                        }
                    }
                }
                else
                    cout << "\033[1;31mInvalid option\033[0m\n";
            }

            if (FD_ISSET(sock, &readfds)) {
                if ((x = recv(sock, server_reply, 1024, 0)) < 0) {
                    cout << "registration failed\n";
                }
                else {
                    server_reply[x] = '\0';
                    cout << server_reply << endl;
                }
            }
        }
    } //end for infinite while loop

} //end of main function
