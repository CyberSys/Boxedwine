void dynamic_jump(DynamicData* data, DynReg reg, U32 inst, U32 len1, U32 len2) {
    startIf(reg, (inst==JumpZ?DYN_NOT_EQUALS_ZERO:DYN_EQUALS_ZERO), true);
    INCREMENT_EIP(len2);
    blockNext2();
    startElse();
    INCREMENT_EIP(len1);
    blockNext1();
    endIf();
    data->done = true;
}

enum DynArg {
    DYN_Mem=0,
    DYN_Reg,
    DYN_Const,
    DYN_None,
};

U32 cpuOffset(U32 r, DynWidth width) {
    if (width==DYN_8bit)
        return OFFSET_REG8(r);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(reg[r].u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(reg[r].u32);
    else {
        kpanic("dynamic cpuOffset unexpected width: %d", width);
        return 0;
    }
}

U32 cpuOffsetResult(DynWidth width) {
    if (width==DYN_8bit)
        return CPU_OFFSET_OF(result.u8);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(result.u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(result.u32);
    else {
        kpanic("dynamic cpuOffsetResult unexpected width: %d", width);
        return 0;
    }
}

U32 cpuOffsetDst(DynWidth width) {
    if (width==DYN_8bit)
        return CPU_OFFSET_OF(dst.u8);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(dst.u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(dst.u32);
    else {
        kpanic("dynamic cpuOffsetDst unexpected width: %d", width);
        return 0;
    }
}

U32 cpuOffsetSrc(DynWidth width) {
    if (width==DYN_8bit)
        return CPU_OFFSET_OF(src.u8);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(src.u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(src.u32);
    else {
        kpanic("dynamic cpuOffsetSrc unexpected width: %d", width);
        return 0;
    }
}

void dynamic_arith(DynamicData* data, DecodedOp* op, DynArg src, DynArg dst, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool isJump = op->next->inst==JumpZ || op->next->inst==JumpNZ;
    bool isSet = op->next->inst==SetZ_E8 || op->next->inst==SetZ_R8 || op->next->inst==SetNZ_E8 || op->next->inst==SetNZ_R8;
    bool needResultReg = isJump || isSet;
    DynReg jmpReg = DYN_NOT_SET;
    bool needsToSetFlags = op->needsToSetFlags();

    if (cf) {
        callHostFunction(common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
    }   
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        INCREMENT_EIP(op->len);
        return;
    }
    if (!needsToSetFlags || (needResultReg && DecodedOp::willOverwriteFlags(data->block, op->next, instructionInfo[op->inst].flagsSets))) {
        if (src == DYN_Reg) {            
            if (dst==DYN_Reg) {
                movToRegFromCpu(DYN_SRC, cpuOffset(op->rm, width), width);
                if (cf) {
                    if (width!=DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                if (needResultReg) {
                    if (op->reg==op->rm && (flags == FLAGS_TEST8 || flags == FLAGS_TEST16 || flags == FLAGS_TEST32)) {
                        jmpReg = DYN_SRC;
                    } else {
                        movToRegFromCpu(DYN_DEST, cpuOffset(op->reg, width), width);
                        instRegReg(inst, DYN_DEST, DYN_SRC, width, true);
                        if (store) {
                            movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, false);
                        }
                        jmpReg = DYN_DEST;
                    }
                } else {
                    instCPUReg(inst, cpuOffset(op->reg, width), DYN_SRC, width, true);
                }
            } else if (dst==DYN_Mem) {
                calculateEaa(op, DYN_ADDRESS);
                movToRegFromCpu(DYN_SRC, cpuOffset(op->reg, width), width);
                if (cf) {
                    if (width!=DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                if (needResultReg) {
                    movFromMem(width, DYN_ADDRESS, false);
                    instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
                    if (store) {
                        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, false);
                    }
                    jmpReg = DYN_CALL_RESULT;
                } else {
                    instMemReg(inst, DYN_ADDRESS, DYN_SRC, width, true, true);
                }
            }
        } else if (src == DYN_Mem) {            
            if (dst==DYN_Reg) {
                calculateEaa(op, DYN_ADDRESS);
                if (cf) {
                    movToRegFromReg(DYN_DEST, width, DYN_CALL_RESULT, DYN_32bit, false);
                    movFromMem(width, DYN_ADDRESS, true);
                    instRegReg('+', DYN_CALL_RESULT, DYN_DEST, width, true);                                        
                } else {
                    movFromMem(width, DYN_ADDRESS, true);
                }
                if (needResultReg) {
                    movToRegFromCpu(DYN_DEST, cpuOffset(op->reg, width), width);
                    instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
                    if (store) {
                        movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, false);
                    }
                    jmpReg = DYN_DEST;
                } else {
                    instCPUReg(inst, cpuOffset(op->reg, width), DYN_CALL_RESULT, width, true);
                }
            }
        } else if (src == DYN_Const) {
            if (dst==DYN_Reg) {
                if (cf) {
                    if (width!=DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegImm('+', DYN_CALL_RESULT, width, op->imm);
                    if (needResultReg) {
                        movToRegFromCpu(DYN_DEST, cpuOffset(op->reg, width), width);
                        instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
                        if (store) {
                            movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, false);
                        }
                        jmpReg = DYN_DEST;
                    } else {
                        instCPUReg(inst, cpuOffset(op->reg, width), DYN_CALL_RESULT, width, true);
                    }
                } else {
                    if (needResultReg) {
                        movToRegFromCpu(DYN_DEST, cpuOffset(op->reg, width), width);
                        instRegImm(inst, DYN_DEST, width, op->imm);
                        if (store) {
                            movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, false);
                        }
                        jmpReg = DYN_DEST;
                    } else {
                        instCPUImm(inst, cpuOffset(op->reg, width), width, op->imm);
                    }
                }
            } else if (dst==DYN_Mem) {
                calculateEaa(op, DYN_ADDRESS);      
                if (cf) {
                    movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
                    instRegImm('+', DYN_SRC, width, op->imm);
                    if (needResultReg) {
                        movFromMem(width, DYN_ADDRESS, false);
                        instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
                        if (store) {
                            movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, false);
                        }
                        jmpReg = DYN_CALL_RESULT;
                    } else {
                        instMemReg(inst, DYN_ADDRESS, DYN_SRC, width, true, true);
                    }
                } else {
                    if (needResultReg) {
                        movFromMem(width, DYN_ADDRESS, false);
                        instRegImm(inst, DYN_CALL_RESULT, width, op->imm);
                        if (store) {
                            movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, false);
                        }
                        jmpReg = DYN_CALL_RESULT;
                    } else {
                        instMemImm(inst, DYN_ADDRESS, width, op->imm, true);
                    }
                }
            }
        }       
    } else {
        if (cf) {
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, false);
        }
        if (src == DYN_Reg) {            
            if (dst==DYN_Reg) {
                movToCpuFromCpu(cpuOffsetSrc(width), cpuOffset(op->rm, width), width, DYN_SRC, false);
                movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(op->reg, width), width, DYN_DEST, false);
                
                if (cf) {
                    if (width!=DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                instRegReg(inst, DYN_DEST, DYN_SRC, width, true);
                movToCpuFromReg(cpuOffsetResult(width), DYN_DEST, width, !store);
                if (store) {
                    movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, true);                
                }
            } else if (dst==DYN_Mem) {
                movToCpuFromCpu(cpuOffsetSrc(width), cpuOffset(op->reg, width), width, DYN_SRC, false);                                
                if (cf) {
                    if (width!=DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, true);                                        
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                calculateEaa(op, DYN_ADDRESS);
                movToCpuFromMem(cpuOffsetDst(width), width, DYN_ADDRESS, !store, false);
                instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);                
                movToCpuFromReg(cpuOffsetResult(width), DYN_CALL_RESULT, width, !store);
                if (store) {
                    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true);
                }
            }
        } else if (src == DYN_Mem) {            
            if (dst==DYN_Reg) {
                movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
                calculateEaa(op, DYN_ADDRESS);
                movToCpuFromMem(cpuOffsetSrc(width), width, DYN_ADDRESS, true, false);
                movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(op->reg, width), width, DYN_DEST, false);
                if (cf) {
                    instRegReg('+', DYN_CALL_RESULT, DYN_SRC, width, true);
                }
                instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
                movToCpuFromReg(cpuOffsetResult(width), DYN_DEST, width, !store);
                if (store) {
                    movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, true);
                }
            }
        } else if (src == DYN_Const) {
            if (dst==DYN_Reg) {
                movToCpu(cpuOffsetSrc(width), width, op->imm);
                movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(op->reg, width), width, DYN_DEST, false);
                instRegImm(inst, DYN_DEST, width, op->imm);
                if (cf) {               
                    if (width!=DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);                    
                }
                movToCpuFromReg(cpuOffsetResult(width), DYN_DEST, width, !store);
                if (store) {
                    movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, true);
                }
            } else if (dst==DYN_Mem) {
                calculateEaa(op, DYN_ADDRESS);      
                movToCpu(cpuOffsetSrc(width), width, op->imm);
                if (cf) {
                    movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);                    
                    movToCpuFromMem(cpuOffsetDst(width), width, DYN_ADDRESS, !store, false);
                    instRegImm('+', DYN_SRC, width, op->imm);
                    instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
                } else {                    
                    movToCpuFromMem(cpuOffsetDst(width), width, DYN_ADDRESS, !store, false);
                    instRegImm(inst, DYN_CALL_RESULT, width, op->imm);                    
                }
                movToCpuFromReg(cpuOffsetResult(width), DYN_CALL_RESULT, width, !store);
                if (store) {
                    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true);
                }
            }
        }
        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)flags);
        data->currentLazyFlags = flags;
    }

    if (!needResultReg || jmpReg==DYN_NOT_SET) {
        INCREMENT_EIP(op->len);
    } else {
        if (jmpReg==DYN_NOT_SET) {
            kpanic("dynamic_arith did not properly set jmpReg");
        }
        if (width!=DYN_32bit) {
            movToRegFromReg(jmpReg, DYN_32bit, jmpReg, width, false);
        }
        if (isJump) {
            dynamic_jump(data, jmpReg, op->next->inst, op->len+op->next->len+op->next->imm, op->len+op->next->len);
        } else if (isSet) {
            INCREMENT_EIP(op->len);
            op = op->next;
            if (op->inst==SetZ_E8 || op->inst==SetNZ_E8) {                
                startIf(jmpReg, (op->inst==SetZ_E8?DYN_EQUALS_ZERO:DYN_NOT_EQUALS_ZERO), true);
                movToReg(DYN_SRC, DYN_8bit, 1);
                startElse();
                movToReg(DYN_SRC, DYN_8bit, 0);
                endIf();
                calculateEaa(op, DYN_ADDRESS);
                movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
            } else {
                startIf(jmpReg, (op->inst==SetZ_R8?DYN_EQUALS_ZERO:DYN_NOT_EQUALS_ZERO), true);
                movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
                startElse();
                movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
                endIf();                
            }
            INCREMENT_EIP(op->len);
            data->skipToOp = op->next;
        }
    }
}

DynWidth getWidthOfCondition(const LazyFlags* flags) {
    if (flags->width==32)
        return DYN_32bit;
    if (flags->width==16)
        return DYN_16bit;
    if (flags->width==8)
        return DYN_8bit;
    kpanic("getWidthOfCondition: invalid flag width: %d", flags->width);
    return DYN_32bit;
}

void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg) {
    if (data->currentLazyFlags==FLAGS_NONE) {
        U32 flag = 0;
        bool not = false;

        switch (condition) {
        case O: flag = OF; break;
        case NO: flag = OF; not = true; break;
        case B: flag = CF; break;
        case NB: flag = CF; not = true; break;
        case Z: flag = ZF; break;
        case NZ: flag = ZF; not = true; break;
        case S: flag = SF; break;
        case NS: flag = SF; not = true; break;
        case P: flag = PF; break;
        case NP: flag = PF; not = true; break;
        }
        if (flag) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(flags), DYN_32bit);
            if (not)
                instReg('~', reg, DYN_32bit);
            instRegImm('&', reg, DYN_32bit, flag);
            return;
        }
    } else if (data->currentLazyFlags && condition==NZ) {
        DynWidth width = getWidthOfCondition(data->currentLazyFlags);
        if (width==DYN_32bit) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        } else if (width==DYN_16bit) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
            movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
        } else if (width==DYN_8bit) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
            movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
        } else {
            kpanic("setConditionInReg: unknown condition width: %d", width);
        }
        return;
    } else if (data->currentLazyFlags && condition==S) {
        DynWidth width = getWidthOfCondition(data->currentLazyFlags);
        if (width==DYN_32bit) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
            instRegImm('&', reg, DYN_32bit, 0x80000000);
        } else if (width==DYN_16bit) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
            instRegImm('&', reg, DYN_16bit, 0x8000);
            movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);            
        } else if (width==DYN_8bit) {
            movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
            instRegImm('&', reg, DYN_8bit, 0x80);
            movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);            
        } else {
            kpanic("setConditionInReg: unknown condition width: %d", width);
        }
        return;
    } else if (data->currentLazyFlags && condition==B) {
        int ii=0;
    }
    switch (condition) {
    case O: callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NO: callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false); break;
    case B: callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NB: callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false); break;
    case Z: callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NZ: callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false); break;
    case BE: callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NBE: callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false); break;
    case S: callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NS: callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false); break;
    case P: callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NP: callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false); break;
    case L: callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NL: callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false); break;
    case LE: callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NLE: callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false); break;
    default:
        kpanic("setConditionInReg: unknown condition %d", condition);
    }
}