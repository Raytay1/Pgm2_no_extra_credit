/**********************************************************************
    cs3743p2.h
    Copyright 2019 Larry Clark, this code may not be copied to any other website
Purpose:
    Defines constants for
        return codes
        maximum record size
    Defines typedefs for
        HashHeader - describes the format of the first record in the hash file
        Vehicle - describes the contents of a Vehicle record
    Prototypes
        functions defined in the driver
        functions the students must code
Notes:
    
**********************************************************************/

#include <stdio.h>
#define TRUE                   1
#define FALSE                  0
#define RC_OK                  0
#define RC_FILE_EXISTS         1   // file already exists
#define RC_REC_EXISTS          2   // record already exists
#define RC_REC_NOT_FOUND       3   // record not found
#define RC_FILE_NOT_FOUND      4   // file not found
#define RC_HEADER_NOT_FOUND    5   // Header record not found
#define RC_BAD_REC_SIZE        6   // invalid record size in info record
#define RC_LOC_NOT_FOUND       7   // Location not found for read
#define RC_LOC_NOT_WRITTEN     8   // Location not written for write
#define RC_SYNONYM             9   // Synonym exists at the specified location
#define RC_NOT_IMPLEMENTED     10  // Not implemented
#define RC_TOO_MANY_COLLISIONS 11  // probing found too many collisions
#define RC_INVALID_PARM        99

#define MAX_REC_SIZE           500

// HashHeader structure for record at RBN 0. 
// Notes:
//     - do NOT write the sizeof this record.  Instead,
//       use the iRecSize.
//     - it contains filler on the end to make certain
//     - it is as big as a hash record.
typedef struct
{
    int  iMaxHash;          // Number of records defined for Primary 
                            // area.  (This isn't the number of
                            // records currently in the Primary Area.)
    int  iRecSize;          // Size in bytes for each record in the
                            // hash file
    int iMaxProbe;          // Max number of probes for a collision
    char sbFiller[MAX_REC_SIZE]; // This should be set to zeros.  It is 
                            // here to make certain the Info (header) record is 
                            // at least as big as the data.  When writing,
                            // use iRecsize not sizeof(HashHeader)
} HashHeader;
// HashFile structure containing a FILE pointer and HashHeader
// record for the hash file.
// Notes:
//     - 
typedef struct
{
    FILE *pFile;            // FILE pointer to the hash file
    HashHeader hashHeader;  // the header record contents for a hash file
} HashFile;

// Vehicle is the structure that contains Vehicle data.
typedef struct 
{
    char szVehicleId[7];     // Individual Vehicle Id 
    char szMake[13];         // Vheicle Make (e.g., FORD, TOYOTA)
    char szModel[12];        // Vehicle Model (e.g., EXPLORER, 4RUNNER, PRIUS)
    int  iYear;              // year this particular vehicle was manufactured
} Vehicle;

// Driver functions that the student can use
int printAll(char szFileNm[]);
int hash(char szKey[], int iMaxHash);
void printVehicle(Vehicle *pVehicle, int iMaxHash);
void errExit(const char szFmt[], ... );


// Functions written by the student
int hashCreate(char szFileNm[], HashHeader *pHashHeader);
int hashOpen(char szFileNm[], HashFile *pHashFile);
int readRec(HashFile *pHashFile, int iRBN, void *pRecord);
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord);
int vehicleInsert(HashFile *pHashFile, Vehicle *pVehicle);
int vehicleRead(HashFile *pHashFile, Vehicle *pVehicle, int *piRBN);
int vehicleUpdate(HashFile *pHashFile, Vehicle *pVehicle);
int vehicleDelete(HashFile *pHashFile, char *pszVehicleId); 
