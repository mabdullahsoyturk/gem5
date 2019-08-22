/** @file
 * Declaration of a structure to insert faults to different levels of caches. 
 * It allows to insert stuck at 0 and stuck at 1 faults for permanent, intermittent 
 * and transient faults that were declared in an input file. In order to see an 
 * example input file, check input.txt in the top level directory of gem5 source code.
 */

#ifndef __MEM_CACHE_FAULT_INJECTOR_HH__
#define __MEM_CACHE_FAULT_INJECTOR_HH__

#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>

#include "params/FaultInjector.hh"
#include "sim/sim_object.hh"
#include "mem/packet.hh"
#include "mem/cache/cache_blk.hh"
#include "mem/cache/base.hh"
#include "mem/cache/tags/base_set_assoc.hh"
#include "mem/cache/tags/base.hh"
#include "base/logging.hh"

class BaseTags;
class CacheBlk;

struct CacheFault {
    int set; // Address of the block that will be corrupted.
    int byteOffset; // Byte offset of the address from the beginning of block address.
    int bitOffset; // Determines which bit of the byte will be corrupted.
    std::string cacheToBeInserted;
};

class FaultInjector : public SimObject
{
    private:
        /** Absolute path of input file that contains faults. */
        std::string inputPath;

        /** Vector that contains faults.  */
        std::vector<CacheFault> faults;

    public:
        FaultInjector(FaultInjectorParams *p);

        /** 
         * Reads all faults from the input file and populates faults vector. 
         * 
         * @param cacheType Which cache owns this fault injector object.
         */
        void init(std::string cacheType);
        
        /** Flips a bit of the data according to stuck at policy. If it is stuck at 1 fault, it flips
         * specified bit to 1. If it is stuck at 0 fault, it flips specified bit to 0.
         * 
         * @param fault The fault that defines the stuck at policy, byte offset and bit offset.
         * @param pkt Pointer to the packet whose data will be corrupted.
         * @param blkSize Size of one block in the cache.
         */
        void flipBit(CacheFault fault, CacheBlk* blk, unsigned blkSize);

        /** Injects all active faults at specified tick. 
         * 
         * @param pkt Packet that will be corrupted.
         * @param blkSize Size of one block in cache.
         * @param isRead Indicates whether we inject faults on a read. This is useful because, for example, there is no point of inserting
         * a transient fault on a write (I am not sure but this logic should probably be changed).  
         * @param cacheType Parameter to understand which cache performs the request.
        */
        void injectFaults(BaseTags* tags, unsigned blkSize, bool isRead, std::string cacheType);
};

#endif // __MEM_CACHE_FAULT_INJECTOR_HH__
