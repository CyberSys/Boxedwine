#include "boxedwine.h"

#ifdef BOXEDWINE_X64
#include "x64Ops.h"
#include "x64CPU.h"
#include "x64Asm.h"
#include "../../hardmmu/hard_memory.h"
#include "x64CodeChunk.h"
#include "../normal/normalCPU.h"
#include "../binaryTranslation/btCodeMemoryWrite.h"

CPU* CPU::allocCPU() {
    return new x64CPU();
}

// hard to guage the benifit, seems like 1% to 3% with quake 2 and quake 3
bool x64CPU::hasBMI2 = true;
bool x64Intialized = false;

x64CPU::x64CPU() {
    if (!x64Intialized) {
        x64Intialized = true;
        x64CPU::hasBMI2 = platformHasBMI2();
    }
}

void x64CPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
    this->negSegAddress[index] = (U32)(-((S32)(this->seg[index].address)));
}

void x64CPU::restart() {
	this->memOffset = this->thread->process->memory->id;
	this->negMemOffset = (U64)(-(S64)this->memOffset);
	for (int i = 0; i < 6; i++) {
		this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
	}
	this->exitToStartThreadLoop = true;
}

void* x64CPU::init() {
    X64Asm data(this);
    void* result;
    Memory* memory = this->thread->memory;
    x64CPU* cpu = this;

    this->negMemOffset = (U64)(-(S64)this->memOffset);
    for (int i = 0; i < 6; i++) {
        this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
    }

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->executableMemoryMutex);
    this->eipToHostInstructionAddressSpaceMapping = this->thread->memory->eipToHostInstructionAddressSpaceMapping;
    this->memOffsets = memory->memOffsets;

	// will push 15 regs, since it is odd, it will balance rip being pushed on the stack and give use a 16-byte alignment
	data.saveNativeState(); // also sets HOST_CPU

    //data.writeToRegFromValue(HOST_CPU, true, (U64)this, 8);
    data.writeToRegFromValue(HOST_MEM, true, cpu->memOffset, 8);

    if (KSystem::useLargeAddressSpace) {
        data.writeToRegFromValue(HOST_LARGE_ADDRESS_SPACE_MAPPING, true, (U64)cpu->eipToHostInstructionAddressSpaceMapping, 8);
    } else {
        data.writeToRegFromValue(HOST_SMALL_ADDRESS_SPACE_SS, true, (U32)cpu->seg[SS].address, 4);
    }

    data.setNativeFlags(this->flags, FMASK_TEST|DF);

    data.writeToRegFromValue(0, false, EAX, 4);
    data.writeToRegFromValue(1, false, ECX, 4);
    data.writeToRegFromValue(2, false, EDX, 4);
    data.writeToRegFromValue(3, false, EBX, 4);
    data.writeToRegFromValue(HOST_ESP, true, ESP, 4);
    data.writeToRegFromValue(5, false, EBP, 4);
    data.writeToRegFromValue(6, false, ESI, 4);
    data.writeToRegFromValue(7, false, EDI, 4);        
    
    data.calculatedEipLen = 1; // will force the long x64 chunk jump
    data.doJmp(false);
    std::shared_ptr<BtCodeChunk> chunk = data.commit(true);
    result = chunk->getHostAddress();
    link(&data, chunk);
    this->pendingCodePages.clear();    
    this->eipToHostInstructionPages = this->thread->memory->eipToHostInstructionPages;

    if (!this->thread->process->returnToLoopAddress) {
        X64Asm returnData(this);
        returnData.restoreNativeState();
        returnData.write8(0xfc); // cld
        returnData.write8(0xc3); // retn
        std::shared_ptr<X64CodeChunk> chunk2 = returnData.commit(true);
        this->thread->process->returnToLoopAddress = chunk2->getHostAddress();
    }
    this->returnToLoopAddress = this->thread->process->returnToLoopAddress;

    if (!this->thread->process->reTranslateChunkAddress) {
        X64Asm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<X64CodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddress = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddress = this->thread->process->reTranslateChunkAddress;
    if (!this->thread->process->reTranslateChunkAddressFromReg) {
        X64Asm translateData(this);
        translateData.createCodeForRetranslateChunk(true);
        std::shared_ptr<X64CodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddressFromReg = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddressFromReg = this->thread->process->reTranslateChunkAddressFromReg;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        X64Asm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary(true);
        std::shared_ptr<X64CodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessary = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
#endif
#ifdef BOXEDWINE_POSIX
    if (!this->thread->process->runSignalAddress) {
        X64Asm translateData(this);
        translateData.createCodeForRunSignal();
        std::shared_ptr<X64CodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->runSignalAddress = chunk3->getHostAddress();
    }
#endif
    return result;
}

std::shared_ptr<BtCodeChunk> x64CPU::translateChunk(U32 ip) {
    return this->translateChunk(NULL, ip);
}

std::shared_ptr<BtCodeChunk> x64CPU::translateChunk(X64Asm* parent, U32 ip) {
    X64Asm data1(this);
    data1.ip = ip;
    data1.startOfDataIp = ip;       
    data1.parent = parent;
    translateData(&data1);

    X64Asm data(this);
    data.ip = ip;
    data.startOfDataIp = ip;  
    data.calculatedEipLen = data1.ip - data1.startOfDataIp;
    data.parent = parent;
    translateData(&data, &data1);        
    S32 failedJumpOpIndex = this->preLinkCheck(&data);

    if (failedJumpOpIndex==-1) {
        std::shared_ptr<BtCodeChunk> chunk = data.commit(false);
        link(&data, chunk);
        return chunk;
    } else {
        X64Asm data2(this);
        data2.ip = ip;
        data2.startOfDataIp = ip;       
        data2.parent = parent;
        data2.stopAfterInstruction = failedJumpOpIndex;
        translateData(&data2);

        X64Asm data3(this);
        data3.ip = ip;
        data3.startOfDataIp = ip;  
        data3.calculatedEipLen = data2.ip - data2.startOfDataIp;
        data3.parent = parent;
        data3.stopAfterInstruction = failedJumpOpIndex;
        translateData(&data3, &data2);

        std::shared_ptr<BtCodeChunk> chunk = data3.commit(false);
        link(&data3, chunk);
        return chunk;
    }    
}

void* x64CPU::translateEipInternal(U32 ip) {
    return translateEipInternal(NULL, ip);
}

void* x64CPU::translateEipInternal(X64Asm* parent, U32 ip) {
    if (!this->isBig()) {
        ip = ip & 0xFFFF;
    }
    U32 address = this->seg[CS].address+ip;
    void* result = this->thread->memory->getExistingHostAddress(address);

    if (!result) {
        std::shared_ptr<BtCodeChunk> chunk = this->translateChunk(parent, ip);
        result = chunk->getHostAddress();
        chunk->makeLive();
    }
    return result;
}

#ifdef __TEST
void x64CPU::postTestRun() {
    for (int i = 0; i < 8; i++) {
        reg_mmx[i].q = *((U64*)(fpuState + 32 + i * 16));
    }
    for (int i = 0; i < 8; i++) {
        xmm[i].pi.u64[0] = *((U64*)(fpuState + 160 + i * 16));
        xmm[i].pi.u64[1] = *((U64*)(fpuState + 160 + i * 16 + 8));
    }
}

void x64CPU::addReturnFromTest() {
    X64Asm data(this);
    data.addReturnFromTest();
    data.commit(true);
}
#endif

S32 x64CPU::preLinkCheck(X64Asm* data) {
    for (S32 i=0;i<(S32)data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;        
        U8 size = data->todoJump[i].offsetSize;

        if (size==4 && data->todoJump[i].sameChunk) {
            bool found = false;

            for (U32 ip=0;ip<data->ipAddressCount;ip++) {
                if (data->ipAddress[ip] == eip) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return data->todoJump[i].opIndex;
            }
        }
    }
    return -1;
}

void x64CPU::link(X64Asm* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk) {
    U32 i;
    if (!fromChunk) {
        kpanic("x64CPU::link fromChunk missing");
    }
    for (i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;        
        U8* offset = (U8*)fromChunk->getHostAddress()+offsetIntoChunk+data->todoJump[i].bufferPos;
        U8 size = data->todoJump[i].offsetSize;

        if (size==4 && data->todoJump[i].sameChunk) {
            U8* host = (U8*)fromChunk->getHostFromEip(eip);
            if (!host) {
                kpanic("x64CPU::link can not link into the middle of an instruction");
            }
            data->write32Buffer(offset, (U32)(host - offset - 4));            
        } else if (size==4 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)this->thread->memory->getExistingHostAddress(eip);

            if (!toHostAddress) {
                U8 op = 0xce;
                U32 hostIndex = 0;
                std::shared_ptr<X64CodeChunk> chunk = std::make_shared<X64CodeChunk>(1, &eip, &hostIndex, &op, 1, eip-this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();            
            }
            std::shared_ptr<BtCodeChunk> toChunk = this->thread->memory->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("x64CPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, true);
            data->write32Buffer(offset, (U32)(toHostAddress - offset - 4));            
        } else if (size==8 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)this->thread->memory->getExistingHostAddress(eip);

            if (!toHostAddress) {
                X64Asm returnData(this);
                returnData.startOfOpIp = eip - this->seg[CS].address;
                returnData.callRetranslateChunk();
                U32 hostIndex = 0;
                std::shared_ptr<X64CodeChunk> chunk = std::make_shared<X64CodeChunk>(1, &eip, &hostIndex, returnData.buffer, returnData.bufferPos, eip - this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();
            }
            std::shared_ptr<BtCodeChunk> toChunk = this->thread->memory->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("x64CPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, false);
            data->write64Buffer(offset, (U64)&(link->toHostInstruction));
        } else {
            kpanic("x64CPU::link unexpected patch size");
        }
    }
    markCodePageReadOnly(data);
}

void x64CPU::translateInstruction(X64Asm* data, X64Asm* firstPass) {
    data->startOfOpIp = data->ip;  
    data->useSingleMemOffset = !data->cpu->thread->memory->doesInstructionNeedMemoryOffset(data->ip);
#ifdef _DEBUG
    //data->logOp(data->ip);
    // just makes debugging the asm output easier
#ifndef __TEST
    data->writeToMemFromValue(data->ip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
#endif
#endif
    if (data->dynamic) {
        data->addDynamicCheck(false);
    } else {
#ifdef _DEBUG
        //data->addDynamicCheck(true);
#endif
    }   
    while (1) {  
        data->op = data->fetch8();            
        data->inst = data->baseOp + data->op;        
        if (!x64Decoder[data->inst](data)) {                
            break;
        }            
    }
    data->tmp1InUse = false;
    data->tmp2InUse = false;
    data->tmp3InUse = false;
}

void x64CPU::translateData(X64Asm* data, X64Asm* firstPass) {
    U32 codePage = (data->ip+data->cpu->seg[CS].address) >> K_PAGE_SHIFT;
    U32 nativePage = this->thread->memory->getNativePage(codePage);
    if (this->thread->memory->dynamicCodePageUpdateCount[nativePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
    while (1) {  
        U32 address = data->cpu->seg[CS].address+data->ip;
        void* hostAddress = this->thread->memory->getExistingHostAddress(address);
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
        if (firstPass) {
            U32 nextEipLen = firstPass->calculateEipLen(data->ip+data->cpu->seg[CS].address);
            U32 page = (data->ip+data->cpu->seg[CS].address+nextEipLen) >> K_PAGE_SHIFT;

            if (page!=codePage) {
                codePage = page;
                nativePage = this->thread->memory->getNativePage(codePage);
                if (data->dynamic) {                    
                    if (this->thread->memory->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // continue to cross from my dynamic page into another dynamic page
                    } else {
                        // we will continue to emit code that will self check for modified code, even though the page we spill into is not dynamic
                    }
                } else {
                    if (this->thread->memory->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // we crossed a page boundry from a non dynamic page to a dynamic page
                        data->dynamic = true; // the instructions from this point on will do their own check
                    } else {
                        // continue to cross from one non dynamic page into another non dynamic page
                    }
                }
            }
        }
        data->mapAddress(address, data->bufferPos);
        translateInstruction(data, firstPass);
        if (data->done) {
            break;
        }
        if (data->stopAfterInstruction!=-1 && (int)data->ipAddressCount==data->stopAfterInstruction) {
            break;
        }
        data->resetForNewOp();
    }     
}

// for string instruction, we modify (add memory offset and segment) rdi and rsi so that the native string instruction can be used, this code will revert it back to the original values
bool x64CPU::fixStringOp(DecodedOp* op, U64 rsi, U64 rdi) {
    if (op->inst == Lodsb || op->inst == Lodsw || op->inst == Lodsd) {
        THIS_ESI = (U32)(rsi - this->memOffset);
        if (this->thread->process->hasSetSeg[op->base]) {
            THIS_ESI -= this->seg[op->base].address;
        }
        return true;
    }
    // uses di (Examples: diablo 1 will trigger this in the middle of the Stosd when creating a new game)
    else if (op->inst == Stosb || op->inst == Stosw || op->inst == Stosd ||
        op->inst == Scasb || op->inst == Scasw || op->inst == Scasd) {
        THIS_EDI = (U32)(rdi - this->memOffset);
        if (this->thread->process->hasSetSeg[ES]) {
            THIS_EDI -= this->seg[ES].address;
        }
        return true;
    }
    // uses si and di
    else if (op->inst == Movsb || op->inst == Movsw || op->inst == Movsd ||
        op->inst == Cmpsb || op->inst == Cmpsw || op->inst == Cmpsd) {
        THIS_ESI = (U32)(rsi - this->memOffset);
        THIS_EDI = (U32)(rdi - this->memOffset);
        if (this->thread->process->hasSetSeg[ES]) {
            THIS_EDI -= this->seg[ES].address;
        }
        if (this->thread->process->hasSetSeg[op->base]) {
            THIS_ESI -= this->seg[op->base].address;
        }
        return true;
    }
    return false;
}

U64 x64CPU::getRipFromEip() {
    U32 a = this->getEipAddress();
    U64 result = (U64)this->thread->memory->getExistingHostAddress(a);
    if (!result) {
        this->translateEip(this->eip.u32);
        result = (U64)this->thread->memory->getExistingHostAddress(a);
    }
    if (result == 0) {
        kpanic("x64::getRipFromEip failed to translate code");
    }
    return result;
}

// if the page we are writing to has code that we have cached, then it will be marked NATIVE_FLAG_CODEPAGE_READONLY
//
// 1) This function will clear the page of all cached code
// 2) Mark all the old cached code with "0xce" so that if the program tries to run it again, it will re-compile it
// 3) NATIVE_FLAG_CODEPAGE_READONLY will be removed
U64 x64CPU::handleCodePatch(U64 rip, U32 address, U64 rsi, U64 rdi, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
#endif
    Memory* memory = this->thread->memory;
    U32 nativePage = memory->getNativePage(address >> K_PAGE_SHIFT);
    // get the emulated eip of the op that corresponds to the host address where the exception happened
    std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress((void*)rip);
    
    this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)rip, NULL, NULL) - this->seg[CS].address;

    // make sure it wasn't changed before we got the executableMemoryMutex lock
    if (!(memory->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        return getRipFromEip();
    }    

    // get the emulated op that caused the write
    DecodedOp* op = this->getOp(this->eip.u32, true);
    if (op) {  
        if (doSyncFrom) {
            doSyncFrom(op);
        }
        if (this->flags & DF) {
            this->df = -1;
        } else {
            this->df = 1;
        }
        U32 addressStart = address;
        U32 len = instructionInfo[op->inst].writeMemWidth / 8;

        // Fix string will set EDI and ESI back to their correct values so we can re-enter this instruction
        if (fixStringOp(op, rsi, rdi)) {
            if (op->repNotZero || op->repZero) {
                len = len * (isBig() ? THIS_ECX : THIS_CX);
            }

            if (this->df == 1) {
                addressStart = (this->isBig() ? THIS_EDI : THIS_DI) + this->seg[ES].address;
            } else {
                addressStart = (this->isBig() ? THIS_EDI : THIS_DI) + this->seg[ES].address + (instructionInfo[op->inst].writeMemWidth / 8) - len;
            }

            // only need to sync back if we changed ESI or EDI for string op
            if (doSyncTo) {
                doSyncTo(op);
            }
        }
        U32 startPage = addressStart >> K_PAGE_SHIFT;
        U32 endPage = (addressStart + len - 1) >> K_PAGE_SHIFT;
        memory->clearHostCodeForWriting(memory->getNativePage(startPage), memory->getNativePage(endPage - startPage + 1));            
        op->dealloc(true);
        return getRipFromEip();
    } else {                        
        kpanic("Threw an exception from a host location that doesn't map to an emulated instruction");
    }
    return 0;
}

U64 x64CPU::handleMissingCode(U64 r8, U64 r9, U32 inst) {
    U32 page = (U32)r8;
    U32 offset = (U32)r9;

    this->eip.u32 = ((page << K_PAGE_SHIFT) | offset) - this->seg[CS].address;
    this->translateEip(this->eip.u32);  
    if (inst==0xCA148B4F) {
        return (U64)(this->eipToHostInstructionPages[page]);
    } else {
        return (U64)(this->eipToHostInstructionPages[page][offset]);
    }
}

U32 dynamicCodeExceptionCount;

U64 x64CPU::handleAccessException(U64 rip, U64 address, bool readAddress, std::function<U64(U32 reg)>getReg, std::function<void(U32 reg, U64 value)>setReg, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) {
    if ((address & 0xFFFFFFFF00000000l) == this->thread->memory->id) {
        U32 emulatedAddress = (U32)address;
        U32 page = emulatedAddress >> K_PAGE_SHIFT;
        Memory* m = this->thread->memory;
        U32 flags = m->flags[page];
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);

        std::shared_ptr<BtCodeChunk> chunk = m->getCodeChunkContainingHostAddress((void*)rip);
        if (chunk) {
            this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)rip, NULL, NULL) - this->seg[CS].address;
            // if the page we are trying to access needs a special memory offset and this instruction isn't flagged to looked at that special memory offset, then flag it
            if (flags & PAGE_MAPPED_HOST && (((flags & PAGE_READ) && readAddress) || ((flags & PAGE_WRITE) && !readAddress))) {
                m->setNeedsMemoryOffset(getEipAddress());
                DecodedOp* op = this->getOp(this->eip.u32, true);
                fixStringOp(op, getReg(6), getReg(7)); // if we were in the middle of a string op, then reset RSI and RDI so that we can re-enter the same op
                chunk->releaseAndRetranslate();
                return getRipFromEip();
            }
            else {
                if (!readAddress && (flags & PAGE_WRITE) && !(this->thread->memory->nativeFlags[this->thread->memory->getNativePage(page)] & NATIVE_FLAG_CODEPAGE_READONLY)) {
                    // :TODO: this is a hack, why is it necessary
                    m->updatePagePermission(page, 1);
                    return 0;
                } else {
                    int ii = 0;
                }
            }
        }
    }

    U32 inst = *((U32*)rip);
    U64 r9 = getReg(9);
    U64 r8 = getReg(8);

    if (inst == 0xCE24FF43 && this->thread->memory->isValidReadAddress((U32)r9, 1)) { // useLargeAddressSpace = true
        this->translateEip((U32)r9 - this->seg[CS].address);
        return 0;
    } else if ((inst==0x0A8B4566 || inst==0xCA148B4F) && (r8 || r9)) { // if these constants change, update handleMissingCode too     
        // rip is not adjusted so we don't need to check for stack alignment
        setReg(10, this->handleMissingCode(r8, r9, inst));
        return 0;
    } else if (inst==0xcdcdcdcd) {
        // this thread was waiting on the critical section and the thread that was currently in this handler removed the code we were running
        void* host = this->thread->memory->getExistingHostAddress(this->eip.u32+this->seg[CS].address);
        if (host) {
            return (U64)host;
        } else {
            U64 result = (U64)this->translateEip(this->eip.u32); 
            if (!result) {
                kpanic("x64CPU::handleAccessException tried to run code in a free'd chunk");
            }
            return result;
        }
    } else {          
        // check if the emulated memory caused the exception
        if ((address & 0xFFFFFFFF00000000l) == this->thread->memory->id) {                
            U32 emulatedAddress = (U32)address;
            U32 page = emulatedAddress >> K_PAGE_SHIFT;
            // check if emulated memory that caused the exception is a page that has code
            if (this->thread->memory->nativeFlags[this->thread->memory->getNativePage(page)] & NATIVE_FLAG_CODEPAGE_READONLY) {
                dynamicCodeExceptionCount++;                    
                return this->handleCodePatch(rip, emulatedAddress, getReg(6), getReg(7), doSyncFrom, doSyncTo);                    
            }
        }   
#ifdef _DEBUG
        void* fromHost = this->thread->memory->getExistingHostAddress(this->fromEip);
        std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress((void*)rip);
#endif
        if (doSyncFrom) {
            doSyncFrom(NULL);
        }
        // this can be exercised with Wine 5.0 and CC95 demo installer, it is triggered in strlen as it tries to grow the stack
        this->thread->seg_mapper((U32)address, readAddress, !readAddress, false);
        if (doSyncTo) {
            doSyncTo(NULL);
        }
        U64 result = (U64)this->translateEip(this->eip.u32); 
        if (result==0) {
            kpanic("x64CPU::handleAccessException failed to translate code");
        }
        return result;
    }
}

#endif
