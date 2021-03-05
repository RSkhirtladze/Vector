#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(v != NULL && elemSize > 0 && initialAllocation > 0);

    v->logLen = 0;
    v->allocLen = initialAllocation;
    v->reallocSize = initialAllocation;
    v->elemSize = elemSize;
    v->freeFn = freeFn;

    v->elems = malloc(initialAllocation * elemSize);
    assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
    assert(v != NULL);

    if (v->freeFn == NULL){}
    else{
        for (int iter = 0; iter < v->logLen; iter++){
            v->freeFn(VectorNth(v, iter));
        }
    }

    free(v->elems);
}

int VectorLength(const vector *v)
{ 
    assert(v != NULL);
    return v->logLen;
}

void *VectorNth(const vector *v, int position)
{
    assert(v != NULL && position >= 0 && position < v->logLen);
    return (char *)(v->elems) + (position * v->elemSize);
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(v != NULL && position >= 0 && position < v->logLen);
    
    void *currElem = (char *)(v->elems) + (position * v->elemSize);

    assert(currElem != NULL);
    
    if (v->freeFn != NULL){
       v->freeFn(currElem);
    }

    memmove(currElem, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position <= v->logLen);
    
    if(v->logLen == v->allocLen) {
        v->allocLen += v->reallocSize;
        v->elems = realloc(v->elems, v->allocLen * v->elemSize);
        assert(v->elems != NULL);
    }

    memmove((char*)v->elems + v->elemSize*(position+1),(char*)v->elems + v->elemSize*position,v->elemSize*(v->logLen-position));
    memcpy((char *)v->elems + v->elemSize * position,elemAddr,v->elemSize);
    v->logLen++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    assert(v != NULL);

    if(v->logLen == v->allocLen) {
        v->allocLen += v->reallocSize;
        v->elems = realloc(v->elems, v->allocLen * v->elemSize);
        assert(v->elems != NULL);
    }

    v->logLen++;
    memmove(VectorNth(v, v->logLen - 1), elemAddr, v->elemSize);

}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0);
    assert(position <= v->logLen);

    if(v->freeFn != NULL) {
        v->freeFn((char *) v->elems + v->elemSize*position);
    }
    memmove((char*)v->elems + v->elemSize*position,(char*)v->elems + v->elemSize*(position+1),(v->logLen - position - 1)*v->elemSize);
    v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    qsort(v->elems, v->logLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int iter = 0; iter < v->logLen; iter++){
        mapFn(VectorNth(v, iter), auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
    assert(key != NULL && searchFn != NULL && startIndex >= 0 && startIndex <= v->logLen);
    if (v->logLen == 0) return kNotFound;

    void * found;

    void *startPos = (char *)(v->elems) + (startIndex * v->elemSize);
    size_t elemsNumToSearch = v->logLen - startIndex;

    if (isSorted){
        found = bsearch(key, startPos, elemsNumToSearch, v->elemSize, searchFn);
    }else{
        found = lfind(key, startPos, &elemsNumToSearch, v->elemSize, searchFn);
    }

    if (!found) {
        return kNotFound;
    }
    return ((char *)found - (char *)v->elems) / v->elemSize;
}