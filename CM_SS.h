#ifndef CM_SS_H
#define CM_SS_H

/*
 * On-Off sketch on finding persistent items
 */

#include "bitset.h"
#include "Abstract.h"
#include <stdlib.h>

template<typename DATA_TYPE, typename COUNT_TYPE, uint32_t SLOT_NUM>
class CM_SS : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Bucket {
        DATA_TYPE items[SLOT_NUM];
        COUNT_TYPE counters[SLOT_NUM];
        COUNT_TYPE stability[SLOT_NUM];

        inline COUNT_TYPE Query(const DATA_TYPE item){
            for(uint32_t i = 0;i < SLOT_NUM;++i){
                if(items[i] == item)
                    return counters[i];
            }
            return 0;
        }
    };

    CM_SS(uint64_t memory) :
            length((double)memory / (sizeof(Bucket)+ SLOT_NUM * BITSIZE)){
        buckets = new Bucket[length];

        memset(buckets, 0, length * sizeof(Bucket));
        bucketBitsets = new BitSet(SLOT_NUM * length);
    }

    ~CM_SS(){
        delete [] buckets;
        delete bucketBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){

        uint8_t res=0;
        uint32_t pos = this->hash(item) % length;
        uint32_t bucketBitPos = pos * SLOT_NUM;

        for(uint32_t i = 0;i < SLOT_NUM;++i){
            if(buckets[pos].items[i] == item){
                res=(!bucketBitsets->SetNGet(bucketBitPos + i));
                buckets[pos].counters[i] += res;
                buckets[pos].stability[i] += res;
                return;
            }
        }
        uint32_t minValue=0;
        for(uint32_t i = 1;i < SLOT_NUM;++i){
            if(buckets[pos].counters[i] < buckets[pos].counters[minValue]){
                minValue=i;
            }
        }
        
        uint32_t probability=buckets[pos].counters[minValue]+buckets[pos].stability[minValue]+1;
//        uint32_t probability = buckets[pos].counters[minValue]+ 1;
        if (rand()%(probability)==0){
            buckets[pos].items[minValue] = item;
            buckets[pos].counters[minValue] = 1;
            buckets[pos].stability[minValue] = 0;
            bucketBitsets->Set(bucketBitPos + minValue);
        }

        return;
        
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        return buckets[this->hash(item) % length].Query(item);
    }

    void NewWindow(const COUNT_TYPE window){
        bucketBitsets->Clear();
    }

private:
    const uint32_t length;
    BitSet* bucketBitsets;
    Bucket* buckets;
};

#endif
