/* proxy.c - pseudocode/structure for proxy program that uses TCP by Dr. Yi Gu */
// any header files here
#ifndef unix
#define WIN32
#include <windows.h>
#include <winsock.h>
#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PROTOPORT 5200
#define QLEN 6

void HandleClientConnection(int newsocket1);

int main() // works as "server" side
{
    struct hostent *ptrh;   /* pointer to a host table entry       */
    struct protoent *ptrp;  /* pointer to a protocol table entry   */
    struct sockaddr_in sad; /* structure to hold server's address  */
    struct sockaddr_in cad; /* structure to hold client's address  */
    int sd0, sd1;           /* socket descriptors                  */
    int port;               /* protocol port number                */
    int addrlen;            /* length of address                   */
    char buf[1000];

    pid_t pid; //This wont work, nor will fork on windows. Need to use CreateThread or CreateProcess

    memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;             /* set family to Internet   */
    sad.sin_addr.s_addr = INADDR_ANY;     /* set the local IP address(not specify, reserved */
    port = PROTOPORT;                     /* use the specified port number */
    if (port > 0)                         /* test for illegal value       */
    {
        sad.sin_port = htons((u_short)port);
    }
    else
    { /* print error message and exit */
        fprintf(stderr, "bad port number %s\n", port);
        exit(1);
    }

    printf("Beginning daemon process\n");

    /* Map TCP transport protocol name to protocol number */
    //Seems to work
    if (((int)(ptrp = getprotobyname("tcp"))) == 0)
    {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }
    else
    {
        printf("TCP mapped\n"); //For testing
    }
    //getprotobyname()

    /* Create a socket */
    sd0 = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd0 < 0)
    {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    else
    {
        printf("Host socket created\n");
    }
    // socket();

    /* Bind a local address to the socket */
    //If you don't unbind or close the socket, sometimes the port will stay open, whoopsie
    if (bind(sd0, (struct sockaddr *)&sad, sizeof(sad)) < 0)
    {
        fprintf(stderr, "bind failed\n");
        printf("test");
        exit(1);
    }
    else
    {
        printf("Host socket is bound\n");
    }
    // bind()

    /* Listen at the port */
    if (listen(sd0, QLEN) < 0)
    {
        fprintf(stderr, "listen failed\n");
        exit(1);
    }
    else
    {
        printf("Now listening");
    }
    // listen()

    //test
    /* Main server loop - accept and handle requests */
    //Second socket will be used here
    while (1)
    {
        // accept request
        addrlen = sizeof(cad);
        printf("\nI'm waiting for connections ...\n");
        fflush(stdout);
        if ((sd1 = accept(sd0, (struct sockaddr *)&cad, &addrlen)) < 0)
        {
            fprintf(stderr, "accept failed\n");
            exit(1);
        }
        else
        {
            printf("Accepted a connection\n");
        }

        fflush(stdout);

        //This works
        //HandleClientConnection(sd1);

        //Now work on this:
        // fork a process
        pid = fork();

        if (pid == 0)
        { //Child process
            //Close old socket
            closesocket(sd0);
            printf("child process\n");
            //HandleClientConnection(newsocket1);
            HandleClientConnection(sd1);
            break; //This was commented out?
        }
        else
        { //Parent process
            printf("parent process\n");
            //Close the new socket
            closesocket(sd1);
            continue;
        }
    }

    //closesocket(sd0);
    //closesocket(sd1);
    //Maybe close the other socket here?
    return 0;
}

//Server side implementation SHOULD be working now

// implement "HandleClientConnection(new socket1)" - works as "client" side
void HandleClientConnection(int newsocket1)
{ // general socket settings at the client side
    //Note to self: look at client1.pdf for help, maybe
    struct hostent *ptrh;   /* pointer to a host table entry       */
    struct protoent *ptrp;  /* pointer to a protocol table entry   */
    struct sockaddr_in sad; /* structure to hold an IP address     */
    int sd;                 /* socket descriptor                   */
    int port;               /* protocol port number                */
    char *host;             /* pointer to host name                */
    int i;                  /* number of characters read           */
    char buf[1000];
    char nbuf[1000];
    char clibuf[1000];
    char servbuf[1000];
    char *strgrab;

    char hostactual[100];

    fd_set testfd;
    struct timeval timeout;
    int re_select;
    int maxfds = 5;

    FILE *fptr;

    FD_ZERO(&testfd);

    //This procs
    printf("handling a client right now\n");

    memset(buf, '\0', 1000);            //Keeps funny symbols out of buffer
    i = recv(newsocket1, buf, 1000, 0); //i holds the number of characters read on completion

    printf("This is the buffer to start: %s\n", buf);
    printf("This is the length of the buffer string: %d\n", strlen(buf));
    strcpy(nbuf, buf);

    while (1) //get the real server's IP addr from "GET" command
    {         // read to buffer and compare, such as

        //Read first line to check for GET or CONNECT in here
        if (strncmp("GET", nbuf, 3) == 0 || strncmp("CONNECT", nbuf, 7) == 0)
        {
            printf("Connection request made: %s \n", nbuf);

            fptr = fopen("HTTPHeader.txt", "a");
            fputs(buf, fptr);
            fclose(fptr);

            while (1)
            {
                //...; // read more
                strgrab = strstr(nbuf, "Host");

                //Read second line for host name here
                if (strncmp("Host", strgrab, 4) == 0)
                {
                    host = strtok(strgrab, " ");
                    host = strtok(NULL, "\n");
                    printf("%s is the host\n", host);

                    break;
                }
            }
            break;
        }
    }

    /* Convert real host name to equivalent IP address and copy to sad. */ // gethostbyname();
    char buffer[strlen(host) - 1];
    memcpy(buffer, host, strlen(host) - 1); //Removes the null terminator from the string for handling

    snprintf(hostactual, sizeof(buffer) + 1, "%s", buffer);

    printf("%s is the host buffer\n", hostactual);
    //Print host buffer for testing

    ptrh = gethostbyname(hostactual);
    if (((char *)ptrh) == NULL)
    {
        fprintf(stderr, "invalid host: %s\n", hostactual);
        exit(1);
    }
    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
    memset(buffer, '\0', sizeof(buffer)); //Cleans and keeps funny symbols out of buffer (maybe)

    /* Map TCP transport protocol name to protocol number. */
    if (((int)(ptrp = getprotobyname("tcp"))) == 0)
    {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }

    /* Create a newsocket2. */ // socket();

    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0)
    {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    //Should be correct, as seen in client1.pdf example

    /* Connect the socket to the specified server. */ // connect()
                                                      //This was failing here, possibly because family and port weren't set?
    sad.sin_family = AF_INET;                         //If these aren't set, stuff goes awry
    sad.sin_port = htons(80);
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0)
    {
        fprintf(stderr, "Handled connection failed\n");
        exit(1);
    }
    else
    {
        printf("Handled the connection!!!\n");
    }

    //Send request to the host
    int n = write(sd, buf, sizeof(buf));
    memset(buf, '\0', sizeof(buf));

    FD_SET(newsocket1, &testfd);
    FD_SET(sd, &testfd);

    timeout.tv_sec = 3600;
    timeout.tv_usec = 500;

    while (1)
    {
        re_select = select(maxfds, &testfd, NULL, NULL, &timeout);
        if (re_select < 0)
        {
            fprintf(stderr, "Select failed\n");
            exit(1);
        }
        else if (re_select == 0)
        {
            fprintf(stderr, "Time out\n");
            exit(1);
        }
        else
        {
            if (FD_ISSET(newsocket1, &testfd))
            {
                memset(buf, 0, sizeof(buf));
                i = recv(newsocket1, buf, sizeof(buf), 0);
                if (i <= 0)
                    break;
                send(sd, buf, i, 0);
            }
            else if (FD_ISSET(sd, &testfd))
            {
                memset(buf, 0, sizeof(buf));
                i = recv(sd, buf, sizeof(buf), 0);
                if (i <= 0)
                    break;
                send(newsocket1, buf, i, 0);
            }
        }
    }

    close(sd);
}
