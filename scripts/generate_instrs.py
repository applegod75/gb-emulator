
f = open('out.c', 'w');

regs = [ 'a', 'b', 'c', 'd', 'e', 'h', 'l' ];
instrs = [ 'add', 'adc', 'sub', 'sbc' ];
bitwise_ops = [ 'and', 'xor', 'or' ];

for i in regs:
    for j in regs:
        f.write(f'void ld_{i.lower()}_{j.lower()}(State* state){{\n');
        f.write(f'\t(*state->cpu->{i.upper()}) = (*state->cpu->{j.upper()});\n');
        f.write(f'\t(*state->cpu->PC)++;\n');
        f.write(f'\tsleep_cycle(state);\n');
        f.write(f'}}\n\n');
    f.write(f'void ld_{i.lower()}hl(State* state){{\n');
    f.write(f'\t(*state->cpu->{i.upper()}) = mm_read(state, *state->cpu->HL);\n');
    f.write(f'\t(*state->cpu->PC)++;\n');
    f.write(f'\tsleep_cycles(state, 2);\n');
    f.write(f'}}\n\n');

for i in regs:
    f.write(f'void ldhl_{i.lower()}(State* state){{\n');
    f.write(f'\tmm_write(state, *state->cpu->HL, *state->cpu->{i.upper()});\n');
    f.write(f'\t(*state->cpu->PC)++;\n');
    f.write(f'\tsleep_cycles(state, 2);\n');
    f.write(f'}}\n\n');

f.write('void halt(State* state){\n');
f.write('\tprintf("WARN: reached unimplemented instruction \'HALT\', skipping...\\n");\n');
f.write('}\n\n');

for instr in instrs:
    for i in regs:
        f.write(f'void {instr.lower()}_a_{i.lower()}(State* state){{\n');
        f.write(f'\t{instr.lower()}_u8(state, state->cpu->A, state->cpu->{i.upper()});\n');
        f.write(f'\t(*state->cpu->PC)++;\n');
        f.write(f'\tsleep_cycle(state);\n');
        f.write(f'}}\n\n');
    f.write(f'void {instr.lower()}_ahl(State* state){{\n');
    f.write(f'\tuint8_t v = mm_read(state, *state->cpu->HL);\n');
    f.write(f'\t{instr.lower()}_u8(state, state->cpu->A, &v);\n');
    f.write(f'\t(*state->cpu->PC)++;\n');
    f.write(f'\tsleep_cycles(state, 2);\n');
    f.write(f'}}\n\n');

def str_to_op(s: str) -> str:
    match s.upper():
        case 'AND':
            return '&=';
        case 'XOR':
            return '^=';
        case 'OR':
            return '|=';
    return '';

for op in bitwise_ops:
    for r in regs:
        o = str_to_op(op);
        f.write(f'void {op.lower()}_{r.lower()}(State* state){{\n');
        if op.upper() == 'AND':
            f.write(f'\tsetHalfCarry(state, 1);\n');
        else:
            f.write(f'\tsetHalfCarry(state, 0);\n');
        f.write('\tsetCarry(state, 0);\n');
        f.write('\tsetSubtraction(state, 0);\n');
        f.write(f'\t(*state->cpu->A) {o} (*state->cpu->{r.upper()});\n');
        f.write('\tif ((*state->cpu->A) == 0) {\n');
        f.write('\t\tsetZ(state, 1);\n\t} else {\n');
        f.write('\t\tsetZ(state, 0);\n\t}');
        f.write('\t(*state->cpu->PC)++;\n');
        f.write('\tsleep_cycle(state);\n');
        f.write('}\n\n');
    o = str_to_op(op);
    f.write(f'void {op.lower()}_hl(State* state){{\n');
    if op.upper() == 'AND':
        f.write(f'\tsetHalfCarry(state, 1);\n');
    else:
        f.write(f'\tsetHalfCarry(state, 0);\n');
    f.write('\tsetCarry(state, 0);\n');
    f.write('\tsetSubtraction(state, 0);\n');
    f.write(f'\t(*state->cpu->A) {o} mm_read(state, *state->cpu->HL);\n');
    f.write('\tif ((*state->cpu->A) == 0) {\n');
    f.write('\t\tsetZ(state, 1);\n\t} else {\n');
    f.write('\t\tsetZ(state, 0);\n\t}');
    f.write('\t(*state->cpu->PC)++;\n');
    f.write('\tsleep_cycles(state, 2);\n');
    f.write('}\n\n');

for r in regs:
    f.write(f'void cp_{r.lower()}(State* state){{\n');
    f.write(f'\tuint8_t a = (*state->cpu->A);\n');
    f.write(f'\tsub_u8(state, &a, state->cpu->{r.upper()});\n');
    f.write(f'\t(*state->cpu->PC)++;\n');
    f.write(f'\tsleep_cycle(state);\n');
    f.write(f'}}\n\n');

f.write(f'void cphl(State* state){{\n');
f.write(f'\tuint8_t a = (*state->cpu->A);\n');
f.write(f'\tuint8_t hl = (*state->cpu->HL);\n');
f.write(f'\tsub_u8(state, &a, &hl);\n');
f.write(f'\t(*state->cpu->PC)++;\n');
f.write(f'\tsleep_cycles(state, 2);\n');
f.write(f'}}\n\n');
