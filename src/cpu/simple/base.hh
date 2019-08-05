#ifndef __CPU_SIMPLE_BASE_HH__
#define __CPU_SIMPLE_BASE_HH__

#include "base/statistics.hh"
#include "config/the_isa.hh"
#include "cpu/base.hh"
#include "cpu/checker/cpu.hh"
#include "cpu/exec_context.hh"
#include "cpu/pc_event.hh"
#include "cpu/simple_thread.hh"
#include "cpu/static_inst.hh"
#include "mem/packet.hh"
#include "mem/port.hh"
#include "mem/request.hh"
#include "sim/eventq.hh"
#include "sim/full_system.hh"
#include "sim/system.hh"

// forward declarations
class Checkpoint;
class Process;
class Processor;
class ThreadContext;

namespace TheISA
{
    class DTB;
    class ITB;
}

namespace Trace {
    class InstRecord;
}

struct BaseSimpleCPUParams;
class BPredUnit;
class SimpleExecContext;

class BaseSimpleCPU : public BaseCPU
{
  protected:
    ThreadID curThread;
    BPredUnit *branchPred;

    void checkPcEventQueue();
    void swapActiveThread();

  public:
    BaseSimpleCPU(BaseSimpleCPUParams *params);
    virtual ~BaseSimpleCPU();
    void wakeup(ThreadID tid) override;
    void init() override;
  public:
    Trace::InstRecord *traceData;
    CheckerCPU *checker;

    std::vector<SimpleExecContext*> threadInfo;
    std::list<ThreadID> activeThreads;

    /** Current instruction */
    TheISA::MachInst inst;
    StaticInstPtr curStaticInst;
    StaticInstPtr curMacroStaticInst;

  protected:
    enum Status {
        Idle,
        Running,
        Faulting,
        ITBWaitResponse,
        IcacheRetry,
        IcacheWaitResponse,
        IcacheWaitSwitch,
        DTBWaitResponse,
        DcacheRetry,
        DcacheWaitResponse,
        DcacheWaitSwitch,
    };

    Status _status;

  public:
    Addr dbg_vtophys(Addr addr);


    void checkForInterrupts();
    void setupFetchRequest(const RequestPtr &req);
    void preExecute();
    void postExecute();
    void advancePC(const Fault &fault);

    void haltContext(ThreadID thread_num) override;

    // statistics
    void regStats() override;
    void resetStats() override;

    void startup() override;

    virtual Fault readMem(Addr addr, uint8_t* data, unsigned size,
                          Request::Flags flags,
                          const std::vector<bool>& byteEnable =
                              std::vector<bool>())
    { panic("readMem() is not implemented\n"); }

    virtual Fault initiateMemRead(Addr addr, unsigned size,
                                  Request::Flags flags,
                                  const std::vector<bool>& byteEnable =
                                      std::vector<bool>())
    { panic("initiateMemRead() is not implemented\n"); }

    virtual Fault writeMem(uint8_t* data, unsigned size, Addr addr,
                           Request::Flags flags, uint64_t* res,
                           const std::vector<bool>& byteEnable =
                               std::vector<bool>())
    { panic("writeMem() is not implemented\n"); }

    virtual Fault amoMem(Addr addr, uint8_t* data, unsigned size,
                         Request::Flags flags,
                         AtomicOpFunctor *amo_op)
    { panic("amoMem() is not implemented\n"); }

    virtual Fault initiateMemAMO(Addr addr, unsigned size,
                                 Request::Flags flags,
                                 AtomicOpFunctor *amo_op)
    { panic("initiateMemAMO() is not implemented\n"); }

    void countInst();
    Counter totalInsts() const override;
    Counter totalOps() const override;

    void serializeThread(CheckpointOut &cp, ThreadID tid) const override;
    void unserializeThread(CheckpointIn &cp, ThreadID tid) override;

};

#endif // __CPU_SIMPLE_BASE_HH__
