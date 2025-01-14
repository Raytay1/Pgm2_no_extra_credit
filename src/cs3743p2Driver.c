/**********************************************************************
    cs3743p2Driver.c by Larry Clark
    Copyright 2018 Larry Clark, this code may not be copied to any 
    other website.
Purpose:
    This program demonstrates a Hash File which uses probing.
    It uses a stream input command file to test the various 
    functions written by the students.  The command file comes from stdin.
Command Parameters:
    p2 < infile > outfile
    The stream input file contains commands.  There are commands 
    which specify create, insert, read, delete, update, and printall.
   
    Commands in the input file:
    
    * comment       
        This is just a comment which helps explain what is being tested.
    
    CREATE VEHICLE fileName maxHash maxProbes  
        Create the specified hash file if it doesn't already exist
        using hashCreate.
        The file will have a max of maxHash in the hash file.
        Record 0 contains information about this hash file.
        The size of each record is the size of a Vehicle structure.   
    OPEN VEHICLE fileName 
        Opens the specified file using hashOpen.  Place the returned
        FILE pointer in the driver's HashFile array.
    INSERT VEHICLE vehicleId,make,model,year
        Uses sscanf to get those attributes and populate a Vehicle structure.
        It invokes VehicleInsert to insert the Vehicle into the hash file.
    READ VEHICLE VehicleId
        Invokes VehicleRead to read the Vehicle from the hash file and prints
        that Vehicle.
    PRINTALL VEHICLE fileName
        Prints the contents of the specified file.
    NUKE VEHICLE fileName
        Removes the specified file.
    UPDATE VEHICLE VehicleId,make,model,year
        Uses sscanf to get those attributes and populate a Vehicle structure.
        It invokes VehicleUpdate to update the Vehicle in the hash file.
    DELETE VEHICLE VehicleId
        Invokes VehicleDelete to delete the Vehicle in the hash file.  This will leave
        an empty entry, affecting most other functions. This is for extra credit.
Results:
    All output is written to stdout.
    Prints each of the commands and their command parameters.  Some of the commands
    print information:  
        CREATE - prints the record size
        INSERT - prints the hashed RBN (record block number)
        READ   - prints (if found):
                 -- Actual RBN
                 -- the Vehicle information
                 -- original hash RBN
        PRINTALL- prints the file's contents
Returns:
    0       Normal
    99       Processing Error

Notes:
   
**********************************************************************/
// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "cs3743p2.h"

#define MAX_TOKEN_SIZE 50		// largest token size for tokenizer
#define MAX_BUFFER_SZ 200       // size of input buffer for command file
#define ERROR_PROCESSING 99

// prototypes only used by the driver
void processCommands(FILE *pfileCommand);
int getSimpleToken(char szInput[], const char szDelims[]
    , int *piBufferPosition, char szToken[]);
void printRC(int rc);

int main(int argc, char *argv[])
{
    // Process commands for manipulating hash files
    processCommands(stdin);
    printf("\n");
    return 0;
}

/******************** processCommands **************************************
    void processCommands(FILE *pfileCommand)
Purpose:
    Reads the Command file to process commands.  There are several types of
    records (see the program header for more information).
Parameters:
    I FILE *pfileCommand    command stream input
Notes:
    This calls functions inside:
        cs3723p2 
**************************************************************************/
void processCommands(FILE *pfileCommand)
{
    // variables for command processing
    char szInputBuffer[MAX_BUFFER_SZ+1];    // input buffer for a single text line
    char szCommand[MAX_TOKEN_SIZE + 1];     // command
    int bGotToken;                          // TRUE if getSimpleToken got a token
    int iBufferPosition;                    // This is used by getSimpleToken to 
                                            // track parsing position within input buffer
    int iScanfCnt;                          // number of values returned by sscanf
    int rc;                                 // return code from functions
    Vehicle vehicle;                        // Vehicle structure used by functions
    HashFile hashFile;                      // hash file 
    char szHashContent[11];                 // content type for hash file (e.g., Vehicle)
    char szFileNm[31];                      // Hash File Nm
    int iRBN;                               // RBN of a read Vehicle

    // Initialize the hash File structure with null information.
    hashFile.pFile = NULL;
    memset(&hashFile.hashHeader, 0, sizeof(HashHeader));

    //  get command data until EOF
    while (fgets(szInputBuffer, MAX_BUFFER_SZ, pfileCommand) != NULL)
    {
        // if the line is just a line feed, ignore it
        if (szInputBuffer[0] == '\n')
            continue;

        // get the command
        iBufferPosition = 0;                // reset buffer position
        bGotToken = getSimpleToken(szInputBuffer, " \n\r", &iBufferPosition, szCommand);
        if (! bGotToken)
            errExit("Invalid stream input record:\n%s", szInputBuffer);

        // see if the command is a comment
        if (szCommand[0]== '*')
        {
            printf("%s", szInputBuffer);
            continue;       // it was just a comment
        }
        printf(" >> %s", szInputBuffer);

        // Process each type of input command
        if (strcmp(szCommand, "CREATE") == 0)
        { // CREATE VEHICLE fileName maxHash maxProbes 
            memset(hashFile.hashHeader.sbFiller, 0
                , sizeof(hashFile.hashHeader.sbFiller));
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %30s %d %d"
                , szHashContent
                , szFileNm
                , &hashFile.hashHeader.iMaxHash
                , &hashFile.hashHeader.iMaxProbe
                );
            if (iScanfCnt <  4)
                errExit("Invalid input:\n%s", szInputBuffer);
            hashFile.hashHeader.iRecSize = sizeof(Vehicle);
            printf("    Record size is %d\n", hashFile.hashHeader.iRecSize);
            rc = hashCreate(szFileNm, &hashFile.hashHeader);
            if (rc != 0)
                printRC(rc);
        }
        else if (strcmp(szCommand, "OPEN") == 0)
        { // OPEN VEHICLE fileName 
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %30s"
                , szHashContent
                , szFileNm);
            if (iScanfCnt < 2)
                errExit("Invalid input:\n%s", szInputBuffer);
            hashFile.hashHeader.iRecSize = sizeof(Vehicle);
            rc = hashOpen(szFileNm, &hashFile);
            if (rc != 0)
                printRC(rc);
        }
        else if (strcmp(szCommand, "INSERT") == 0)
        { // INSERT VEHICLE VehicleId,make,model,year
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %6[^,],%12[^,],%11[^,],%d"
                , szHashContent
                , vehicle.szVehicleId
                , vehicle.szMake
                , vehicle.szModel
                , &vehicle.iYear
                );
            if (iScanfCnt <  5)
                errExit("Invalid input:\n%s", szInputBuffer);
            printf("            Hash RBN is %d\n", hash(vehicle.szVehicleId
                , hashFile.hashHeader.iMaxHash));
            rc = vehicleInsert(&hashFile, &vehicle);
            if (rc != 0)
                printRC(rc);
        }
        else if (strcmp(szCommand, "READ") == 0)
        { // READ VEHICLE vehicleId
            memset(&vehicle, 0, sizeof(vehicle));
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %6s"
                , szHashContent
                , vehicle.szVehicleId);
            if (iScanfCnt <  2)
                errExit("Invalid input:\n%s", szInputBuffer);
            printf("            Hash RBN is %d\n", hash(vehicle.szVehicleId
                , hashFile.hashHeader.iMaxHash));
            rc = vehicleRead(&hashFile,&vehicle, &iRBN);
            if (rc != 0)
                printRC(rc);
            else
            {
                printf("    %2d", iRBN);  //indent like the PRINTALL command
                printVehicle(&vehicle, hashFile.hashHeader.iMaxHash);
            }
        }

        else if (strcmp(szCommand, "UPDATE") == 0)
        { // UPDATE VEHICLE vehicleId,make,model,year
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %6[^,],%12[^,],%11[^,],%d"
                , szHashContent
                , vehicle.szVehicleId
                , vehicle.szMake
                , vehicle.szModel
                , &vehicle.iYear
                );
            if (iScanfCnt <  5)
                errExit("Invalid input:\n%s", szInputBuffer);
            printf("            Hash RBN is %d\n", hash(vehicle.szVehicleId
                , hashFile.hashHeader.iMaxHash));
            rc = vehicleUpdate(&hashFile,&vehicle);
            if (rc != 0)
                printRC(rc);
        }
        
        else if (strcmp(szCommand, "DELETE") == 0)
        { // DELETE VEHICLE vehicleId
            memset(&vehicle, 0, sizeof(vehicle));
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %11s"
                , szHashContent
                , vehicle.szVehicleId);
            if (iScanfCnt <  2)
                errExit("Invalid input:\n%s", szInputBuffer);
            rc = vehicleDelete(&hashFile,vehicle.szVehicleId);
            if (rc != 0)
                printRC(rc);
        }
        else if (strcmp(szCommand, "PRINTALL") == 0)
        { // PRINTALL VEHICLE fileName
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %30s"
                , szHashContent
                , szFileNm);
            if (iScanfCnt < 2)
                errExit("Invalid input:\n%s", szInputBuffer);
            // Flush the Hash File so that printAll can see it
            fflush(hashFile.pFile);
            rc = printAll(szFileNm);
            if (rc != 0)
                printRC(rc);
        }
        else if (strcmp(szCommand, "NUKE") == 0)
        { // NUKE VEHICLE fileName
            iScanfCnt = sscanf(&szInputBuffer[iBufferPosition]
                , "%10s %30s"
                , szHashContent
                , szFileNm);
            if (iScanfCnt < 2)
                errExit("Invalid input:\n%s", szInputBuffer);
            rc = remove(szFileNm);   // Ignore the rc
        }
        else if (strcmp(szCommand, "HALT") == 0)
        { // HALT
            break;
        }
        else
            printf("   *** Unknown command in input: '%s'\n", szCommand);
    }
}
/******************** hash **************************************
  int hash(char szKey[],int iMaxHash)
Purpose:
    Hashes a key to return an RBN between 1 and iMaxHash 
    (inclusive).
Parameters:
    I   char szKey[]           key to be hashed
    I   iMaxHash               number of primary records
Returns:
    Returns a hash value into the primary area.  This value is between 1
    and iMaxHash
Notes:
    - hash area of the hash table has subscripts from 1 to 
      iMaxHash.
    - The hash function is average at spreading the values.  
**************************************************************************/
int hash(char szKey[],int iMaxHash)
{
    int iHash = 0;
    int i;
    if (iMaxHash <= 0)
        errExit("hash function received an invalid iMaxHash value: %d\n"
            , iMaxHash);
    for (i = 0; i < (int) strlen(szKey); i++)
    {
        iHash += szKey[i];
    }
    // restrict it to the hash area
    iHash = abs(iHash) % iMaxHash +1; 
    return iHash;
}

/******************** printAll **************************************
  int printAll(char szFileNm[]))
Purpose:
    Prints the contents of the specified hash file. 
Parameters:
    I   char szFileNm[]               hash file name
Returns:
    RC_OK - normal
    RC_FILE_NOT_FOUND - file not found
    RC_HEADER_NOT_FOUND - header record not found
    RC_BAD_REC_SIZE - bad record size
Notes:
    - hash area of the hash table has subscripts from 1 to 
      pHashHeader.iMaxHash.
**************************************************************************/
int printAll(char szFileNm[])
{
    // variables used for the buffer passed to hexdump 
    int rc;                                 // return code
    HashFile hashFile;                      // hash file structure containing
                                            // a FILE pointer and header.

    struct Record
    {
        int iNextChain;
        char sbBuffer[MAX_REC_SIZE];
    } record;

    long lRBA;
    long lRecNum;
    int rcFseek;
    // open the hash file
    memset(&hashFile.hashHeader, 0, sizeof(HashHeader));
    hashFile.hashHeader.iRecSize = sizeof(Vehicle);
    rc = hashOpen(szFileNm, &hashFile);
    if (rc != 0)
        return rc;

    // print the Hash Header
    printf("    MaxHash=%d, RecSize=%d, MaxProbes=%d\n"
        , hashFile.hashHeader.iMaxHash
        , hashFile.hashHeader.iRecSize
        , hashFile.hashHeader.iMaxProbe);
    if (hashFile.hashHeader.iRecSize < sizeof(Vehicle))
        return RC_BAD_REC_SIZE;

    // locate the first record
    lRecNum = 1;
    lRBA = lRecNum*hashFile.hashHeader.iRecSize;
    rcFseek = fseek(hashFile.pFile, lRBA, SEEK_SET);
    assert(rcFseek == 0);
    while(fread(&record, hashFile.hashHeader.iRecSize
        , 1L, hashFile.pFile)==1)
    {
        if (record.sbBuffer[0] != '\0')
        {
            printf("    %2ld", lRecNum);
            printVehicle((Vehicle *) &record, hashFile.hashHeader.iMaxHash);
        }
        lRecNum += 1;
    }
    fclose(hashFile.pFile);
    return RC_OK;
}
/******************** printVehicle **************************************
    void printVehicle(Vehicle *pVehicle, int iMaxHash)
Purpose:
    Prints the Vehicle information for one Vehicle and its original
    hash value.
Parameters:
    I Vehicle *pVehicle       // Vehicle info
    I int iMaxHash            // maximum hash value
**************************************************************************/
void printVehicle(Vehicle *pVehicle, int iMaxHash)
{
    printf(" %6s %4d %-12s %11s Hash=%d\n"
        , pVehicle->szVehicleId
        , pVehicle->iYear
        , pVehicle->szMake
        , pVehicle->szModel
        , hash(pVehicle->szVehicleId, iMaxHash)
        );
}


/******************** printRC **************************************
    void printRC(int rc)
Purpose:
    For a non-zero RC, it prints an appropriate message
Parameters:
    I int rc;           // return code value which is an error if
                        // non-zero

**************************************************************************/
void printRC(int rc)
{
    char *pszMsg;           // pointer to an error message
    char szBuf[100];        // buffer for building an error message
    switch(rc)
    {
        case RC_OK:
            return;
        case RC_FILE_EXISTS:
            pszMsg = "file already exists";
            break;
        case RC_REC_EXISTS:
            pszMsg = "record already exists";
            break;
        case RC_REC_NOT_FOUND:
            pszMsg = "record not found";
            break;
        case RC_INVALID_PARM:
            pszMsg = "invalid parameter";
            break;
        case RC_FILE_NOT_FOUND:
            pszMsg = "file not found or invalid header record";
            break;
        case RC_HEADER_NOT_FOUND:
            pszMsg = "header record not found";
            break;
        case RC_BAD_REC_SIZE:
            pszMsg = "invalid record size in header recordd";
            break;
        case RC_LOC_NOT_FOUND:
            pszMsg = "Location not found for read";
            break;
        case RC_LOC_NOT_WRITTEN:
            pszMsg = "Location not written for write";
            break;
        case RC_SYNONYM:
            pszMsg = "Synonym exists at the specified location";
            break;
        case RC_NOT_IMPLEMENTED:
            pszMsg = "Function not implemented";
            break;
        case RC_TOO_MANY_COLLISIONS:
            pszMsg = "Too Many Collisions to write record";
            break;
        default:
            sprintf(szBuf, "unknown return code: %d", rc);
            pszMsg = szBuf;
    }
    printf("    **** ERROR: %s\n", pszMsg);
}
/******************** getSimpleToken **************************************
 int getSimpleToken(char szInput[], char szDelims[]
     , int *piBufferPosition, char szToken[]) 
Purpose:
    Returns the next token in a string.  The delimiter(s) for the token is  
    passed as a parameter.  To help positioning for a subsequent call, it 
    also returns the next position in the buffer.
Parameters:
    I   char szInput[]          input buffer to be parsed
    I   char szDelims[]         delimiters for tokens
    I/O int *piBufferPosition   Position to begin looking for the next token.
                                This is also used to return the next 
                                position to look for a token (which will
                                follow the delimiter).
    O   char szToken[]          Returned token.  
Returns:
    Functionally:
        TRUE - a token is returned
        FALSE - no more tokens
    iBufferPosition parm - the next position for parsing
    szToken parm - the returned token.  If not found, it will be an empty string.
Notes:
    - If the token is larger than MAX_TOKEN_SIZE, we return a truncated value.
**************************************************************************/

int getSimpleToken(char szInput[], const char szDelims[]
    , int *piBufferPosition, char szToken[]) 
{
    int iDelimPos;                      // found position of delim
    int iCopy;                          // number of characters to copy
    
    // check for past the end of the string
    if (*piBufferPosition >= (int) strlen(szInput))
    {
        szToken[0] = '\0';              // mark it empty
        return FALSE;                   // no more tokens
    }

    // get the position of the first delim, relative to the iBufferPosition
    iDelimPos = strcspn(&szInput[*piBufferPosition], szDelims);

    // see if we have more characters than target token, if so, trunc
    if (iDelimPos > MAX_TOKEN_SIZE)
        iCopy = MAX_TOKEN_SIZE;             // truncated size
    else
        iCopy = iDelimPos;
    
    // copy the token into the target token variable
    memcpy(szToken, &szInput[*piBufferPosition], iCopy);
    szToken[iCopy] = '\0';              // null terminate

    // advance the position
    *piBufferPosition += iDelimPos + 1;  
    return TRUE;
}

/******************** errExit **************************************
  void errExit(const char szFmt[], ... )
Purpose:
    Prints an error message defined by the printf-like szFmt and the
    corresponding arguments to that function.  The number of 
    arguments after szFmt varies dependent on the format codes in
    szFmt.  
    It also exits the program.
Parameters:
    I   const char szFmt[]      This contains the message to be printed
                                and format codes (just like printf) for 
                                values that we want to print.
    I   ...                     A variable-number of additional arguments
                                which correspond to what is needed
                                by the format codes in szFmt. 
Returns:
    Exits the program with a return code of ERROR_PROCESSING (99).
Notes:
    - Prints "ERROR: " followed by the formatted error message specified 
      in szFmt. 
    - Requires including <stdarg.h>
**************************************************************************/
void errExit(const char szFmt[], ... )
{
    va_list args;               // This is the standard C variable argument list type
    va_start(args, szFmt);      // This tells the compiler where the variable arguments
                                // begins.  They begin after szFmt.
    printf("ERROR: ");
    vprintf(szFmt, args);       // vprintf receives a printf format string and  a
                                // va_list argument
    va_end(args);               // let the C environment know we are finished with the
                                // va_list argument
    printf("\n"); 
    exit(ERROR_PROCESSING);
}
