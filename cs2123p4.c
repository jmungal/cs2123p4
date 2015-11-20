/******************************************************************
 cs2123p4.c by Justin Mungal
 
 Machine Improvement Proposal
 
 Purpose:
 
 Our current configuration utilizes a single server for processing 
 widgets. This server is a bottleneck and does many things.
 
 This program runs a simulation that utilizes two active servers at
 a time. The statistics printed at the end of the simulation can 
 then be used to compare our hypothetical simulated performance with 
 the performance that has been observer in our current configuration.
 
 Widgets are read in from standard input. The expected formatting is
 as follows:
 
 lWidgetNr iStep1tu iStep2tu iArrivalDelta iWhichServer
 %ld       %d       %d         %d           %d
 
 lWidgetNr - identifies a widget going through the system
 iStep1tu - time units for step 1 for this widget
 iStep2tu - time units for step 2 for this widget
 iArrivalDelta - delta time units before next widget arrives
 iWhichServer - determines which server to use when given an option:
                1 = server M, 2 = server W
 
 Returns:
 N/A
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "cs2123p4.h"

int main(int argc, char *argv[])
{
    int iTimeLimit = MAX_CLOCK_TIME;
    
    Simulation simulation = newSimulation();
    
    //process command line switches
    processCommandSwitches(argc, argv, simulation);
    
    //call populateSim to populate our sim from standard input
    generateArrival(simulation);
    
    //call run simulation
    runSimulation(simulation, iTimeLimit);

}
/************************* runSimulation **********************************
 void runSimulation(Simulation simulation, int iTimeLimit)
 Purpose: 
 The core function of the program. This function runs the
 simulation, and utilizes an array of helper functions to get the job done 
 in a modular fashion.
 Parameters:
 I  Simulation simulation           The simulation structure used to store
                                    simulation-related information.
 I  int iTimeLimit                  The maximum amount of time units that
                                    the simulation is allowed to run for.
                                    (not enforced).
                                    test
 **************************************************************************/
void runSimulation(Simulation simulation, int iTimeLimit)
{
    Event event;
    
    //declare our queues
    Queue queueW = newQueue("queueW");
    Queue queueM = newQueue("queueM");
    
    //declare and allocate our two servers
    Server serverW = newServer("serverW");
    Server serverM = newServer("serverM");
    
    //read in all of our arrival events from the input file
    //generateArrival(simulation);
    
    //Format header differently depending if we're in verbose mode or not
    if (simulation->bVerbose == TRUE)
        printf("Time\t Widget\t Event\n");
    else
        printf("Time\t       \t Event");
    
    //iterate while there are events to process in the linked-list
    while (removeLL(simulation->eventList, &event))
    {
        //advance clock to the next arrival time with each iteration
        simulation->iClock = event.iTime;
        
        switch(event.iEventType)
        {
            case EVT_ARRIVAL:
                if (simulation->bVerbose == TRUE)
                    printf("%d\t %ld\t Arrived\n", simulation->iClock\
                           , event.widget.lWidgetNr);
                
                if (event.widget.iWhichServer == 1)
                {
                    queueUp(simulation, queueM, &event.widget);
                    seize(simulation, queueM, serverM);
                }
                else
                {
                    queueUp(simulation, queueW, &event.widget);
                    seize(simulation, queueW, serverW);
                }
                break;
                
            case EVT_SERVERM_COMPLETE:
                release(simulation, queueM, serverM, &event.widget);
                leaveSystem(simulation, &event.widget);
                break;
                
            case EVT_SERVERW_COMPLETE:
                release(simulation, queueW, serverW, &event.widget);
                leaveSystem(simulation, &event.widget);
                break;
                
            default:
                ErrExit(ERR_ALGORITHM, "Unknown event type: %d\n", event.iEventType);
        }
    }
    
    //print simulation statistics
    printf("\n%d\t\t Simulation complete for alternative A.\n\n", simulation->iClock);
    printf("Average Queue Time for Server M: %.1f\n"\
           , (double) queueM->lQueueWaitSum / queueM->lQueueWidgetTotalCount);
    printf("Average Queue Time for Server W: %.1f\n"\
           , (double) queueW->lQueueWaitSum / queueW->lQueueWidgetTotalCount);
    printf("Average time in System: %.1f\n\n"\
           , (double) simulation->lSystemTimeSum / simulation->lWidgetCount);
    
    //The simulation is complete. Free up our memory
    free(queueW);
    free(queueM);
    free(serverW);
    free(serverM);
    free(simulation->eventList);
    free(simulation);
}
/****************************** seize *************************************
 void seize(Simulation simulation, Queue queue, Server server)
 Purpose:
 This function seizes a server if it isn't busy, and marks it as busy. This
 function is what enables widgets to be processed only when the server is 
 not already processing a widget (enforces serialization of widget processing).
 Parameters:
 I  Simulation simulation               The simulation structure
 I  Queue queue                         The specific queue we will pull from
                                        if the server isn't busy.
 I  Server server                       The server we are attempting to
                                        seize.
 Notes:
 The function utilizes a queue data structure to pull widgets that are
 waiting to be processed. The first widget in will be the first widget out.
 **************************************************************************/
void seize(Simulation simulation, Queue queue, Server server)
{
    //Only execute the body of the function (seize the server)
    //if the server is not marked busy
    if (server->bBusy == FALSE)
    {
        QElement qElement;
        Event eventServerComplete;
        int iWaited;
        
        //mark the server as busy, and remove a widget from the queue
        server->bBusy = TRUE;
        removeQ(queue, &qElement);
        
        //assign the widget to the server
        server->widget = qElement.widget;
        
        //update statistics
        iWaited = simulation->iClock - qElement.iEnterQTime;
        queue->lQueueWaitSum += iWaited;
        
        if (simulation->bVerbose == TRUE)
            printf("%d\t %ld\t Seized server %s\n", simulation->iClock\
                   , qElement.widget.lWidgetNr, server->szServerName);
        
        //set the values of our completion event
        eventServerComplete.iTime = simulation->iClock + server->widget.iStep1tu\
            + server->widget.iStep2tu;
        eventServerComplete.widget.lWidgetNr = server->widget.lWidgetNr;
        eventServerComplete.widget.iWhichServer = server->widget.iWhichServer;
        eventServerComplete.widget.iArrivalTime = server->widget.iArrivalTime;
        
        if (eventServerComplete.widget.iWhichServer == 1)
        {
            eventServerComplete.iEventType = EVT_SERVERM_COMPLETE;
            
            if (simulation->bVerbose == TRUE)
                printf("%d\t %ld\t Leave Queue M, waited %d\n"\
                       , simulation->iClock, qElement.widget.lWidgetNr, iWaited);
        }
        else
        {
            eventServerComplete.iEventType = EVT_SERVERW_COMPLETE;
            
            if (simulation->bVerbose == TRUE)
                printf("%d\t %ld\t Leave Queue W, waited %d\n"\
                       , simulation->iClock, qElement.widget.lWidgetNr, iWaited);
        }
        
        //finally, store the event in our linked-list
        insertOrderedLL(simulation->eventList, eventServerComplete);
    }
}
/************************** generateArrival *******************************
 void generateArrival(Simulation simulation)
 Purpose:
 Our arrival events are provided from input. This function scans the input
 (generally a file), and stores it in a linked list.
 Parameters:
 Simulation simulation                  The simulation structure
 Notes:
 The scanned in events are stored in the linked list, which is inside of 
 the simulation object. Timing related information is based on the 
 simulation clock and iArrivalDelta(provided by input).
 **************************************************************************/
void generateArrival(Simulation simulation)
{
    Event eventArrival;
    char szInputBuffer[MAX_LINE_SIZE];
    int iArrivalDelta;

    FILE *inputFile;
    inputFile = fopen(INPUT_FILE, "r");

    //set event types
    eventArrival.iEventType = EVT_ARRIVAL;
    
    //iterate with each \n delimited line from standard input
    //create an arrival event in our linked list with each iteration
    while (fgets(szInputBuffer, 100, inputFile) != NULL)
    {
        if (szInputBuffer[0] == '\n')
            break;

        //scan data into eventArrival and iArrivalDelta
        sscanf(szInputBuffer, "%ld %d %d %d %d", &eventArrival.widget.lWidgetNr\
               , &eventArrival.widget.iStep1tu, &eventArrival.widget.iStep2tu\
               , &iArrivalDelta, &eventArrival.widget.iWhichServer);
        
        //populate the rest of eventArrival
        eventArrival.iTime = simulation->iClock;
        eventArrival.widget.iArrivalTime = simulation->iClock;
        
        //insert the arrival into our linked list
        insertOrderedLL(simulation->eventList, eventArrival);
        
        //advance the clock so that the next arrival time is correct
        simulation->iClock = simulation->iClock + iArrivalDelta;
    }
}
/***************************** queueUp ************************************
 void queueUp(Simulation simulation, Queue queue, Widget *pWidget)
 Purpose:
 Place incoming widgets into a queue, which provides serialization in case
 a server is not available for immediate processing.
 Parameters:
 I Simulation simulation            The simulation structure
 I Queue queue                      The queue we will be inserting into
 I Widget *pWidget                  A pointer to the widget that will be
                                    pushed into the server
 Notes:
 This function also sets the time the widget entered the queue, which is 
 based on the current clock time, and increments the count of widgets 
 inserted into the queue. This is used for calculating simulation statistics.
 **************************************************************************/
void queueUp(Simulation simulation, Queue queue, Widget *pWidget)
{
    QElement qElement;
    qElement.widget = *pWidget;
    qElement.iEnterQTime = simulation->iClock;
    
    insertQ(queue, qElement);
    
    queue->lQueueWidgetTotalCount++;
    
    if (simulation->bVerbose == TRUE)
        printf("%d\t %ld\t Enter %s\n", simulation->iClock, qElement.widget.lWidgetNr\
        , queue->szQName);
}
/**************************** release *************************************
 void release(Simulation simulation, Queue queue, Server server, Widget *pWidget)
 Purpose:
 Relase a server so that it can process a widget
 Parameters:
 I Simulation simulation            The simulation structure
 I Queue queue                      The queue we will pull from
 I Server server                    The server that will be released
 I Widget *pWidget                  A pointer to the widget that the server
                                    processed
 Notes:
 In addition to releasing a server, this function also seizes the server so 
 that it can process another widget. If release did not do this, we would have
 to wait until the next clock cycle (which would skew our processing times)
 **************************************************************************/
void release(Simulation simulation, Queue queue, Server server, Widget *pWidget)
{
    server->bBusy = FALSE;
    
    if (simulation->bVerbose == TRUE)
        printf("%d\t %ld\t Released server W\n", simulation->iClock, pWidget->lWidgetNr);
    
    //don't seize if the queue is empty
    if (queue->pHead != NULL)
        seize(simulation, queue, server);
}
/************************ leaveSystem *************************************
 void leaveSystem(Simulation simulation, Widget *pWidget)
 Purpose:
 When a server has completed processing step1 and step2 for a widget, the 
 widget will be removed from the system.
 Parameters:
 I Simulation simulation            The simulation struct
 I Widget *pWidget                  A pointer to the widget being removed
 Notes:
 When a widget is removed from the system, we increment lWidgetCount (the
 amount of widgets processed by the system). We also calculate the amount 
 of time the widget was in the simulation, and then add that to
 lSystemTimeSum, which is the total time that widgets were in the system.
 **************************************************************************/
void leaveSystem(Simulation simulation, Widget *pWidget)
{
    simulation->lWidgetCount++;
    
    //total widget time is when is when it arrived subtracted from the
    //the current clock time
    int iSpentInSystem = simulation->iClock - pWidget->iArrivalTime;
    simulation->lSystemTimeSum += iSpentInSystem;
    
    if (simulation->bVerbose == TRUE)
        printf("%d\t %ld\t Exit System, in system %d\n", simulation->iClock\
               , pWidget->lWidgetNr, iSpentInSystem);
}