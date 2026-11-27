f = open('out.c', 'w');

instrs = [ 'rlc', 'rrc', 'rl', 'rr', 'sla', 'sra', 'swap', 'srl' ];
regs = [ 'a', 'b', 'c', 'd', 'e', 'h', 'l' ];
large_instrs = [ 'bit', 'res', 'set' ];

def func_for_instr(s: str) -> str:
    match s.upper():
        case 'RLC':
            return 'rotlc';
        case 'RRC':
            return 'rotrc';
        case 'RL':
            return 'rotl';
        case 'RR':
            return 'rotr';
        case 'SLA':
            return 'lshift';
        case 'SRA':
            return 'rshift_retain_b7';
        case 'SWAP':
            return 'swp';
        case 'SRL':
            return 'rshift';

def reg(s: str) -> str:
    return f'(state->cpu->{s.upper()})';

def cycles(s: str) -> int:
    return 2;

def fin(s: str) -> str:
    if s.upper() == 'RLC' or s.upper() == 'RRC' or s.upper() == 'RL' or s.upper() == 'RR':
        return ', 1);\n';
    else:
        return ');\n';

for i in instrs:
    for r in regs:
        f.write(f'void {i.lower()}_{r.lower()}(State* state){{\n');
        f.write(f'\t(*state->cpu->PC)++;\n');
        f.write(f'\t{func_for_instr(i)}(state, {reg(r)}{fin(i)}');
        f.write(f'\t(*state->cpu->PC)++;\n');
        f.write(f'\tsleep_cycles(state, 2);\n');
        f.write(f'}}\n\n');
    f.write(f'void {i.lower()}hl(State* state){{\n');
    f.write(f'\tuint8_t a = mm_read(state, *state->cpu->HL);\n');
    f.write(f'\t{func_for_instr(i)}(state, &a{fin(i)}');
    f.write(f'\t(*state->cpu->PC) += 2;\n');
    f.write(f'\tsleep_cycles(state, 2);\n');
    f.write(f'}}\n\n');

def perform(op: str, reg: str, param: int) -> str:
    if reg.upper() == 'HL':
        match op.upper():
            case 'BIT':
                return f'\tuint8_t a = mm_read(state, *state->cpu->HL);\n\tsetZ(state, ~(a & (1 << {param})));\n';
            case 'RES':
                return f'\tuint8_t a = mm_read(state, *state->cpu->HL);\n\tmm_write(state, *state->cpu->HL, a & (~(1 << {param})));\n';
            case 'SET':
                return f'\tuint8_t a = mm_read(state, *state->cpu->HL);\n\tmm_write(state, *state->cpu->HL, a | (1 << {param}));\n';
    else:
        match op.upper():
            case 'BIT':
                return f'\tsetZ(state, ~((*state->cpu->{reg.upper()}) & (1 << {param})));\n';
            case 'RES':
                return f'\t*state->cpu->{reg.upper()} &= (~(1 << {param}));\n';
            case 'SET':
                return f'\t*state->cpu->{reg.upper()} |= (1 << {param});\n';
    return '';

regs.append('hl');

for instr in large_instrs:
    for r in regs:
        for i in range(8):
            f.write(f'void {instr}_{i}_{r.lower()}(State* state){{\n');
            f.write(f'{perform(instr, r, i)}');
            f.write(f'\t(*state->cpu->PC) += 2;\n');
            f.write(f'\tsleep_cycles(state, 2);\n');
            f.write(f'}}\n\n');


