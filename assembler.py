import sys, re
input_assembly=open(sys.argv[1], "rt")
output_binary=open(sys.argv[2], "wb")
size=int(sys.argv[3])
 
#Register Bit Opeand Instructions
TEST_REGEX=re.compile("^\s*TEST\s+([ABXY])\s*,\s*([0-7])\s*(;.*)?$")
PUTF_REGEX=re.compile("^\s*PUTF\s+([ABXY])\s*,\s*([0-7])\s*(;.*)?$")

#Register Register Operand Instructions
MOVE_REGEX=re.compile("^\s*MOVE\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")
XOR_REGEX=re.compile("^\s*XOR\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")
OR_REGEX=re.compile("^\s*OR\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")
AND_REGEX=re.compile("^\s*AND\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")
ADD_REGEX=re.compile("^\s*ADD\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")
ADDF_REGEX=re.compile("^\s*ADDF\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")
SUBF_REGEX=re.compile("^\s*SUBF\s+([ABXY])\s*,\s*([ABXY])\s*(;.*)?$")

#Register Memory Operand Instructions
MEMORY_PC_REGEX=re.compile("^\s*MEMORY\s+([ABXY])\s*,\s*\[PC\+([ABXY])\]\s*(;.*)?$")
MEMORY_DP_REGEX=re.compile("^\s*MEMORY\s+([ABXY])\s*,\s*\[DP\+([ABXY])\]\s*(;.*)?$")
MEMORY_YX_REGEX=re.compile("^\s*MEMORY\s+([ABXY])\s*,\s*\[YX\]\s*(;.*)?$")

#Register Operand Instructions 
JUMP_REGEX=re.compile("^\s*JUMP\s+([ABXY])\s*(;.*)?$")
JUMPLINK_REGEX=re.compile("^\s*JUMPLINK\s+([ABXY])\s*(;.*)?$")
RR_REGEX=re.compile("^\s*RR\s+([ABXY])\s*(;.*)?$")
INCF_REGEX=re.compile("^\s*INCF\s+([ABXY])\s*(;.*)?$")
DECF_REGEX=re.compile("^\s*DECF\s+([ABXY])\s*(;.*)?$")
NOT_REGEX=re.compile("^\s*NOT\s+([ABXY])\s*(;.*)?$")

#No Operand Instructions
HALT_REGEX=re.compile("^\s*HALT\s*(;.*)?$")
LONGJUMP_REGEX=re.compile("^\s*LONGJUMP\s*(;.*)?$")
SET_REGEX=re.compile("^\s*SET\s*(;.*)?$")
RESET_REGEX=re.compile("^\s*RESET\s*(;.*)?$")
IN_REGEX=re.compile("^\s*IN\s*(;.*)?$")
OUT_REGEX=re.compile("^\s*OUT\s*(;.*)?$")
FLIPF_REGEX=re.compile("^\s*FLIPF\s*(;.*)?$")
SKIPF_REGEX=re.compile("^\s*SKIPF\s*(;.*)?$")
MULT_REGEX=re.compile("^\s*MULT\s*(;.*)?$")
YXDP_REGEX=re.compile("^\s*YXDP\s*(;.*)?$")

#16 bit operand instructions
SWAP_DP_REGEX=re.compile("^\s*SWAP\s*DP\s*(;.*)?$")
SWAP_AB_REGEX=re.compile("^\s*SWAP\s*AB\s*(;.*)?$")
SWAP_YX_REGEX=re.compile("^\s*SWAP\s*YX\s*(;.*)?$")
INC_DP_REGEX=re.compile("^\s*INC\s*DP\s*(;.*)?$")
DEC_DP_REGEX=re.compile("^\s*DEC\s*DP\s*(;.*)?$")

#Psuedo Ops
BYTEHEX_REGEX=re.compile("^\s*BYTEHEX\s*([0-9a-fA-F][0-9a-fA-F])\s*(;.*)?$")
EMPTY_LINE_REGEX=re.compile("^\s*(;.*)?$")
NOP_REGEX=re.compile("^\s*NOP\s*(;.*)?$")
RELOCATE_REGEX=re.compile("^\s*RELOCATE\s*([0-9a-fA-F]{1,4})\s*(;.*)?$")
RL_REGEX=re.compile("^\s*RL\s+([ABXY])\s*(;.*)?$")


register_dict={"A":0,"B":1,"X":2,"Y":3}
output_binary_raw=bytearray([0xff]*size)
output_binary_index=-1
while True:
    line=input_assembly.readline()
    if (line==""):
        break
    output_binary_index+=1
    line_match=EMPTY_LINE_REGEX.match(line)
    if(line_match):
        output_binary_index-=1 #reverse this
        continue
    line_match=BYTEHEX_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index] = int(line_match.group(1),16)
        continue
    line_match=RELOCATE_REGEX.match(line)
    if(line_match):
        output_binary_index = int(line_match.group(1),16)-1
        continue
    line_match=TEST_REGEX.match(line)
    if(line_match):
        register=register_dict[line_match.group(1)]<<6
        bit=int(line_match.group(2))<<3
        output_binary_raw[output_binary_index]=register+bit
        continue
    line_match=PUTF_REGEX.match(line)
    if(line_match):
        register=register_dict[line_match.group(1)]<<6
        bit=int(line_match.group(2))<<3
        output_binary_raw[output_binary_index]=register+bit+1
        continue
    line_match=MOVE_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+2
        continue
    line_match=MEMORY_PC_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<5
        source=register_dict[line_match.group(2)]
        if source<=1:
            output_binary_raw[output_binary_index]=(source<<4)+dest+3
        else:
            output_binary_raw[output_binary_index] = ((source-2) << 4) + dest + 7
        continue
    line_match=MEMORY_DP_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<5
        source=register_dict[line_match.group(2)]
        if source<=1:
            output_binary_raw[output_binary_index]=(source<<4)+dest+(0x83)
        else:
            output_binary_raw[output_binary_index] = ((source-2) << 4) + dest + (0x87)
        continue
    line_match=MEMORY_YX_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        output_binary_raw[output_binary_index]=dest+(0x2b)
        continue
    line_match=XOR_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+4
        continue

    line_match=OR_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+5
        continue

    line_match=AND_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+6
        continue
    line_match=ADD_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+12
        continue
    line_match=ADDF_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+13
        continue

    line_match=SUBF_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+14
        continue
    line_match=SUBF_REGEX.match(line)
    if(line_match):
        dest=register_dict[line_match.group(1)]<<6
        source=register_dict[line_match.group(2)]<<4
        output_binary_raw[output_binary_index]=source+dest+14
        continue
    line_match=LONGJUMP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0xef
        continue
    line_match=HALT_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0xff
        continue
    line_match=SET_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0xcf
        continue
    line_match=RESET_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0xdf
        continue
    line_match=IN_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0xbf
        continue
    line_match=OUT_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0xaf
        continue

    line_match=FLIPF_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x7f
        continue
    line_match=SKIPF_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x6f
        continue

    line_match=MULT_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x8f
        continue
    line_match=YXDP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x1a
        continue
    line_match=JUMP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x0b+(register_dict[line_match.group(1)]<<6)
        continue
    line_match=JUMPLINK_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x1b+(register_dict[line_match.group(1)]<<6)
        continue

    line_match=DECF_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x3b+(register_dict[line_match.group(1)]<<6)
        continue
        
    line_match=NOT_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x0a+(register_dict[line_match.group(1)]<<6)
        continue
    line_match=RR_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x2a+(register_dict[line_match.group(1)]<<6)
        continue

    line_match=RL_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x0d+((register_dict[line_match.group(1)]<<4)*5)
        continue
    line_match=INCF_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x3a+(register_dict[line_match.group(1)]<<6)
        continue
    line_match=SWAP_DP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x9f
        continue

    line_match=SWAP_AB_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x4f
        continue
    line_match=SWAP_YX_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x5f
        continue
        
    line_match=INC_DP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x2f
        continue
        
    line_match=DEC_DP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x3f
        continue
    line_match=NOP_REGEX.match(line)
    if(line_match):
        output_binary_raw[output_binary_index]=0x02
        continue
        
    print("SYNTAX ERROR ON LINE: "+line)
    break



output_binary.write(output_binary_raw)