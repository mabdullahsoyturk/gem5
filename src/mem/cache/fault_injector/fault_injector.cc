/** @file
 * Definition of FaultInjector class functions.
 */

#include "mem/cache/fault_injector/fault_injector.hh"
#include "debug/Cache.hh"
#include "debug/FaultTrace.hh"

FaultInjector::FaultInjector(FaultInjectorParams *params) : 
    SimObject(params),inputPath(params->input_path)
{}

void 
FaultInjector::init(std::string owner) 
{
    std::ifstream ifs(inputPath);
    
    int set,byteOffset,bitOffset;
    std::string cacheToBeInserted;
    int numberOfFaults = 0;

    DPRINTF(FaultTrace, "\t%s faults:\n\n", owner);

    if(ifs.is_open()) {
        while(ifs >> set >> byteOffset >> bitOffset >> cacheToBeInserted){
            if((cacheToBeInserted.compare(owner)) == 0) {
                CacheFault fault;
                fault.set = set;
                fault.byteOffset = byteOffset;
                fault.bitOffset = bitOffset;
                fault.cacheToBeInserted = cacheToBeInserted;

                faults.push_back(fault);
                DPRINTF(FaultTrace, "Set: %#x, Byte Offset: %d, Bit Offset: %d\n", fault.set, fault.byteOffset, fault.bitOffset);

                numberOfFaults++;
            }
	    }

        ifs.close();   
    }

    DPRINTF(FaultTrace, "Number of faults in total: %d\n", numberOfFaults);
}

void 
FaultInjector::flipBit(CacheFault fault, CacheBlk* blk, unsigned blkSize)
{
    uint8_t* data = blk->data;
    
    uint8_t oldValue = data[fault.byteOffset];
    data[fault.byteOffset] &= ~(1UL << fault.bitOffset);
    DPRINTF(FaultTrace, "Set: %#x, Byte Offset: %d, Bit Offset: %d\n corrupted", fault.set, fault.byteOffset, fault.bitOffset);
    DPRINTF(FaultTrace, "Old value of byte %d : %d, New value of byte %d: %d\n", fault.byteOffset, oldValue, fault.byteOffset, data[fault.byteOffset]);
}

void 
FaultInjector::injectFaults(BaseTags* tags, unsigned blkSize, bool isRead, std::string cacheType) 
{
    for(std::vector<CacheFault>::iterator it = faults.begin(); it != faults.end(); ++it) {
        CacheBlk* blk = static_cast<CacheBlk*>(tags->findBlockBySetAndWay(it->set, 0));

        if(blk && blk->isValid() && it->cacheToBeInserted == cacheType) {
            flipBit(*it, blk, blkSize);
        }    
    }
}

FaultInjector* FaultInjectorParams::create()
{
    return new FaultInjector(this);
}
