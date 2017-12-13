/***************************Programming Assignment 4 Client*******************
 * Programmer:     Ilyas Khurshid
 *
 * Course:         CSCI 4534.01
 *
 * Date:           November 30, 2017
 *
 * Assignment:     Programming Assignment #4
 *
 * Environment:    XCode running on macOS Sierra Version 10.13.1
 *
 * Files Included: a:\\program4_Server.c,  a:\\program4_Client.c,    a:\\README.txt,
 *                 a:\\testInputOutput.txt
 *
 * Purpose:        Multiple clients send a burst and arrival time to the server. The
 *                 server acts as a scheduler and performs round-robin scheduling for as many clients as
 *                 possible. When a process finishes, it goes to the wait queue where FCFS is performed.
 *                 The server then sends information back and is printed by the clients.
 *
 * Input:          User is asked to insert number of clients and time quant.
 *
 * Preconditions:  User inputs correctly when prompted for information
 *
 * Output:         Finish time of each job and CPU utilization
 *
 * Postconditions: None
 *
 * Algorithm:
*
*                 struct{ClientPID, clientJob[5] finishTime}
*
*                 Open common FIFO to write
*                 Ask user for an int as burst and IO
*                 get clientPid
*                 create Private FIFO
*                 open private FIFO to read
*                 Send struct to Server
*
*                 Wait for response from Server...
*
*                 Read struct from Server
*
*                 Close common FIFO
*                 Close to_Client FIFO
*
*
*                          Estimated                Actual
* Design:                    1 hr                   1 hr
* Implementation:            1.5 hr                 3 hr
* Testing:                  30 min                  30 min
*
***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

main (void)
{
    int to_Server;    // to write to server
    int to_Client;    // to read from server
    char temp[14];    // store client ID
    int clientID;     // store client ID
    int num_processes;
    int i = 0;

    // struct definition
    struct values
    {
        char privateFIFO[14]; // Client ID
        int clientJob[5]; // Burst and I/O
        int finishTime;
    }input;


    // Open server write only
    if((to_Server=open("FIFO_to_server", O_WRONLY))<0)
        printf("cant open fifo to write");

        for(i = 0; i < 5; i++)
        {
            input.clientJob[i] = 0;
        }

    i = 0;

    while(i == 0)
    {
        printf("Client: How many processes do you want to do? Must be odd and max 5 allowed.\n");
        printf("User: ");
        scanf("%i", &(num_processes)); // read in answer

        if(num_processes == 1 || num_processes == 3 || num_processes == 5)
        {
            i = 1;;
        }
        else
        {
            printf("Client: Invalid input.\n");
        }
    }

    for(i = 0; i < num_processes; i++)
    {
        if(i == 0 || i == 2 || i == 4)
        {
            printf("Client: Please enter a number for the burst.\n"); // Ask user for number
            printf("User: ");
            scanf("%i", &(input.clientJob[i])); // read in answer
        }
        else
        {
            printf("Client: Please enter a number for the I/O.\n"); // Ask user for number
            printf("User: ");
            scanf("%i", &(input.clientJob[i])); // read in answer
        }
    }

    // Get and store client id in struct
    clientID = getpid();
    strcpy(input.privateFIFO, "FIFO_");   //get name of private FIFO
    sprintf(temp, "%d", clientID);
    strcat(input.privateFIFO, temp);
    printf("\nClient: FIFO name is %s ", input.privateFIFO);
    write(to_Server, &input, sizeof(input)); // send struct to server

    printf("\nClient: Sent information to server. Now waiting for response.");

    // Create FIFO to read only
    if ((mkfifo(input.privateFIFO,0666)<0 && errno != EEXIST))
    {
        perror("Client: Cant create FIFO_to_client");
        exit(-1);
    }

    if((to_Client=open(input.privateFIFO, O_RDONLY))<0)
        printf("Client: Cant open fifo to read");

    read(to_Client, &input, sizeof(input)); // read from server

    printf("\n\n\nClient: Finish time for job was %i", input.finishTime);

    //Close both FIFOs
    close(to_Server);
    close(to_Client);

    printf ("\nClient: shutting down.\n");
}
