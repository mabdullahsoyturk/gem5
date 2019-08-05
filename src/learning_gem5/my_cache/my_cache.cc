#include "learning_gem5/my_cache/my_cache.hh"

#include "base/random.hh"
#include "debug/MyCache.hh"
#include "sim/system.hh"

MyCache::MyCache(MyCacheParams *params) :
    MemObject(params), event([this]{processEvent();}, name()), latency(params->latency), 
    blockSize(params->system->cacheLineSize()), capacity(params->size / blockSize), 
    goldenRun(params->golden_run), faultType(params->fault_type), faultTick(params->fault_tick), 
    faultAddr(params->fault_addr),faultBit(params->fault_bit), memPort(params->name + ".mem_side_port", this), 
    blocked(false), originalPacket(nullptr), waitingPortId(-1)
{
    for (int i = 0; i < params->port_cpu_side_connection_count; ++i) {
        cpuPorts.emplace_back(name() + csprintf(".cpu_side[%d]", i), i, this);
    }
}

void
MyCache::processEvent()
{
    DPRINTF(MyCache, "Processing fault injection!\n");

    RequestPtr req = std::make_shared<Request>(faultAddr, blockSize, 0, 0); // Address: 0x1c0c60
    req->setFlags(Request::SECURE | Request::NO_ACCESS);
    PacketPtr faultyPacket = new Packet(req, MemCmd::WriteReq, blockSize);
    
    uint8_t *data = new uint8_t[blockSize]();
    data[0] |= 1;
    faultyPacket->dataStatic(data);

    cacheStore[faultyPacket->getAddr()] = data;

    uint8_t* temp = cacheStore[faultyPacket->getAddr()];
    printBits(sizeof(temp[0]),&temp[0]);
}

void
MyCache::startup()
{
    schedule(event, 0);
}

Port &
MyCache::getPort(const std::string &if_name, PortID idx)
{
    // This is the name from the Python SimObject declaration in MyCache.py
    if (if_name == "mem_side") {
        panic_if(idx != InvalidPortID, "Mem side of simple cache not a vector port");
        return memPort;
    } else if (if_name == "cpu_side" && idx < cpuPorts.size()) {
        return cpuPorts[idx];
    } else {
        return MemObject::getPort(if_name, idx);
    }
}

void MyCache::CPUSidePort::sendPacket(PacketPtr pkt) 
{
        panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

        DPRINTF(MyCache, "Sending %s to CPU\n", pkt->print());
        if(!sendTimingResp(pkt)) { // If we cannot send the packet accross the port, store it for later
            DPRINTF(MyCache, "failed!\n");
            blockedPacket = pkt;
        }
}

AddrRangeList
MyCache::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void MyCache::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) { // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(MyCache, "Sending retry req\n");
        sendRetryReq();
    }
}

void MyCache::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    return owner->handleFunctional(pkt);
}

bool MyCache::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(MyCache, "Got request %s\n", pkt->print());
    DDUMP(MyCache, pkt->getConstPtr<uint8_t>(), pkt->getSize());  

    if (blockedPacket || needRetry) {
        DPRINTF(MyCache, "Request blocked\n"); // The cache may not be able to send a reply if this is blocked
        needRetry = true;
        return false;
    }
    
    if (!owner->handleRequest(pkt, id)) { // forward to cache.
        DPRINTF(MyCache, "Request failed\n");
        needRetry = true; // stalling.
        return false;
    } else {
        DPRINTF(MyCache, "Request succeeded\n");
        return true;
    }
}

void MyCache::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr); // We should have a blocked packet if this function is called.

    PacketPtr pkt = blockedPacket; // Grab the blocked packet.
    blockedPacket = nullptr;

    DPRINTF(MyCache, "Retrying response pkt %s\n", pkt->print());

    sendPacket(pkt); // Try to resend it. It's possible that it fails again.

    trySendRetry(); // We may now be able to accept new packets.
}

void MyCache::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    if (!sendTimingReq(pkt)) { // If we can't send the packet across the port, store it for later.
        blockedPacket = pkt;
    }
}

bool MyCache::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

void MyCache::MemSidePort::recvReqRetry()
{
    assert(blockedPacket != nullptr); // We should have a blocked packet if this function is called.

    PacketPtr pkt = blockedPacket; // Grab the blocked packet.
    blockedPacket = nullptr;

    sendPacket(pkt); // Try to resend it. It's possible that it fails again.
}

void
MyCache::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

bool MyCache::handleRequest(PacketPtr pkt, int port_id) 
{
    if (blocked) {
        return false; // stall 
    }

    DPRINTF(MyCache, "Got request for addr %#x\n", pkt->getAddr());

    blocked = true; // Blocked, waiting for response to this packet

    assert(waitingPortId == -1);
    waitingPortId = port_id; // Store the port when we get the response.

    schedule(new EventFunctionWrapper([this, pkt]{ accessTiming(pkt); }, name() + ".accessEvent", true), clockEdge(latency));

    return true;
}

bool MyCache::handleResponse(PacketPtr pkt) 
{
    assert(blocked);
    DPRINTF(MyCache, "Got response for addr %#x\n", pkt->getAddr());

    insert(pkt); // For now assume that inserts are off of the critical path and don't count for any added latency.

    missLatency.sample(curTick() - missTime); // for statistics purposes

    if(originalPacket != nullptr) {
        DPRINTF(MyCache, "Copying data from new packet to old\n");
        
        bool hit = accessFunctional(originalPacket); // We had to upgrade a previous packet. We can functionally deal with the cache access now.
        panic_if(!hit, "Should always hit after inserting");
        
        originalPacket->makeResponse();
        
        delete pkt;
        pkt = originalPacket;
        originalPacket = nullptr;
    }

    sendResponse(pkt);

    return true;
}

void MyCache::sendResponse(PacketPtr pkt)
{
    assert(blocked);
    DPRINTF(MyCache, "Sending resp for addr %#x\n", pkt->getAddr());

    int port = waitingPortId;

    // The packet is now done. We're about to put it in the port, no need for
    // this object to continue to stall.
    // We need to free the resource before sending the packet in case the CPU
    // tries to send another request immediately (e.g., in the same callchain).
    blocked = false;
    waitingPortId = -1;

    // Simply forward to the memory port
    cpuPorts[port].sendPacket(pkt);

    // For each of the cpu ports, if it needs to send a retry, it should do it
    // now since this memory object may be unblocked now.
    for (auto& port : cpuPorts) {
        port.trySendRetry();
    }
}

void MyCache::handleFunctional(PacketPtr pkt)
{
    if (accessFunctional(pkt)) {
        pkt->makeResponse();
    } else {
        memPort.sendFunctional(pkt);
    }
}

void MyCache::accessTiming(PacketPtr pkt)
{
    bool hit = accessFunctional(pkt);

    DPRINTF(MyCache, "%s for packet: %s\n", hit ? "Hit" : "Miss", pkt->print());

    if (hit) {
        // Respond to the CPU side
        hits++; // update stats
        DDUMP(MyCache, pkt->getConstPtr<uint8_t>(), pkt->getSize());
        pkt->makeResponse();
        sendResponse(pkt);
    } else {
        misses++; // update stats
        missTime = curTick();
        // Forward to the memory side.
        // We can't directly forward the packet unless it is exactly the size
        // of the cache line, and aligned. Check for that here.
        Addr addr = pkt->getAddr();
        Addr block_addr = pkt->getBlockAddr(blockSize);
        unsigned size = pkt->getSize();
        if (addr == block_addr && size == blockSize) {
            // Aligned and block size. We can just forward.
            DPRINTF(MyCache, "forwarding packet\n");
            memPort.sendPacket(pkt);
        } else {
            DPRINTF(MyCache, "Upgrading packet to block size\n");
            panic_if(addr - block_addr + size > blockSize,
                     "Cannot handle accesses that span multiple cache lines");
            // Unaligned access to one cache block
            assert(pkt->needsResponse());
            MemCmd cmd;
            if (pkt->isWrite() || pkt->isRead()) {
                // Read the data from memory to write into the block.
                // We'll write the data in the cache (i.e., a writeback cache)
                if(pkt->isWrite()) {
                    DPRINTF(MyCache, "\n\n DATA: %s\n\n", pkt->print());
                }
                cmd = MemCmd::ReadReq;
            } else {
                panic("Unknown packet type in upgrade size");
            }

            // Create a new packet that is blockSize
            PacketPtr new_pkt = new Packet(pkt->req, cmd, blockSize);
            new_pkt->allocate();

            // Should now be block aligned
            assert(new_pkt->getAddr() == new_pkt->getBlockAddr(blockSize));

            // Save the old packet
            originalPacket = pkt;

            DPRINTF(MyCache, "forwarding packet\n");
            memPort.sendPacket(new_pkt);
        }
    }
}

bool MyCache::accessFunctional(PacketPtr pkt)
{
    Addr block_addr = pkt->getBlockAddr(blockSize);
    auto it = cacheStore.find(block_addr);
    if (it != cacheStore.end()) {
        if (pkt->isWrite()) {
            // Write the data into the block in the cache
            uint8_t * temp = cacheStore[block_addr];
            printBits(sizeof(temp[0]), &temp[0]);
            
            pkt->writeDataToBlock(it->second, blockSize);
        } else if (pkt->isRead()) {
            // Read the data out of the cache block into the packet
            pkt->setDataFromBlock(it->second, blockSize);
        } else {
            panic("Unknown packet type!");
        }
        return true;
    }
    return false;
}

//assumes little endian
void MyCache::printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void MyCache::insert(PacketPtr pkt)
{
    // The packet should be aligned.
    assert(pkt->getAddr() ==  pkt->getBlockAddr(blockSize));
    // The address should not be in the cache
    assert(cacheStore.find(pkt->getAddr()) == cacheStore.end());
    // The pkt should be a response
    assert(pkt->isResponse());

    if (cacheStore.size() >= capacity) {
        // Select random thing to evict. This is a little convoluted since we
        // are using a std::unordered_map. See http://bit.ly/2hrnLP2
        int bucket, bucket_size;
        do {
            bucket = random_mt.random(0, (int)cacheStore.bucket_count() - 1);
        } while ( (bucket_size = cacheStore.bucket_size(bucket)) == 0 );
        auto block = std::next(cacheStore.begin(bucket),
                               random_mt.random(0, bucket_size - 1));

        DPRINTF(MyCache, "Removing addr %#x\n", block->first);

        // Write back the data.
        // Create a new request-packet pair
        RequestPtr req = std::make_shared<Request>(
            block->first, blockSize, 0, 0);

        PacketPtr new_pkt = new Packet(req, MemCmd::WritebackDirty, blockSize);
        new_pkt->dataDynamic(block->second); // This will be deleted later

        DPRINTF(MyCache, "Writing packet back %s\n", pkt->print());
        // Send the write to memory
        memPort.sendPacket(new_pkt);

        // Delete this entry
        cacheStore.erase(block->first);
    }

    DPRINTF(MyCache, "Inserting %s\n", pkt->print());
    DDUMP(MyCache, pkt->getConstPtr<uint8_t>(), blockSize);

    // Allocate space for the cache block data
    uint8_t *data = new uint8_t[blockSize];

    // Insert the data and address into the cache store
    cacheStore[pkt->getAddr()] = data;

    // Write the data into the cache
    DPRINTF(MyCache, "Writing packet back %d\n", *(cacheStore[pkt->getAddr()]));
    pkt->writeDataToBlock(data, blockSize);
}

AddrRangeList
MyCache::getAddrRanges() const
{
    DPRINTF(MyCache, "Sending new ranges\n");
    // Just use the same ranges as whatever is on the memory side.
    return memPort.getAddrRanges();
}

void MyCache::sendRangeChange()
{
    for (auto& port : cpuPorts) {
        port.sendRangeChange();
    }
}

void
MyCache::regStats()
{
    MemObject::regStats(); // For initialization

    hits.name(name() + ".hits")
        .desc("Number of hits")
        ;

    misses.name(name() + ".misses")
        .desc("Number of misses")
        ;

    missLatency.name(name() + ".missLatency")
        .desc("Ticks for misses to the cache")
        .init(16) // number of buckets
        ;

    hitRatio.name(name() + ".hitRatio")
        .desc("The ratio of hits to the total accesses to the cache")
        ;

    hitRatio = hits / (hits + misses);
}

MyCache*
MyCacheParams::create()
{
    return new MyCache(this);
}
