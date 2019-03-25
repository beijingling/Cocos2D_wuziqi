/*
 *  AICarbon.h
 *
 */

#ifndef _AICARBONHASH
#define _AICARBONHASH

#include "OXTypes.h"
#include "HashVal.h"
#include "AICarbonParameter.h"

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <algorithm>

class HashTable
{
private:
    struct HashRec
    {
        ULONG   hashB, hashC;
        short   value;
        short   depth;
#ifndef NDEBUG
        short   moves;
#endif
        OXPoint best;
    };

public:

    void clear()
    {
        hashASize = 0;
    }

    bool present()
    {
        return currentItem->hashB == hashB && currentItem->hashC == hashC;
    }

    void move(int x, int y, OXPiece who)
    {
        int offset = x + (y << 6) + (who << 11);
        hashA = (hashA + hashValA[offset]) % hashASize;
        currentItem = &elem[hashA];
        hashB = (hashB + hashValB[offset]);
        hashC = (hashC + hashValC[offset]);
    }

    void undo(int x, int y, OXPiece who)
    {
        int offset = x + (y << 6) + (who << 11);
        hashA = ((int)(hashA - hashValA[offset]) % (int)hashASize);
        if ((int)hashA < 0) hashA += hashASize;
        currentItem = &elem[hashA];
        hashB = (hashB - hashValB[offset]);
        hashC = (hashC - hashValC[offset]);
    }

    short value() { return currentItem->value; }
    short depth() { return currentItem->depth; }
#ifndef NDEBUG
    short moves() { return currentItem->moves; }
#endif
    OXPoint best() { return currentItem->best; }

    void update(short _value, short _depth, short _moves, OXPoint _best)
    {
        HashRec* c = currentItem;
        c->value = _value;
        c->depth = _depth;
#ifndef NDEBUG
        c->moves = _moves;
#endif
        c->hashB = hashB;
        c->hashC = hashC;
        c->best = _best;
    }

    void resize(ULONG size)
    {
        if (size > hashASize)
        {
            ULONG maxBytes = info_max_memory;
            if (maxBytes == 0) {
                maxBytes = 1000000000; // 1GB
            }
            maxBytes = std::max(maxBytes, (unsigned long)7500000) - 7000000;
            ULONG num = maxBytes / sizeof(HashRec);

            if (maxSize * 2 < num || maxSize > num) {
                elem = (HashRec*)realloc(elem, maxBytes);
                maxSize = num;
            }
            if (hashASize < maxSize) {
                hashASize = std::min(size * 2, maxSize);
                memset(elem, 0, hashASize * sizeof(HashRec));
            }
        }
    }

    HashTable()
    {
        maxSize = 0;
        hashASize = 0;
        elem = 0;

#ifndef NDEBUG
        for (int n = 0; n < 4096; n++)
            assert(hashValA[n] < 2000000000);
#endif
    }

    ~HashTable()
    {
        if (elem)
            free(elem);
    }

private:
    HashRec* currentItem;
    ULONG hashASize, maxSize;
    ULONG hashA, hashB, hashC;
    HashRec* elem;
};

#endif
