/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "normal_shift.h"
void OPCALL rol8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rol8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rol8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rol8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rol16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rol16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rol16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rol32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rol32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rol32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rol32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL ror8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL ror8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL ror8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL ror16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL ror16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL ror16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL ror32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL ror32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL ror32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ror32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rcl8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rcl8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rcl8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rcl16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rcl16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rcl16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rcl32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rcl32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rcl32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcl32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rcr8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rcr8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rcr8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rcr16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rcr16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rcr16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL rcr32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL rcr32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL rcr32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    rcr32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL shl8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL shl8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL shl8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL shl16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL shl16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL shl16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL shl32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL shl32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL shl32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shl32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL shr8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL shr8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL shr8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL shr16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL shr16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL shr16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL shr32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL shr32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL shr32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    shr32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar8_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar8_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL sar8_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar8_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL sar8cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar8cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL sar8cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar16_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar16_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL sar16_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar16_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL sar16cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar16cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL sar16cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar32_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar32_reg(cpu, op->reg, op->imm);
    NEXT();
}
void OPCALL sar32_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar32_mem(cpu, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL sar32cl_reg_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar32cl_reg(cpu, op->reg, CL & 0x1f);
    NEXT();
}
void OPCALL sar32cl_mem_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    sar32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL normal_dshlr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshlr16r16(cpu, op->reg, op->rm, op->imm);
    NEXT();
}
void OPCALL normal_dshle16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshle16r16(cpu, op->reg, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL normal_dshlr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshlr32r32(cpu, op->reg, op->rm, op->imm);
    NEXT();
}
void OPCALL normal_dshle32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshle32r32(cpu, op->reg, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL normal_dshlclr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshlclr16r16(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_dshlcle16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshlcle16r16(cpu, op->reg, eaa(cpu, op));
    NEXT();
}
void OPCALL normal_dshlclr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshlclr32r32(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_dshlcle32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshlcle32r32(cpu, op->reg, eaa(cpu, op));
    NEXT();
}
void OPCALL normal_dshrr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshrr16r16(cpu, op->reg, op->rm, op->imm);
    NEXT();
}
void OPCALL normal_dshre16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshre16r16(cpu, op->reg, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL normal_dshrr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshrr32r32(cpu, op->reg, op->rm, op->imm);
    NEXT();
}
void OPCALL normal_dshre32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshre32r32(cpu, op->reg, eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL normal_dshrclr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshrclr16r16(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_dshrcle16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshrcle16r16(cpu, op->reg, eaa(cpu, op));
    NEXT();
}
void OPCALL normal_dshrclr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshrclr32r32(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_dshrcle32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    dshrcle32r32(cpu, op->reg, eaa(cpu, op));
    NEXT();
}
