/********************************************************
 cs3743p1.c by Raynard Taylor UKJ147

 Purpose:
    Provides the rest of the functions needed to make
    the driver program run.
 ********************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cs3743p2.h"

/******************** hashCreate **********************************
    int hashCreate(char szFileNm[], HashHeader *pHashHeader)
Purpose:
    This function creates a hash file containing only the HashHeader
    record.
Parameters:
    I char szFileNm[]           hash file name
    I HashHeader *pHashHeader   HashHeader to be written to file
Return value:
    RC_FILE_EXISTS - file already exists
    RC_LOC_NOT_WRITTEN - Location not written for write
    RC_OK - normal
**************************************************************************/
int hashCreate(char szFileNm[], HashHeader *pHashHeader)
{
    FILE *pFile;
    pFile = fopen(szFileNm, "rb");
    if(pFile != NULL)
    {
        return RC_FILE_EXISTS;      // File existed
    }

    pFile = fopen(szFileNm, "wb");
    long rc, lRBA;
    int rcFseek;

    lRBA = 0 * pHashHeader->iRecSize;           // determines the offset
    rcFseek = fseek(pFile, lRBA, SEEK_SET);     // fseek to the position in the file
    assert(rcFseek == 0);

    rc = fwrite(&(*pHashHeader), pHashHeader->iRecSize, 1L, pFile);
    if(rc == 0)
    {
        return RC_LOC_NOT_WRITTEN;
    }

    fclose(pFile);
    return RC_OK;
}

/******************** hashOpen **********************************
    int hashOpen(char szFileNm[], HashFile *pHashFile)
Purpose:
    This function opens an existing hash file which must contain a
    HashHeader record, and sets the pHashFile->pFile.
    It returns the HashHeader record by setting pHashFile->hashHeader
Parameters:
    I char szFileNm[]           hash file name
    I HashFile *pHashFile       contains HashHeader and File pointer
Return value:
    RC_FILE_NOT_FOUND - file not found
    RC_HEADER_NOT_FOUND - Header record not found
    RC_OK - normal
**************************************************************************/
int hashOpen(char szFileNm[], HashFile *pHashFile)
{
    long rc;

    pHashFile->pFile = fopen(szFileNm, "rb+");
    if(pHashFile->pFile == NULL)
    {
        return RC_FILE_NOT_FOUND;      // File was not found
    }

    rc = fread(&pHashFile->hashHeader, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile);
    if(rc == 0){
        return RC_HEADER_NOT_FOUND;
    }else
    {
        return RC_OK;
    }
}

/******************** readRec ********************************************
    int readRec(HashFile *pHashFile, int iRBN, void *pRecord)
Purpose:
    This function reads a record at the specified RBN in the
    specified file. Returns if the location couldn't be found
    or if was found.
Parameters:
    I HashFile *pHashFile       contains HashHeader and File pointer
    I int iRBN                  RBN to be looked for in file
    O void *pRecord             Vehicle info
Return value:
    RC_LOC_NOT_FOUND - Location not found for read
    RC_OK - normal
**************************************************************************/
int readRec(HashFile *pHashFile, int iRBN, void *pRecord)
{
    Vehicle tmp;

    int rcFseek, RecSize;
    long lRBA, rc;

    RecSize = pHashFile->hashHeader.iRecSize;               // size for one Record
    lRBA = iRBN * RecSize;                                  // determines the offset
    rcFseek = fseek(pHashFile->pFile, lRBA, SEEK_SET);      // fseek to the position in the file
    assert(rcFseek == 0);


    rc = fread(&tmp ,RecSize, 1L, pHashFile->pFile);
    *(Vehicle*) pRecord = tmp;
    if(rc != 1 || tmp.szVehicleId[0] == '\0'){
        return RC_LOC_NOT_FOUND;
    }else
    {
        return RC_OK;
    }

}

/******************** writeRec *******************************************
    int writeRec(HashFile *pHashFile, int iRBN, void *pRecord)
Purpose:
    This function writes a record to the specified RBN in the
    specified file and returns if the write failed or if it
    was successful.
Parameters:
    I HashFile *pHashFile       contains HashHeader and File pointer
    I int iRBN                  RBN to be written to in file
    I void *pRecord             Vehicle info
Return value:
    RC_LOC_NOT_WRITTEN - Location not written for write
    RC_OK - normal
**************************************************************************/
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord)
{
    int rcFseek, RecSize;
    long lRBA, rc;
    Vehicle tmp = *(Vehicle*) pRecord;

    RecSize = pHashFile->hashHeader.iRecSize;               // size for one Record
    lRBA = iRBN * RecSize;                                  // determines the offset
    rcFseek = fseek(pHashFile->pFile, lRBA, SEEK_SET);      // fseek to the position in the file
    assert(rcFseek == 0);

    rc = fwrite(&tmp, RecSize, 1L, pHashFile->pFile);
    if(rc == 0)
        {
        return RC_LOC_NOT_WRITTEN;
    }else
    {
        return RC_OK;
    }
}

/******************** vehicleInsert **********************************
    int vehicleInsert(HashFile *pHashFile, Vehicle *pVehicle)
Purpose:
    This function inserts a vehicle into the specified file and
    returns if the record already exist or if it's a synonym.
Parameters:
    I HashFile *pHashFile       contains HashHeader and File pointer
    I Vehicle *pVehicle         Vehicle info
Return value:
    RC_REC_EXISTS - record already exists
    RC_TOO_MANY_COLLISIONS - probing found too many collisions
**************************************************************************/
int vehicleInsert(HashFile *pHashFile, Vehicle *pVehicle)
{
    int i, rc, RBN;
    Vehicle rVehicle;

    RBN = hash(pVehicle->szVehicleId, pHashFile->hashHeader.iMaxHash);
    rc = readRec(&(*pHashFile), RBN, &rVehicle);
    if(rc == RC_LOC_NOT_FOUND || rVehicle.szVehicleId[0] == '\0')
    {
        writeRec(&(*pHashFile), RBN, &(*pVehicle));
    }
    else if(rc == RC_OK && (strcmp(pVehicle->szVehicleId, rVehicle.szVehicleId)==0))
    {
        return RC_REC_EXISTS;
    }else
    {
        //synonym code goes here
        int nRBN = -1;  // RBN for inserting synonym
                        // if still -1 when loop ends then no open spots
        for(i=1;i<pHashFile->hashHeader.iMaxProbe;i++)
        {
            rc = readRec(&(*pHashFile), RBN+i, &rVehicle);
            if(rVehicle.szVehicleId[0] == '\0')
            {
               if(nRBN == -1)
               {
                  nRBN = RBN+i;
               }
            }else if(rc == RC_OK && strcmp(pVehicle->szVehicleId, rVehicle.szVehicleId)==0)
            {
                return RC_REC_EXISTS;
            }
        }
        if(nRBN != -1)
        {
            writeRec(&(*pHashFile), nRBN, &(*pVehicle));
        }else
        {
            return RC_TOO_MANY_COLLISIONS;
        }

    }
}

/******************** vehicleRead **********************************
    int vehicleRead(HashFile *pHashFile, Vehicle *pVehicle, int *piRBN)
Purpose:
    This function reads the specified vehicle by its szVehicleId.
    Returns if RC_OK if vehicle is found or RC_REC_NOT_FOUND if
    not found
Parameters:
    I HashFile *pHashFile       contains HashHeader and File pointer
    I/O Vehicle *pVehicle       Vehicle info
    O int *piRBN                result RBN
Return value:
    RC_OK - normal
    RC_REC_NOT_FOUND - record not found
**************************************************************************/
int vehicleRead(HashFile *pHashFile, Vehicle *pVehicle, int *piRBN)
{
    int i, RBN;
    Vehicle rVehicle;

    RBN = hash(pVehicle->szVehicleId, pHashFile->hashHeader.iMaxHash);
    readRec(&(*pHashFile), RBN, &rVehicle);
    if(strcmp(pVehicle->szVehicleId, rVehicle.szVehicleId) == 0)
    {
        *pVehicle = rVehicle;
        *piRBN = RBN;
        return RC_OK;
    }else
    {
        //synonym code goes here
        for(i=1;i<pHashFile->hashHeader.iMaxProbe;i++)
        {
            if(RBN+i > pHashFile->hashHeader.iMaxHash)
            {
                return RC_REC_NOT_FOUND; // went past max amount of records
            }

            readRec(&(*pHashFile), RBN+i, &rVehicle);

            if(strcmp(pVehicle->szVehicleId, rVehicle.szVehicleId) == 0)
            {
                *pVehicle = rVehicle;
                *piRBN = RBN+i;
                return RC_OK;
            }

        }
        // if loop ends max probes reached
        return RC_REC_NOT_FOUND;
    }
}

/******************** vehicleUpdate **********************************
    int vehicleUpdate(HashFile *pHashFile, Vehicle *pVehicle)
Purpose:
    This function reads the specified vehicle by its szVehicleId.
    If found, it updates the contents of the vehicle in the hash file.
    If not found, it returns RC_REC_NOT_FOUND.
Parameters:
    I HashFile *pHashFile       contains HashHeader and File pointer
    I Vehicle *pVehicle       Vehicle info
Return value:
    RC_OK - normal
    RC_REC_NOT_FOUND - record not found
**************************************************************************/
int vehicleUpdate(HashFile *pHashFile, Vehicle *pVehicle)
{
    int i, RBN;
    Vehicle rVehicle;

    RBN = hash(pVehicle->szVehicleId, pHashFile->hashHeader.iMaxHash);

    for(i=0;i<pHashFile->hashHeader.iMaxProbe;i++)
    {
        if(RBN+i > pHashFile->hashHeader.iMaxHash)
        {
            return RC_REC_NOT_FOUND; // went past max amount of records
        }

        readRec(&(*pHashFile), RBN+i, &rVehicle);

        if(strcmp(pVehicle->szVehicleId, rVehicle.szVehicleId) == 0)
        {
            writeRec(&(*pHashFile), RBN+i, &(*pVehicle));
            return RC_OK;
        }

    }
    // if loop ends max probes reached
    return RC_REC_NOT_FOUND;

}

/******************** vehicleDelete **********************************
    int vehicleDelete(HashFile *pHashFile, char *pszVehicleId)
Purpose:
    This function finds the specified vehicle and deletes it by simply
    setting all bytes in that record to '\0'.
Parameters:
    I HashFile *pHashFile       contains HashHeader and File pointer
    I char *pszVehicleId        Vehicle id
Return value:

**************************************************************************/
int vehicleDelete(HashFile *pHashFile, char *pszVehicleId)
{
    // extra credit
    int i, RBN;
    Vehicle rVehicle, zVehicle;
    memset(&zVehicle, 0, sizeof(Vehicle));

    RBN = hash(pszVehicleId, pHashFile->hashHeader.iMaxHash);

    for(i=0;i<pHashFile->hashHeader.iMaxProbe;i++)
    {
        if(RBN+i > pHashFile->hashHeader.iMaxHash)
        {
            return RC_REC_NOT_FOUND; // went past max amount of records
        }

        readRec(&(*pHashFile), RBN+i, &rVehicle);

        if(strcmp(pszVehicleId, rVehicle.szVehicleId) == 0)
        {
            writeRec(&(*pHashFile), RBN+i, &zVehicle);
            return RC_OK;
        }

    }
    // if loop ends max probes reached
    return RC_REC_NOT_FOUND;

}