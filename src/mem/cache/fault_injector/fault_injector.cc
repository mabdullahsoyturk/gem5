/** @file
 * Definition of FaultInjector class functions.
 */

#include "mem/cache/fault_injector/fault_injector.hh"
#include "debug/Cache.hh"
#include "debug/FaultTrace.hh"

FaultInjector *gFIptr;

FaultInjector::FaultInjector(FaultInjectorParams *params) :
    SimObject(params),inputPath(params->input_path),enabled(false), assoc(params->assoc)
{
}


void 
FaultInjector::init(std::string owner) 
{
    if(owner == "l1d"){
        gFIptr = this;
    }

    std::ifstream ifs(inputPath);
    
    int type,index,byteOffset,bitOffset;
    std::string cacheToBeInserted;
    int numberOfFaults = 0;

    DPRINTF(FaultTrace, "\t%s faults:\n\n", owner);

    if(ifs.is_open()) {
        while(ifs >> type >> index >> byteOffset >> bitOffset >> cacheToBeInserted){
            if((cacheToBeInserted.compare(owner)) == 0) {
                CacheFault fault;
                fault.type = type;

                // Calculate set and way from entry index
                const std::lldiv_t div_result = std::div((long long)index, assoc);
                const uint32_t set = div_result.quot;
                const uint32_t way = div_result.rem;

                fault.set = set;
                fault.way = way;
                fault.byteOffset = byteOffset;
                fault.bitOffset = bitOffset;
                fault.cacheToBeInserted = cacheToBeInserted;

                faults.push_back(fault);

                DPRINTF(FaultTrace, "Type: %d, Set: %d, Way: %d, Byte Offset: %d, Bit Offset: %d\n", fault.type, fault.set, fault.way, fault.byteOffset, fault.bitOffset);

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
    DPRINTF(FaultTrace, "Set: %#x, Way: %#x, Byte Offset: %d, Bit Offset: %d\n corrupted", fault.set, fault.way, fault.byteOffset, fault.bitOffset);
    DPRINTF(FaultTrace, "Old value of byte %d : %d, New value of byte %d: %d\n", fault.byteOffset, oldValue, fault.byteOffset, data[fault.byteOffset]);
}

void 
FaultInjector::injectFaults(BaseTags* tags, unsigned blkSize, bool isRead, std::string cacheType) 
{
    if (!enabled) {
        return;
    }
    
    DPRINTF(FaultTrace, "injectFaults method is working\n");
    for (std::vector<CacheFault>::iterator it = faults.begin();
                                        it != faults.end(); ++it) {

        CacheBlk* blk = static_cast<CacheBlk*>
                        (tags->findBlockBySetAndWay(it->set, it->way));
        if (blk && blk->isValid() && it->cacheToBeInserted == cacheType) {
            if(it->type == 0) { // Permanent
                flipBit(*it, blk, blkSize);
            }else if(it->type == 1 && isRead && it->is_injected == 0) { // Transient
                flipBit(*it, blk, blkSize);
                it->is_injected = 1;
            }
        }    
    }
}

FaultInjector* FaultInjectorParams::create()
{
    return new FaultInjector(this);
}

void
FaultInjector::enableFI(){
    DPRINTF(FaultTrace, "I am enabling fault injection\n");
    enabled = true;
}

void
FaultInjector:: disableFI(){
    DPRINTF(FaultTrace, "I am disabling fault injection\n");
    enabled=false;
}


