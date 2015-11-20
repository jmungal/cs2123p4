/******************************************************************
 cs2123p4_helper.c by Justin Mungal
 
 Machine Improvement Proposal - Helper Functions
 
 Purpose:
 
 This file contains the "helper" functions that are used throughout
 the program, but fall outside the core functions of the simulation.
 
 Returns:
 N/A
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "cs2123p4.h"

/******************** newSimulation / NewServer ***************************
 Simulation newSimulation()
 Server newServer(char szServerNm[])
 Purpose:
 These functions are used for allocating new simulation and server structures,
 along with setting appropriate initial values to various structure elements.
 Parameters - newServer:
 I      char szServerNm[]               name of server
 Notes - newSimulation:
 Various numeric values are set to zero. These will be summed and incremented
 throughout the simulation, and must be set to zero when the simulation begins.
 Notes - newServer:
 A newly allocated server will have bBusy set to FALSE. This is so that the
 new server can service widgets right away.
 **************************************************************************/
Simulation newSimulation()
{
    Simulation s = (Simulation) malloc(sizeof(SimulationImp));
    s->iClock = 0;
    s->lWidgetCount = 0;
    s->lSystemTimeSum = 0;
    s->eventList = newLinkedList();
    return s;
}
//create a new server, and mark it as not busy
Server newServer(char szServerNm[])
{
    Server s = (Server)malloc(sizeof(ServerImp));
    strcpy(s->szServerName,szServerNm);
    s->bBusy = FALSE;
    return s;
}
/******************** processCommandSwitches *****************************
 void processCommandSwitches(int argc, char *argv[])
 Purpose:
 Checks the syntax of command line arguments and returns the filenames.
 If any switches are unknown, it exits with an error.
 Parameters:
 I   int argc                        count of command line arguments
 I   char *argv[]                    array of command line arguments
 Notes:
 If a -? switch is passed, the usage is printed and the program exits
 with USAGE_ONLY.
 If a syntax error is encountered (e.g., unknown switch), the program
 prints a message to stderr and exits with ERR_COMMAND_LINE_SYNTAX.
 **************************************************************************/
void processCommandSwitches(int argc, char *argv[], Simulation simulation)
{
    int i;
    // Examine each of the command arguments other than the name of the program.
    for (i = 1; i < argc; i++)
    {
        // check for a switch
        if (argv[i][0] != '-')
            exitUsage(i, ERR_EXPECTED_SWITCH, argv[i]);
        // determine which switch it is
        switch (argv[i][1])
        {
            case 'v':
                simulation->bVerbose = TRUE;
                break;
            case '?':
                exitUsage(USAGE_ONLY, "", "");
                break;
            default:
                exitUsage(i, ERR_EXPECTED_SWITCH, argv[i]);
        }
    }
}
/******************** ErrExit **************************************
 void ErrExit(int iexitRC, char szFmt[], ... )
 Purpose:
 Prints an error message defined by the printf-like szFmt and the
 corresponding arguments to that function.  The number of
 arguments after szFmt varies dependent on the format codes in
 szFmt.
 It also exits the program with the specified exit return code.
 Parameters:
 I   int iexitRC             Exit return code for the program
 I   char szFmt[]            This contains the message to be printed
 and format codes (just like printf) for
 values that we want to print.
 I   ...                     A variable-number of additional arguments
 which correspond to what is needed
 by the format codes in szFmt.
 Returns:
 Returns a program exit return code:  the value of iexitRC.
 Notes:
 - Prints "ERROR: " followed by the formatted error message specified
 in szFmt.
 - Prints the file path and file name of the program having the error.
 This is the file that contains this routine.
 - Requires including <stdarg.h>
 **************************************************************************/
void ErrExit(int iexitRC, char szFmt[], ... )
{
    va_list args;               // This is the standard C variable argument list type
    va_start(args, szFmt);      // This tells the compiler where the variable arguments
    // begins.  They begin after szFmt.
    printf("ERROR: ");
    vprintf(szFmt, args);       // vprintf receives a printf format string and  a
    // va_list argument
    va_end(args);               // let the C environment know we are finished with the
    // va_list argument
    printf("\n\tEncountered in file %s\n", __FILE__);  // this 2nd arg is filled in by
    // the pre-compiler
    exit(iexitRC);
}
/******************** exitUsage *****************************
 void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo)
 Purpose:
 If this is an argument error (iArg >= 0), it prints a formatted message
 showing which argument was in error, the specified message, and
 supplemental diagnostic information.  It also shows the usage. It exits
 with ERR_COMMAND_LINE_SYNTAX.
 
 If this is just asking for usage (iArg will be -1), the usage is shown.
 It exits with USAGE_ONLY.
 Parameters:
 I int iArg                      command argument subscript
 I char *pszMessage              error message to print
 I char *pszDiagnosticInfo       supplemental diagnostic information
 Notes:
 This routine causes the program to exit.
 **************************************************************************/
void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo)
{
    if (iArg == USAGE_ONLY)
    {
        printf("command line arguents:\n -v \t Enable verbose mode.\n");
        exit(USAGE_ONLY);
    }
    if (iArg >= 0)
    {
        fprintf(stderr, "Error: bad argument #%d.  %s %s\n", iArg, pszMessage, pszDiagnosticInfo);
        printf("Valid arguments: -v, -?\n");
    }
    if (iArg >= 0)
        exit(ERR_COMMAND_LINE_SYNTAX);
    
}
