/** @file
 * Declaration of a structure to insert faults to different levels of caches. 
 * It allows to insert stuck at 0 and stuck at 1 faults for permanent and 
 * transient faults that were declared in an input file.
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
    int type; // Type of fault : permanent(0) or transient(1)
    int set; // Set of the fault
    int way; // Way of the fault
    int byteOffset; // Byte offset of the address from the beginning of block address.
    int bitOffset; // Determines which bit of the byte will be corrupted.
    std::string cacheToBeInserted; // Indicates which cache to be corrupted.
    int is_injected = 0; // Do not inject transient fault if already injected.
};

extern FaultInjector *gFIptr;

class FaultInjector : public SimObject
{
    private:
        /** Absolute path of input file that contains faults. */
        std::string inputPath;

        /** Vector that contains faults.  */
        std::vector<CacheFault> faults;

        /** Whether the fault injector is enabled. */ 
        bool enabled;
        
        /** Set associativity of the cache */ 
        const unsigned assoc;

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
         * @param blk Cache block that will be corrupted
         * @param blkSize Size of one block in the cache.
         */
        void flipBit(CacheFault fault, CacheBlk* blk, unsigned blkSize);

        /** Injects all active faults at specified tick. 
         * 
         * @param tags Cache tags.
         * @param blkSize Size of one block in cache.
         * @param isRead Indicates whether we inject faults on a read. This is useful because, for example, there is no point of inserting
         * a transient fault on a write.  
         * @param cacheType Parameter to understand which cache performs the request.
        */
        void injectFaults(BaseTags* tags, unsigned blkSize, bool isRead, std::string cacheType);

        void enableFI();
        void disableFI();
};

#endif // __MEM_CACHE_FAULT_INJECTOR_HH__
