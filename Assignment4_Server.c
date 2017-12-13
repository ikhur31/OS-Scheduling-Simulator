/***************************Programming Assignment 4 Server*******************
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
 *                 struct{ClientPID, clientJob, finishTime}
 *                 int clock;
 *
 *                 Open common FIFO to read
 *                 Ask user for numofClients and timeQuant
 *
 *                 for(each client)
 *                    read struct from common FIFO
 *
 *                 Create queue lists called Ready with all client structs
 *                 Create empty wait list
 *
 *                 while(all client jobs aren’t 0)
 *                     if(wait.head isn’t empty)
 *                        wait.head--;
 *
 *                     if(Ready.head isn’t empty)
 *                        ready.head--;
 *
 *                     if(ready.head is finished)
 *                        if(ready.head has an I/O)
 *                           send to waitQueue
 *
 *                     if(wait.head is finished)
 *                        send to readyQueue
 *
 *                     if(ready and wait are both empty)
 *                        break;
 *                     else
 *                        clock++;
 *
 *                 calculate cpu utilization
 *                 show finishTime and cpu utilization
 *
 *                 for(each client)
 *                     open client private FIFO
 *                     save finishTime to each client’s struct
 *                     send struct back
 *                     Close to_Client FIFO
 *
 *                 Close common FIFO
 *
 *
 *                          Estimated                Actual
 * Design:                    1 hr                   1 hr
 * Implementation:            1.5 hr                 3 hr
 * Testing:                  30 min                  30 min
 ***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

typedef struct pcb {  /*structure for PCBs */

    char str[12];

    int process[5];

} PCB;

typedef struct node{  /*Nodes stored in the linked list*/

    struct pcb elements;

    struct node *next;

} Node;

typedef struct queue{ /*A struct facilitates passing a queue as an argument*/

    Node *head;       /*Pointer to the first node holding the queue's data*/

    Node *tail;       /*Pointer to the last node holding the queue's data*/

    int sz;           /*Number of nodes in the queue*/

} Queue;



int size( Queue *Ready ){

    return Ready->sz;

}



int isEmpty( Queue *Ready ){

    if( Ready->sz == 0 ) return 1;

    return 0;

}



void enqueue( Queue *Ready, struct pcb elements){

    Node *v = (Node*)malloc(sizeof(Node));/*Allocate memory for the Node*/

    if( !v ){

        printf("ERROR: Insufficient memory\n");

        return;

    }

    v->elements = elements;

    v->next = NULL;

    if( isEmpty(Ready) ) Ready->head = v;

    else Ready->tail->next = v;

    Ready->tail = v;

    Ready->sz++;

}



PCB dequeue( Queue *Ready ){

    PCB temp;
    Node *oldHead;



    if( isEmpty(Ready) ){

        printf("ERROR: Queue is empty\n");

        return temp;

    }

    oldHead = Ready->head;

    temp = Ready->head->elements;

    Ready->head = Ready->head->next;

    free(oldHead);

    Ready->sz--;

    return temp;

}



PCB first( Queue *Ready ){

    PCB temp;
    if( isEmpty(Ready) ){

        printf("ERROR: Queue is empty\n");

        return temp;

    }

    printf("Head of the list is:%s\n", Ready->head->elements.str);

    return temp;
}



void destroyQueue( Queue *Ready ){

    while( !isEmpty(Ready) ) dequeue(Ready);

}



/*A different visit function must be used for each different datatype.*/

/*The name of the appropriate visit function is passed as an argument */

/*to traverseQueue.                                                   */

void visitNode( PCB elements ){

    printf("PCB name and CPU burst/IO: ");
    printf(" %s",elements.str);

    int i;
    for(i = 0; i < 5; i++)
    {
      printf(" %d", elements.process[i]);
    }

    printf("\n");

}



/*The following function isn't part of the Queue ADT, however*/

/*it can be useful for debugging.                            */

void traverseQueue( Queue *Ready ){

    Node *current = Ready->head;

    while( current ){

        visitNode(current->elements);

        current = current->next;

    }

    if(Ready->head == NULL)
    {
      printf("Queue is Empty");
    }

    printf("\n");

}

main (void)
{
    int to_Server;      // to read from client
    int to_Client;      // to write to client
    int finish;         // lets me know that client is done
    int numOfClients;   // because C needs this defined as int
    int timeQuant;
    int clock = 0;
    int i;
    int j;
    int timeQuantCounter;

    // struct definition/xuk
    struct values
    {
        char privateFIFO[14]; // Client ID
        int clientJob[5]; // Burst and I/O
        int finishTime;
    }input;

    // Create FIFO to read only
    if ((mkfifo("FIFO_to_server",0666)<0 && errno != EEXIST))
    {
        perror("Server: Cant create FIFO_to_server");
        exit(-1);
    }

    // Open FIFO to read only
    if((to_Server=open("FIFO_to_server", O_RDONLY))<0)
        printf("Server: Cant open fifo to write");

    printf("Server: How many clients?\n");
    printf("User: ");
    scanf("%i", &(numOfClients)); // read in answer

    int timeFinish[numOfClients];

    printf("Server: What is the time quant?\n");
    printf("User: ");
    scanf("%i", &(timeQuant)); // read in answer


    char pFIFO[numOfClients][14];  // store each client's ID
    Queue ReadyQueue;
    ReadyQueue.head = NULL;
    ReadyQueue.tail = NULL;
    ReadyQueue.sz = 0;

    Queue WaitQueue;
    WaitQueue.head = NULL;
    WaitQueue.tail = NULL;
    WaitQueue.sz = 0;

    PCB PCB1;
    sprintf(PCB1.str, "First PCB");
    PCB PCB2;
    sprintf(PCB2.str, "Second PCB");
    PCB PCB3;
    sprintf(PCB3.str, "Third PCB");

    for(i = 0; i < 5; i++)
    {
        PCB1.process[i] = 0;
        PCB2.process[i] = 0;
        PCB3.process[i] = 0;
    }

    // Read from each client and store info
    for(i = 0; i < numOfClients; i++)
    {
        finish = read(to_Server, &input, sizeof(input));     //read the struct
        strcpy(pFIFO[i],input.privateFIFO);
        printf("\nServer: Successfully read struct from client %s", input.privateFIFO);

        if(i == 0)
        {
            for(j=0;j<5;j++)
            {
                PCB1.process[j] = input.clientJob[j];
            }
        }
        else if(i == 1)
        {
            for(j=0;j<5;j++)
            {
                PCB2.process[j] = input.clientJob[j];
            }
        }
        else
        {
            for(j=0;j<5;j++)
            {
                PCB3.process[j] = input.clientJob[j];
            }
        }
    }

    enqueue(&ReadyQueue,PCB1);

    enqueue(&ReadyQueue,PCB2);

    enqueue(&ReadyQueue,PCB3);

    printf("\n\nTraversing the queue: \n");

    traverseQueue(&ReadyQueue); // NEED TO FIX

    printf("-------------------------------------------------------------\n");

    timeQuantCounter = timeQuant;

    char ganttChartBurst[100] = "";
    char ganttChartIO[100] = "";
    char ganttChartFinishTime[100] = "";
    int ganttChartFinishTimeNumber[100] = {0};
    int ganttChartBurstPCB[100] = {4};
    int ganttChartIOPCB[100] = {4};
    char ganttChartFinishPCB[100] = "";
    int showQueues = 0;
    int ReadyQueueIdle = 0;

    ganttChartBurst[0] = '|';
    ganttChartBurstPCB[0] = 1;
    ganttChartIO[0] = ' ';
    ganttChartFinishTime[0] = 'd';

    int skip = 0;
    int loop = 1;

    while(loop == 1)
    {
        clock++;

        if(WaitQueue.head == NULL) // If wait is empty
        {
            ganttChartIO[clock] = ' ';
            ganttChartFinishTime[clock] = ' ';
        }
        else if(WaitQueue.head->elements.process[0] == 1)
        {
            ganttChartIO[clock] = '|';
            ganttChartFinishTime[clock] = 'd';
            ganttChartFinishTimeNumber[clock] = clock;

            if(WaitQueue.sz > 1)
            {
                if(strcmp(WaitQueue.head->next->elements.str, "First PCB") == 0)
                {
                    ganttChartIOPCB[clock] = 1;
                }
                else if(strcmp(WaitQueue.head->next->elements.str, "Second PCB") == 0)
                {
                    ganttChartIOPCB[clock] = 2;
                }
                else
                {
                    ganttChartIOPCB[clock] = 3;
                }
            }

            if(ReadyQueue.head == NULL)
            {
                ganttChartBurst[clock] = '|';

                if(strcmp(WaitQueue.head->elements.str, "First PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 1;
                }
                else if(strcmp(WaitQueue.head->elements.str, "Second PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 2;
                }
                else
                {
                    ganttChartBurstPCB[clock] = 3;
                }

                skip = 1;
            }
        }
        else
        {
            ganttChartIO[clock] = '-';
            ganttChartFinishTime[clock] = ' ';
        }

        if(ReadyQueue.head == NULL) // If ready is empty
        {
            if(!(ganttChartBurst[clock] == '|'))
            {
                ganttChartBurst[clock] = ' ';
            }

            ReadyQueueIdle++;
        }
        else if(ReadyQueue.head->elements.process[0] == 1)
        {
            showQueues = 1;
            ganttChartBurst[clock] = '|';
            ganttChartFinishTime[clock] = 'd';
            ganttChartFinishTimeNumber[clock] = clock;

            if(ReadyQueue.head->next != NULL)
            {
                if(strcmp(ReadyQueue.head->next->elements.str, "First PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 1;
                }
                else if(strcmp(ReadyQueue.head->next->elements.str, "Second PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 2;
                }
                else
                {
                    ganttChartBurstPCB[clock] = 3;
                }
            }



            if(WaitQueue.head == NULL && !(ReadyQueue.head->elements.process[1] == 0))
            {
                ganttChartIO[clock] = '|';

                if(strcmp(ReadyQueue.head->elements.str, "First PCB") == 0)
                {
                    ganttChartIOPCB[clock] = 1;
                }
                else if(strcmp(ReadyQueue.head->elements.str, "Second PCB") == 0)
                {
                    ganttChartIOPCB[clock] = 2;
                }
                else
                {
                    ganttChartIOPCB[clock] = 3;
                }
            }
        }
        else if(timeQuantCounter == 1)
        {
            ganttChartBurst[clock] = '|';
            ganttChartFinishTime[clock] = 'd';
            ganttChartFinishTimeNumber[clock] = clock;

            if(ReadyQueue.head->next == NULL)
            {
                if(strcmp(ReadyQueue.head->elements.str, "First PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 1;
                }
                else if(strcmp(ReadyQueue.head->elements.str, "Second PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 2;
                }
                else
                {
                    ganttChartBurstPCB[clock] = 3;
                }
            }
            else
            {
                if(strcmp(ReadyQueue.head->next->elements.str, "First PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 1;
                }
                else if(strcmp(ReadyQueue.head->next->elements.str, "Second PCB") == 0)
                {
                    ganttChartBurstPCB[clock] = 2;
                }
                else
                {
                    ganttChartBurstPCB[clock] = 3;
                }
            }
        }
        else
        {
            ganttChartBurst[clock] = '-';
        }

        if(WaitQueue.head == NULL) // If wait is empty
        {
            if(ReadyQueue.head == NULL) // If Ready is empty
            {
                loop = 0;
            }
        }

        if(!(WaitQueue.head == NULL)) // If wait isn't empty, do process
        {
            WaitQueue.head->elements.process[0]--;

            if(WaitQueue.head->elements.process[0] == 0) // If I/O is done
            {
                if(strcmp(WaitQueue.head->elements.str, "First PCB") == 0)
                {
                    for(i = 0; i < 4; i++) // move elements down 1 and make last one 0
                    {
                        PCB1.process[i] = PCB1.process[i+1];
                    }

                    PCB1.process[4] = 0;
                    enqueue(&ReadyQueue, PCB1);
                }
                else if(strcmp(WaitQueue.head->elements.str, "Second PCB") == 0)
                {
                    for(i = 0; i < 4; i++) // move elements down 1 and make last one 0
                    {
                        PCB2.process[i] = PCB2.process[i+1];
                    }

                    PCB2.process[4] = 0;
                    enqueue(&ReadyQueue, PCB2);
                }
                else
                {
                    for(i = 0; i < 4; i++) // move elements down 1 and make last one 0
                    {
                        PCB3.process[i] = PCB3.process[i+1];
                    }

                    PCB3.process[4] = 0;
                    enqueue(&ReadyQueue, PCB3);
                }

                dequeue(&WaitQueue);
            }
        }

        if(!(ReadyQueue.head == NULL) && skip == 0) // If Ready isn't empty, do process
        {
            ReadyQueue.head->elements.process[0]--;
            timeQuantCounter--;

            if(ReadyQueue.head->elements.process[0] == 0) // If burst is done
            {
                if(ReadyQueue.head->elements.process[1] == 0) // No I/O so job is done
                {
                    if(strcmp(ReadyQueue.head->elements.str, "First PCB") == 0)
                    {
                        timeFinish[0] = clock;
                        ganttChartFinishPCB[clock] = '1';
                    }
                    else if(strcmp(ReadyQueue.head->elements.str, "Second PCB") == 0)
                    {
                        timeFinish[1] = clock;
                        ganttChartFinishPCB[clock] = '2';
                    }
                    else
                    {
                        timeFinish[2] = clock;
                        ganttChartFinishPCB[clock] = '3';
                    }
                }
                else // There is an I/O so send to Wait
                {
                    if(strcmp(ReadyQueue.head->elements.str, "First PCB") == 0)
                    {
                        for(i = 0; i < 4; i++) // move elements down 1 and make last one 0
                        {
                            PCB1.process[i] = PCB1.process[i+1];
                        }

                        PCB1.process[4] = ReadyQueue.head->elements.process[0];
                        enqueue(&WaitQueue, PCB1);
                    }
                    else if(strcmp(ReadyQueue.head->elements.str, "Second PCB") == 0)
                    {
                        for(i = 0; i < 4; i++) // move elements down 1 and make last one 0
                        {
                            PCB2.process[i] = PCB2.process[i+1];
                        }

                        PCB2.process[4] = ReadyQueue.head->elements.process[0];
                        enqueue(&WaitQueue, PCB2);
                    }
                    else
                    {
                        for(i = 0; i < 4; i++) // move elements down 1 and make last one 0
                        {
                            PCB3.process[i] = PCB3.process[i+1];
                        }

                        PCB3.process[4] = ReadyQueue.head->elements.process[0];
                        enqueue(&WaitQueue, PCB3);
                    }
                }

                dequeue(&ReadyQueue);
                timeQuantCounter = timeQuant;
            }
            else if(timeQuantCounter == 0) // Time quant is done so switch Ready if possible
            {
                if(strcmp(ReadyQueue.head->elements.str, "First PCB") == 0)
                {
                    for(i = 0; i < 5; i++) // Update burst
                    {
                        PCB1.process[i] = ReadyQueue.head->elements.process[i];
                    }

                    enqueue(&ReadyQueue,PCB1);
                }
                else if(strcmp(ReadyQueue.head->elements.str, "Second PCB") == 0)
                {
                    for(i = 0; i < 5; i++) // Update burst
                    {
                        PCB2.process[i] = ReadyQueue.head->elements.process[i];
                    }

                    enqueue(&ReadyQueue,PCB2);
                }
                else
                {
                    for(i = 0; i < 5; i++) // Update burst
                    {
                        PCB3.process[i] = ReadyQueue.head->elements.process[i];
                    }

                    enqueue(&ReadyQueue,PCB3);
                }

                dequeue(&ReadyQueue);
                timeQuantCounter = timeQuant;
            }
        }
        else if(skip == 0)
        {
            timeQuantCounter--;
        }

        if(skip == 1)
            skip = 0;

        if(showQueues == 1)
        {
            printf("\n\n       ");
            printf("\nClock = %i", clock);
            printf("\nReady Queue: ");

            Node *current = ReadyQueue.head;

            while(current != NULL)
            {
                if(strcmp(current->elements.str, "First PCB") == 0)
                {
                    printf("PCB1, ");
                }
                else if(strcmp(current->elements.str, "Second PCB") == 0)
                {
                    printf("PCB2, ");
                }
                else
                {
                    printf("PCB3, ");
                }

                current = current->next;
            }

            printf("\nWait Queue: ");

            current = WaitQueue.head;

            while(current != NULL)
            {
                if(strcmp(current->elements.str, "First PCB") == 0)
                {
                    printf("PCB1, ");
                }
                else if(strcmp(current->elements.str, "Second PCB") == 0)
                {
                    printf("PCB2, ");
                }
                else
                {
                    printf("PCB3, ");
                }

                current = current->next;
            }

            showQueues = 0;
        }
    }

    printf("\n\n\n       ");

    for(i = 0; i < clock+1; i++)
    {
        int noSpace = 0;

        if(ganttChartFinishPCB[i] == '1')
        {
            printf("P1");
            noSpace = 1;
        }
        else if(ganttChartFinishPCB[i] == '2')
        {
            printf("P2");
            noSpace = 1;
        }
        else if(ganttChartFinishPCB[i] == '3')
        {
            printf("P3");
            noSpace = 1;
        }
        else
        {
            printf(" ");
        }

        if(noSpace == 0)
            printf(" ");
    }

    printf("\n       ");





    for(i = 0; i < clock+1; i++)
    {
        if(ganttChartFinishPCB[i] == '1' || ganttChartFinishPCB[i] == '2' || ganttChartFinishPCB[i] == '3')
        {
            printf("^");
        }
        else
        {
            printf(" ");
        }

        printf(" ");
    }

    printf("\n       ");

    for(i = 0; i < clock+1; i++)
    {
        if(ganttChartFinishPCB[i] == '1' || ganttChartFinishPCB[i] == '2' || ganttChartFinishPCB[i] == '3')
        {
            printf("|");
        }
        else
        {
            printf(" ");
        }

        printf(" ");
    }

    printf("\nBurst: ");

    for(i = 0; i < clock+1; i++)
    {
        printf("%c ",ganttChartBurst[i]);
    }

    printf("\n       ");

    for(i = 0; i < clock+1; i++)
    {
        int noSpace = 0;

        if(ganttChartBurstPCB[i] == 1)
        {
            printf("P1");
            noSpace = 1;
        }
        else if(ganttChartBurstPCB[i] == 2)
        {
            printf("P2");
            noSpace = 1;
        }
        else if(ganttChartBurstPCB[i] == 3)
        {
            printf("P3");
            noSpace = 1;
        }
        else
        {
            printf(" ");
        }

        if(noSpace == 0)
        {
            printf(" ");
        }
    }

    printf("\n\nI/O:   ");

    for(i = 0; i < clock+1; i++)
    {
        printf("%c ",ganttChartIO[i]);
    }

    printf("\n       ");

    for(i = 0; i < clock+1; i++)
    {
        int noSpace = 0;

        if(ganttChartIOPCB[i] == 1)
        {
            printf("P1");
            noSpace = 1;
        }
        else if(ganttChartIOPCB[i] == 2)
        {
            printf("P2");
            noSpace = 1;
        }
        else if(ganttChartIOPCB[i] == 3)
        {
            printf("P3");
            noSpace = 1;
        }
        else
        {
            printf(" ");
        }

        if(noSpace == 0)
        {
            printf(" ");
        }
    }

    printf("\nClock: ");

    for(i = 0; i < clock+1; i++)
    {
        int noSpace = 0;

        if(i%5==0 && i > 5)
        {
            printf("%d", i);
            noSpace = 1;
        }
        else if(i%5==0)
        {
            printf("%d", i);
        }
        else
        {
            printf(" ");
        }

        if(noSpace == 0)
        {
            printf(" ");
        }

    }


    for(i = 0; i < numOfClients; i++)
    {
		printf("\n\n\nFinish time for PCB %i: %i", i+1, timeFinish[i]);

        input.finishTime = timeFinish[i];

        if((to_Client=open(pFIFO[i], O_WRONLY))<0)
            printf("Server: Cant open fifo to read");

        write(to_Client, &input, sizeof(input));
        printf("\nServer: Information sent to client %i", i);

        close(to_Client);
        unlink(pFIFO[i]);
    }

    float temp = clock-1;
    float temp2 = ReadyQueueIdle-1;

    printf("\nCPU Utilization: %f", (((temp - temp2) / temp)) * 100);

    // Close FIFO to server
    close(to_Server);
    unlink("FIFO_to_server");

    printf("\nServer: Shutting down.\n");
}
