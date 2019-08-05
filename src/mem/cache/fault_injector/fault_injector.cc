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

    if(ifs.fail()) {
		panic("Could not read the file.");
    }
    
    int type,blockAddr,byteOffset,bitOffset,tickStart,tickEnd,stuckAt;
    std::string cacheToBeInserted;
    int numberOfFaults = 0;

    DPRINTF(FaultTrace, "\n\n\t%s faults:\n\n", owner);

	while(ifs >> type >> blockAddr >> byteOffset >> bitOffset >> tickStart >> tickEnd >> stuckAt >> cacheToBeInserted){
        if((cacheToBeInserted.compare(owner)) == 0) {
            CacheFault fault;
            fault.type = type;
            fault.blockAddr = blockAddr;
            fault.byteOffset = byteOffset;
            fault.bitOffset = bitOffset;
            fault.tickStart = tickStart;
            fault.tickEnd = tickEnd;
            fault.stuckAt = stuckAt;
            fault.cacheToBeInserted = cacheToBeInserted;
            if(type == 1){ fault.scheduled = 0; }

            faults.push_back(fault);
            DPRINTF(FaultTrace, "Type: %d, Fault address %#x\t stuck at: %d\n", fault.type, fault.blockAddr, fault.stuckAt);

            numberOfFaults++;
        }
	}

    ifs.close();

    DPRINTF(FaultTrace, "Number of faults in total: %d\n", numberOfFaults);
}

void 
FaultInjector::flipBit(CacheFault fault, PacketPtr pkt, unsigned blkSize)
{
    uint8_t* data = pkt->getPtr<uint8_t>();

    if(fault.stuckAt == 0) {
        uint8_t oldValue = data[fault.byteOffset - pkt->getOffset(blkSize)];
        data[fault.byteOffset - pkt->getOffset(blkSize)] &= ~(1UL << fault.bitOffset);
        DPRINTF(FaultTrace, "Old value : %d, New value: %d\n", oldValue, data[fault.byteOffset - pkt->getOffset(blkSize)]);
    } else if(fault.stuckAt == 1) {
        uint8_t oldValue = data[fault.byteOffset - pkt->getOffset(blkSize)];
        data[fault.byteOffset - pkt->getOffset(blkSize)] |= 1UL << fault.bitOffset;
        DPRINTF(FaultTrace, "Old value : %d, New value: %d\n", oldValue, data[fault.byteOffset - pkt->getOffset(blkSize)]);
    } else {
        panic("Invalid stuck at value");
    }
}

void 
FaultInjector::injectFaults(PacketPtr pkt, unsigned blkSize, bool isRead, std::string cacheType) 
{
    for(std::vector<CacheFault>::iterator it = faults.begin(); it != faults.end(); ++it) {
        if(isFaultyAddress(*it, pkt, blkSize) && it->cacheToBeInserted == cacheType) {
            if(isPermanent(*it)) {
                flipBit(*it, pkt, blkSize);
                DPRINTF(FaultTrace, "Permanent fault stuck at %d injected to %#x\n", it->stuckAt ,it->blockAddr + it->byteOffset);
            }else if(isIntermittent(*it) && isFaultActive(*it)) {
                uint8_t* packetData = pkt->getPtr<uint8_t>();
                it->alteredByte = packetData[it->byteOffset - pkt->getOffset(blkSize)]; // Store corrupted byte data so that we can recover later.

                flipBit(*it, pkt, blkSize);
                DPRINTF(FaultTrace, "Intermittent fault stuck at %d injected to %#x\n", it->stuckAt, it->blockAddr + it->byteOffset);
            }else if(isTransient(*it) && it->tickStart <= curTick() && isRead && it->inserted == 0) {
                flipBit(*it, pkt, blkSize);
                it->inserted = 1;
                DPRINTF(FaultTrace, "Transient fault stuck at %d injected to %#x\n", it->stuckAt, it->blockAddr + it->byteOffset);
            }
        }
    }
}

void 
FaultInjector::recoverIntermittentFaults(BaseTags *tags, std::string cacheType)
{
    for(std::vector<CacheFault>::iterator it = faults.begin(); it != faults.end(); ++it) {
        if(isIntermittent(*it) && it->tickEnd + 1 == curTick() && it->cacheToBeInserted == cacheType) {
            CacheBlk *blk = tags->findBlock(it->blockAddr, false);

            if(blk != nullptr) {
                DPRINTF(FaultTrace, "Block found %#x in the cache %s\n", it->blockAddr, cacheType);
                blk->data[it->byteOffset] = it->alteredByte; // recover the altered byte.
            }else {
                DPRINTF(FaultTrace, "Block not found in the cache\n");
            }
        }
    }
}

FaultInjector* FaultInjectorParams::create()
{
    return new FaultInjector(this);
}
