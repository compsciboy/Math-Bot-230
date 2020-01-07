#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdbool.h>

#define MAXLINE 8192 //Max text line length for buf array for storing messages recieved from server or to send to server

/*
*Creates a socket for the client and connects the socket to the server using the returned client file descriptor and the server's IP address and port number.
*Parameters:
    hostIP = char array storing the server IP address in the Internet standard dot notation
    port = integer storing the server process's port number 
*Returns:
    Returns the clients open file descriptor for the socket
*/
int open_clientfd(char *hostIP, int port){
    int clientfd; //Integer variable to store the client's socket file descriptor
    struct sockaddr_in serveraddr; //sockaddr_in structure used to record the server information (IP address and port number)- needed as argument for connect()
   
   //Creates a socket (endpoint for communication) and returns a file descriptor representing that socket or endpoint
   //Create the socket file descriptor with the protocol family AF_INET and socket type SOCK_STREAM
    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ //Prints message to stderr and exits if there is an error (returns -1)
        fprintf(stderr, "Error creating client socket\n"); 
        exit(1);
    }

    bzero((char *)&serveraddr, sizeof(serveraddr)); //Set the socket structure serveraddr with null values (zero out bytes) to remove any junk stored in struct
    serveraddr.sin_family = AF_INET; //Records the protocol family used to connect to server in the serveraddr struct (stored in the sin_family structure element)
    serveraddr.sin_port = htons(port); //htons() converts the port from host byte order to network byte order (big endian format) and stores this in the serveraddr structure (stored in the sin_port structure element)
    
    //inet_aton() function converts the string containing the Internet standard dot notation for the server's IP address held in hostIP to a network address and stores the address in the serveraddr struct (stored in the sin_addr structure element)
    if (inet_aton(hostIP, &serveraddr.sin_addr) == 0){ //Prints message to stderr and exits if there is an error (returns -1)
        fprintf(stderr, "Invalid address for inet_aton function\n");
        exit(1);
    }

    //Connects the client socket referred to by its file descriptor clientfd to the server specified by the IP address and port numbers stored in the serveraddr struct
    //serveraddr casted to a sockaddr pointer (we used sockaddr_in to access individual fields making up the address of a specific family, then casted it to generic socket structure needed to connect)
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){ //Prints message to stderr and exits if there is an error (returns -1)
        fprintf(stderr, "Error connecting socket to server\n");
        exit(1);
    }

    return clientfd; //Returns the client socket's file descriptor
}

/*
*Checks if you have captured the flag if server has sent a response with the flag
*Parameters:
    buf = Pointer to the character array buf storing message recieved from the server
*Returns:
    Returns false if the server has not sent the flag yet, true if it has and you have successfully captured the flag
*/
bool check_flag(char *buf){
    //For loop checks each character in the buf char array by incrementing "i" to check if there is any operators in the message sent by the server
    //If there is an operator this means that the server has sent another math problem and not a flag and therefore this returns false
    for (int i = 0; buf[i] != '\0'; i++){
        if (buf[i] == 'B' && buf[i+1] == 'Y' && buf[i+2] == 'E'){ //Checks if message BYE is at the end of buf which indicates that the flagwas captured
            return true; //Return true if BYE is at the end of buf meaning the server has sent the flag
        }
    }
    return false; //Return false if BYE is not in buf which means the server has not sent the flag
}

/*
*Checks which operator the math equation sent by the server uses and performs that operation.
*Parameters:
    operand1 = Integer storing the first operand of the math equation recieved from the server
    operand2 = Integer storing the second operand of the math equation recieved from the server (operand after the operator)
    operator =  Character storing the operator of the math equation recieved from the server 
*Returns:
    Returns an integer storing the answer to the math equation that was sent by the server
*/
int calculation(int operand1, int operand2, char operator){
    int answer; //Variable to store the answer of the math problem represented by performing operation indicated by "operator" with operand1 and operand2
    
    //Switch case checks which of the 4 operators is stored in operator from the math problem send by the server and performs that operation on operand1 and operand2- stores in answer
    switch(operator){
        //If operator is a plus sign add together operand1 and operand2 and store in answer and break out of switch case 
        case ('+'):
            answer = operand1 + operand2;
            break;

        //If operator is a minus sign subtract operand2 from operand1 and store in answer and break out of switch case
        case ('-'):
            answer = operand1 - operand2;
            break;
        
        //If operator is a division sign divide operand1 by operand2 and store in answer and break out of switch case
        //Since we are performing integer division, if the answer of this operation turns out to be a decimal it will be rounded down to the nearest whole number
        case ('/'):
            answer = operand1 / operand2;
            break;
        
        //If operator is a multiplication sign multiply operand1 by operand2 and store in answer and break out of switch case 
        case ('*'):
            answer = operand1 * operand2;
            break;
        
        //If the operator is none of these 4 operators there has been an error, prints error to stderr and exits program
        default:
            fprintf(stderr, "Error with extracting operator during math problem calculation.\n");
            exit(1);
    }

    return answer;
}

/*
*Extracts the math problem from the message sent from the server. Obtains the two operands and the operator stored in the message that are needed to solve the math equation.
*Parameters:
    buf = Pointer to the character array buf storing message recieved from the server
*Returns:
    Returns an integer storing the answer to the math equation that was sent by the server (answer evaluated and recieved from calculation() function)
*/
int mathproblem(char *buf){
    int operand1; //Integer variable to store the first operand contained in the math problem from the message sent from the server
    int operand2; //Integer variable to store the first operand contained in the math problem from the message sent from the server
    char operator; //Character variable to store the operator contained in the math problem from the message sent from the server 

    int i; //Integer used to increment through the buf char array character by character looking for the operator
    
    //For loop that increments through the buf array until the operator is found, then stores the number contained in the message before the operator in operand1 after converting it to an integer
    for (i = 0; buf[i] != '\0'; i++){
        //Checks if the character contained in buf at index i is an operator, if it is we convert the string containing the first operand to an integer and store in operand1
        //Breaks out of the for loop with index i representing the index of buf the operator is found at
        if (buf[i] == '/' || buf[i] == '*' || buf[i] == '-' || buf[i] == '+'){ 
            char operand[80]; //Char array to temporarily hold the string containing the first operand in
            strncpy(operand, buf+13, (i-1)-13); //Copy the portion of the message from index 13 (the index after the "cs230 STATUS " part of the message stored in buf where the first operand starts) and read the number of bytes equal to the length of the number (i-1)-13
            operand[(i-1)-13] = '\0'; //Place null character at the end of operand string
            operand1 = atoi(operand); //Converts the string operand stored in the char array operand to an integer and stores this in operand1
            break; //Breaks out of for loop
        }
    }

    operator = buf[i]; //Stores the character at buf[i] (which is the index of the operator- found during the for loop) in char operator
    operand2 = atoi(buf+(i+2)); //Converts everything left in buf after the operator plus the space after the operator into an integer and stores in operand2

    int answer = calculation(operand1, operand2, operator); //Calls the calculation function to check which operator is stored in operator and performs correct calculation on operand1 and operand2, storing the result in answer
    return answer;

}

/*
*Main client function that accepts the identification (UMass email address), server port number, and host IP address from the command line, connects to the server
on the specified port number, then sends the UMass ID to the server. The client then recieves a message from the server that contains a math problem. The client solves
this math problem and then sends the correct answer to the server. The server continues to send more math problems and the client continues to send the answers back unti;
the server sends a unique 64-byte string (flag) unique to the UMass NetID meaning the client has successfully captured the flag.
*Parameters:
    argc = the total number of arguments passed into main with one being the name of the program
    *argv[] = array containing all the arguments written in the command line- argv[0] always stores the name of the program
*Returns:
    Returns 0 when the client has successfully captured the flag
*/
int main(int argc, char *argv[]){
    char *identification; //Char pointer used to point to the string containing the UMass email address written in as the first argument to the command line
    int port; //Integer used to store the port number of the server written in as the second argument to the command line
    char *hostIP; //Char pointer used to point to the string containing the IP address (in Internet standard dot notation) written as third argument to command line
    bool flag = false; //boolean value initally set to false to indicate the flag has not been recieved from the server yet, when it is recieved this will be set to true
    ssize_t nr; //Stores the number of bytes read in from the message recieved from the server 

    //If argc does not equal 4 then all of the command arguments needed to run this program have not been entered, prints error to stderr and exits program (1 argument for name of program + 3 written in arguments)
    if (argc != 4){
        fprintf(stderr, "Must enter identification, port, and host IP address into the command line\n");
        exit(1);
    }
    
    //argv[0] contains the name of the program, so the three arguments written into the command line are stored in argv[1], argv[2], and argv[3]
    identification = argv[1]; //Sets the identification pointer to the string contained in argv[1] which is the UMass email address
    port = atoi(argv[2]); //Converts the string stored in argv[2] containing the port number to an integer and stores the integer in port
    hostIP = argv[3]; //Sets the hostIP pointer to the string contained in argv[3] which is the server's IP address (in Internet standard dot notation)

    //Calls the open_clientfd function with hostIP (will convert to network address format) and port as parameters to set up the client socket and establish communication with the server- returns the socket's file descriptor
    //Can send/recieve messages to/from server using the file descriptor returned and stored in clientfd
    int clientfd = open_clientfd(hostIP, port);

    char buf[MAXLINE]; //Sets the buf character array of size MAXLINE
    sprintf(buf, "cs230 HELLO %s\n", identification); //Sets the buf character array to the first string to be sent to the server to identify ourself
    
    //Sends the message stored in buf to the server using the socket file descriptor clientfd
    if (send(clientfd, buf, strlen(buf), 0) == -1){ //If send() returns -1 there was an error and a message is printed accordingly to stderr and the program is exited
        fprintf(stderr, "Error sending data to server\n");
        exit(1);
    }
    printf("Sending to server: %s", buf); //Prints the message stored in buf that was sent to the server

    //While loop that will continue recieving math problems from the server, calculate them, and send the answers back to the server until the server sends the flag and the program breaks from the while loop 
    while(1){
        //Recieves message sent from the server over the socket referred to by the file descriptor clientfd and reads the data into the buf character array up the MAXLINE bytes
        if ((nr = recv(clientfd, buf, MAXLINE, 0)) == -1){ //If recv() returns -1 there was an error and a message is printed accordingly to stderr and the program is exited
            fprintf(stderr, "Error recieving data from server\n");
            exit(1);
        }

        buf[nr] = '\0'; //Adds null terminating character to the end of the string read into buf (nr is the number of bytes read in- adds null character to end of string read in)
        printf("Recieved from server: %s", buf); //Prints the message stored in buf that was recieved from the server

        flag = check_flag(buf); //Calls the check_flag() function with buf to check if the server has sent the flag, if it has flag is set to true, otherwise it remains false
        //If flag is true, indicating the server has sent the flag, break out of the while loop
        if (flag){
            break;
        }

        int answer = mathproblem(buf); //Call the mathproblem() function with buf to extract the math problem out of the message stored in buf and calculate it and store the result in the integer answer
        sprintf(buf, "cs230 %i\n", answer); //Creates a new string containing the answer of the math problem and stores this in buf
        
        //Sends the message stored in buf containing the answer to the last math problem to the server using the socket file descriptor clientfd
        if (send(clientfd, buf, strlen(buf), 0) == -1){ //If send() returns -1 there was an error and a message is printed accordingly to stderr and the program is exited
            fprintf(stderr, "Error sending data to server\n");
            exit(1);
        }
        printf("Sending to server: %s\n", buf); //Prints the message stored in buf that was recieved from the server
    }

    if (check_flag(buf)){
        printf("You have captured the flag!\n"); //If the server sends the flag the program breaks out of the while loop it means the server has sent flag- message is printed that the flag has been captured
    }

    //Closes communication between the client and the server by closing the clientfd file descriptor
    if (close(clientfd) == -1){ //If close() returns -1 there was an error and a message is printed accordingly to stderr and the program is exited
        fprintf(stderr, "Error closing client socket\n");
        exit(1);
    }

    return 0;
}