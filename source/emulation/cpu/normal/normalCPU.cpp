#include "boxedwine.h"

#include "../decoder.h"
#include "normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../x32/x32CPU.h"
#include "../armv7/armv7CPU.h"
#include "../armv8/armv8CPU.h"
#include "../../../util/ptrpool.h"

#ifdef _DEBUG
#define START_OP(cpu, op) op->log(cpu)
#else
#define START_OP(cpu, op)
#endif

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#define NEXT() cpu->eip.u32+=op->len
#define NEXT_DONE() 
#define NEXT_BRANCH1() cpu->eip.u32+=op->len
#define NEXT_BRANCH2() cpu->eip.u32+=op->len
#else
#define NEXT() cpu->eip.u32+=op->len; op->next->pfn(cpu, op->next)
#define NEXT_DONE() cpu->nextBlock = cpu->getNextBlock();
#define NEXT_BRANCH1() cpu->eip.u32+=op->len; if (!DecodedBlock::currentBlock->next1) {DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next1
#define NEXT_BRANCH2() cpu->eip.u32+=op->len; if (!DecodedBlock::currentBlock->next2) {DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next2
#endif

#include "instructions.h"
#include "normal_arith.h"
#include "normal_conditions.h"
#include "normal_incdec.h"
#include "normal_pushpop.h"
#include "normal_setcc.h"
#include "normal_strings.h"
#include "normal_strings_op.h"
#include "normal_shift.h"
#include "normal_shift_op.h"
#include "normal_xchg.h"
#include "normal_bit.h"
#include "normal_mmx.h"
#include "normal_sse.h"
#include "normal_sse2.h"
#include "normal_fpu.h"
#include "normal_other.h"
#include "normal_jump.h"
#include "normal_move.h"

static OpCallback normalOps[NUMBER_OF_OPS];
static U32 normalOpsInitialized;

void OPCALL normal_sidt(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);    
    U32 eaa = eaa(cpu, op);
    cpu->memory->writew(eaa, 1023); // limit
    cpu->memory->writed(eaa, 0); // base
#ifdef _DEBUG
    klog("sidt not implemented");
#endif
    NEXT();
}

static void initNormalOps() {
    if (normalOpsInitialized)
        return;
    normalOpsInitialized = 1;   
    for (int i=0;i<InstructionCount;i++) {
        normalOps[i] = normal_invalid;
    }
#define INIT_CPU(e, f) normalOps[e] = normal_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#undef INIT_CPU    
    
    normalOps[SLDTReg] = 0; 
    normalOps[SLDTE16] = 0;
    normalOps[STRReg] = 0; 
    normalOps[STRE16] = 0;
    normalOps[LLDTR16] = 0; 
    normalOps[LLDTE16] = 0;
    normalOps[LTRR16] = 0; 
    normalOps[LTRE16] = 0;
    normalOps[VERRR16] = 0; 
    normalOps[VERWR16] = 0; 
    normalOps[SGDT] = 0;
    normalOps[SIDT] = normal_sidt;
    normalOps[LGDT] = 0;
    normalOps[LIDT] = 0;
    normalOps[SMSWRreg] = 0; 
    normalOps[SMSW] = 0;
    normalOps[LMSWRreg] = 0; 
    normalOps[LMSW] = 0;
    normalOps[INVLPG] = 0;
    normalOps[Callback] = onExitSignal;
}

OpCallback NormalCPU::getFunctionForOp(DecodedOp* op) {
    initNormalOps();
    return normalOps[op->inst];
}

NormalCPU::NormalCPU(KMemory* memory) : CPU(memory) {   
    initNormalOps();
#ifdef BOXEDWINE_DYNAMIC
    this->firstOp = firstDynamicOp;
#else
    this->firstOp = NULL;
#endif
}

U8 fetchByte(void* data, U32 *eip) {
    CPU* cpu = (CPU*)data;
    if (*eip - cpu->seg[CS].address == 0xFFFF && !cpu->isBig()) {
        kpanic("eip wrapped around.");
    }
    return cpu->memory->readb((*eip)++);
}

class NormalBlock : public DecodedBlock {
public:
    static NormalBlock* alloc();
    static void clearCache();

    NormalBlock();

    virtual void dealloc(bool delayed);  

    void run(CPU* cpu);

    void reset();
};

NormalBlock::NormalBlock() {
    this->reset();
}

void NormalBlock::run(CPU* cpu) {
#ifdef _DEBUG
    if (this==NULL || this->op==NULL || this->op->pfn==NULL) {
        kpanic("NormalBlock::run is about to crash");
    }
#endif  
    this->op->pfn(cpu, this->op);
    this->runCount++;
    cpu->blockInstructionCount+=this->opCount;
}

static PtrPool<NormalBlock> freeBlocks;

void NormalBlock::reset() {
    this->op = NULL;
    this->bytes = 0;
    this->opCount = 0;
    this->runCount = 0;
    this->next1 = NULL;
    this->next2 = NULL;
    this->referencedFrom = NULL;
}

void NormalBlock::clearCache() {
    freeBlocks.deleteAll();
}

NormalBlock* NormalBlock::alloc() {
    return freeBlocks.get();
}

void NormalBlock::dealloc(bool delayed) {
    KThread* thread = KThread::currentThread();
    if (thread) {
        CPU* cpu = thread->cpu;
        if (cpu && cpu->delayedFreeBlock && cpu->delayedFreeBlock != DecodedBlock::currentBlock) {
            DecodedBlock* b = cpu->delayedFreeBlock;
            cpu->delayedFreeBlock = NULL;
            b->dealloc(false);
        }
        if (cpu && ((delayed && !cpu->delayedFreeBlock) || this == DecodedBlock::currentBlock)) {
            cpu->delayedFreeBlock = this;
        } else {
            if (op) {
                this->op->dealloc(true);
                this->op = nullptr;
            }
            freeBlocks.put(this);
        }
    } else {
        this->op->dealloc(true);
        this->op = nullptr;
        freeBlocks.put(this);
    }
    if (this->next1) {
        this->next1->removeReferenceFrom(this);
        this->next1 = NULL;
    }
    if (this->next2) {
        this->next2->removeReferenceFrom(this);
        this->next2 = NULL;
    }
    DecodedBlockFromNode* from = this->referencedFrom;
    while (from) {
        DecodedBlockFromNode* n = from->next;
        if (from->block->next1 == this) {
            from->block->next1 = NULL;
        }
        if (from->block->next2 == this) {
            from->block->next2 = NULL;
        }
        from->dealloc();
        from = n;
    }
    this->referencedFrom = NULL;
}

DecodedBlock* NormalCPU::getBlockForInspectionButNotUsed(CPU* cpu, U32 address, bool big) {
    DecodedBlock* block = NormalBlock::alloc();
    decodeBlock(fetchByte, cpu, address, big, 0, K_PAGE_SIZE, 0, block);
    block->address = address;
    initNormalOps();
    DecodedOp* op = block->op;
    while (op) {
        if (!op->pfn) // callback will be set by decoder
            op->pfn = normalOps[op->inst];
        op = op->next;
    }
    return block;
}

DecodedBlock* NormalCPU::getNextBlock() {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    return NULL;
#else
    if (!this->thread->process) // exit was called, don't need to pre-cache the next block
        return NULL;

    U32 startIp = (this->big?this->eip.u32:this->eip.u16) + this->seg[CS].address;
    DecodedBlock* block = this->thread->memory->getCodeBlock(startIp);

    if (!block) {
        block = NormalBlock::alloc();
        decodeBlock(fetchByte, this, startIp, this->isBig(), 0, K_PAGE_SIZE, 0, block);
        block->address = startIp;
        
        DecodedOp* op = block->op;
        while (op) {
            if (!op->pfn) // callback will be set by decoder
                op->pfn = normalOps[op->inst];
            op = op->next;
        }
        this->thread->memory->addCodeBlock(startIp, block);
        if (this->firstOp) {
            op = DecodedOp::alloc();
            op->inst = Custom1;
            op->pfn = this->firstOp;
            op->next = block->op;
            block->op = op;
        }
    }
    return block;
#endif
}

void NormalCPU::run() {    
    DecodedBlock::currentBlock = this->nextBlock;
    DecodedBlock::currentBlock->run(this);    
#ifdef _DEBUG
    if (!this->nextBlock && !this->yield) {
        kpanic("NormalCPU::run no block set");
    }
    DecodedBlock::currentBlock = NULL;
#endif
}

void NormalCPU::clearCache() {
    NormalBlock::clearCache();
}
