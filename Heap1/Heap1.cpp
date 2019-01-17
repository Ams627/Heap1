#include "stdafx.h"
#include <crtdbg.h> // for _CrtMemBlockHeader
#include "Heap1.h"

#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
    struct _CrtMemBlockHeader * pBlockHeaderNext;
    struct _CrtMemBlockHeader * pBlockHeaderPrev;
    char *                      szFileName;
    int                         nLine;
#ifdef _WIN64
    /* These items are reversed on Win64 to eliminate gaps in the struct
    * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
    * maintained in the debug heap.
    */
    int                         nBlockUse;
    size_t                      nDataSize;
#else  /* _WIN64 */
    int                         nBlockUse;
    size_t                      nDataSize;
#endif  /* _WIN64 */
    long                        lRequest;
    unsigned char               gap[nNoMansLandSize];
    /* followed by:
    *  unsigned char           data[nDataSize];
    *  unsigned char           anotherGap[nNoMansLandSize];
    */
} _CrtMemBlockHeader;

void heapdump(void)
{
    _HEAPINFO hinfo;
    int heapstatus;
    int numLoops;
    hinfo._pentry = NULL;
    numLoops = 0;
    while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK &&
        numLoops < 100'000)
    {
        printf("%8s block at %08p of size %4.4X\n",
            (hinfo._useflag == _USEDENTRY ? "USED" : "FREE"),
            hinfo._pentry, hinfo._size);


        _CrtMemBlockHeader* block = reinterpret_cast<_CrtMemBlockHeader*>(hinfo._pentry);
        block = block;
        if (block->nBlockUse == _CRT_BLOCK || block->nBlockUse == _NORMAL_BLOCK)
        {
            if (block->nLine > 0 && block->nLine < 10000)
            {
                std::string name = block->szFileName;
                std::cout << "filename: " << name << std::endl;
                block = block;
            }
        }

        numLoops++;
    }

    switch (heapstatus)
    {
    case _HEAPEMPTY:
        printf("OK - empty heap\n");
        break;
    case _HEAPEND:
        printf("OK - end of heap\n");
        break;
    case _HEAPBADPTR:
        printf("ERROR - bad pointer to heap\n");
        break;
    case _HEAPBADBEGIN:
        printf("ERROR - bad start of heap\n");
        break;
    case _HEAPBADNODE:
        printf("ERROR - bad node in heap\n");
        break;
    }
}


// Memory block identification
#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

void PrintHeapFromMemstate(_CrtMemState *memstate)
{
    char *blockTypes[] = { "free", "normal", "crt", "ignore", "client" };

    for (int i = 0; i < _MAX_BLOCKS; i++)
    {
        printf("%s blocks: %d \n", blockTypes[i], memstate->lCounts[i]);
    }

    auto pHeader = memstate->pBlockHeader;
    while (pHeader)
    {
        printf("filename pointer %08p, line %u block type %u, size %u\n", pHeader->szFileName, pHeader->nLine, pHeader->nBlockUse, pHeader->nDataSize);
        printf("filename: %s", pHeader->szFileName);
        pHeader = pHeader->pBlockHeaderNext;
    }
}

int main()
{
    void *p2 = malloc(123457);

    _CrtMemState memstate;
    _CrtMemCheckpoint(&memstate);
    PrintHeapFromMemstate(&memstate);
}
