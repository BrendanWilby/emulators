import sys
import os

MEMORY_SIZE = 32768
REG_START = 32768
REG_END = 32775
NUM_REGISTERS = 8
OP_LENGTH_1 = 1
OP_LENGTH_2 = 2
OP_LENGTH_3 = 3
OP_LENGTH_4 = 4

class Synacor():
    def __init__(self, debug=False):
        self.stack = []
        self.debug = debug
        self.running = False
        self.pc = 0
        self.memory = [0 for i in range(MEMORY_SIZE)]
        self.registers = [0 for i in range(NUM_REGISTERS)]

        self.instructions = {
            0 : self.op_halt,
            1 : self.op_set,
            2 : self.op_push,
            3 : self.op_pop,
            4 : self.op_eq,
            5 : self.op_gt,
            6 : self.op_jmp,
            7 : self.op_jt,
            8 : self.op_jf,
            9 : self.op_add,
            10 : self.op_mult,
            11 : self.op_mod,
            12 : self.op_and,
            13 : self.op_or,
            14 : self.op_not,
            15 : self.op_rmem,
            16 : self.op_wmem,
            17 : self.op_call,
            18 : self.op_ret,
            19 : self.op_out,
            20 : self.op_in,
            21 : self.op_noop
        }

        self.disassembly = {
            0   : ("HALT", OP_LENGTH_1),
            1   : ("SET", OP_LENGTH_3),
            2   : ("PUSH", OP_LENGTH_2),
            3   : ("POP", OP_LENGTH_2),
            4   : ("EQ", OP_LENGTH_4),
            5   : ("GT", OP_LENGTH_4),
            6   : ("JMP", OP_LENGTH_2),
            7   : ("JT", OP_LENGTH_3),
            8   : ("JF", OP_LENGTH_3),
            9   : ("ADD", OP_LENGTH_4),
            10  : ("MULT", OP_LENGTH_4),
            11  : ("MOD", OP_LENGTH_4),
            12  : ("AND", OP_LENGTH_4),
            13  : ("OR", OP_LENGTH_4),
            14  : ("NOT", OP_LENGTH_3),
            15  : ("RMEM", OP_LENGTH_3),
            16  : ("WMEM", OP_LENGTH_3),
            17  : ("CALL", OP_LENGTH_2),
            18  : ("RET", OP_LENGTH_1),
            19  : ("OUT", OP_LENGTH_2),
            20  : ("IN", OP_LENGTH_2),
            21  : ("NOOP", OP_LENGTH_1)
        }

    def load_rom(self, file_path, disassemble=False):
        rom_buffer = []

        # load rom into rom_buffer
        with open(file_path, "rb") as rom_file:
            file_contents = rom_file.read()

            for i in range(0, len(file_contents), 2):
                rom_buffer.append(file_contents[i + 1] << 8 | file_contents[i])
        
        rom_size = len(rom_buffer)
        
        for i in range(rom_size):
            self.memory[i] = rom_buffer[i]

        if disassemble:
            diss_file_path = "disassembly.txt"
            diss_file = open(diss_file_path, "w")
            buff_pc = 0
            op_params = ""

            while buff_pc < rom_size:
                opcode = rom_buffer[buff_pc]

                if opcode in self.disassembly.keys():
                    (mnemonic, op_length) = self.disassembly[opcode]
                    op_params_list = []

                    # OUT opcode should print ASCII character
                    if mnemonic == "OUT":
                        # print out the arguments of each successive OUT instruction until a new line or another instruction is encountered
                        while self.disassembly[rom_buffer[buff_pc + op_length]][0] == "OUT" and chr(rom_buffer[buff_pc + 1]) != "\n":
                            op_params_list.append(chr(rom_buffer[buff_pc + 1]))
                            buff_pc += op_length

                        op_params = "".join(op_params_list)
                    else:
                        for i in range(buff_pc + 1, buff_pc + op_length):
                            rom_val = rom_buffer[i]

                            # Print out register name in form "R0", "R1", ..., "R7"
                            if rom_val >= REG_START and rom_val <= REG_END:
                                op_params_list.append(f"R{rom_val - REG_START}")
                            else:
                                op_params_list.append(str(rom_val))
                    
                        op_params = ", ".join(op_params_list)
                    
                    diss_file.write(f"{mnemonic} {op_params}\n")

                    buff_pc += op_length
                else:
                    diss_file.write(f"Invalid OPCODE ({str(opcode)})\n")
                    buff_pc += 1

            diss_file.close()
            print(f"Disassembly successfully written to {diss_file_path}")

    def run(self):
        self.running = True

        while self.running:
            opcode = self.memory[self.pc]
            
            if self.debug:
                (mnemonic, op_length) = self.disassembly[opcode]
                op_params = ", ".join(str(param) for param in self.memory[self.pc + 1 : self.pc + op_length])
                print(f"{mnemonic} {op_params}\n")

            self.instructions[opcode]()

    # Reads value in memory at address and returns the value stored in a register, or just the value stored in memory at that address
    # NOT to be used for getting references to registers
    def get_value(self, address):
        mem_value = self.memory[address]

        if mem_value >= REG_START and mem_value <= REG_END:
            return self.registers[mem_value - REG_START]
        elif mem_value > REG_END:
            raise Exception("Trying to read invalid data")

        return mem_value

    def set_value(self, address, value):
        if address >= REG_START and address <= REG_END:
            self.registers[address - REG_START] = value
        else:
            self.memory[address] = value

    """
    ============================================================
            INSTRUCTIONS
    ============================================================
    """
    # halt: 0
    # stop execution and terminate the program
    def op_halt(self):
        self.running = False
        self.pc += 1

    # set: 1 a b
    # set register <a> to the value of <b>
    def op_set(self):
        reg_a = self.memory[self.pc + 1]
        val_b = self.get_value(self.pc + 2)

        self.set_value(reg_a, val_b)

        self.pc += 3

    # push: 2 a
    # push <a> onto the stack
    def op_push(self):
        val_a = self.get_value(self.pc + 1)
        self.stack.append(val_a)

        self.pc += 2

    # pop: 3 a
    # remove the top element from the stack and write it into <a>; empty stack = error
    def op_pop(self):
        if self.stack:
            stack_val = self.stack.pop()
            ref_a = self.memory[self.pc + 1]
            self.set_value(ref_a, stack_val)
        else:
            raise Exception("Stack is empty")

        self.pc += 2

    # eq: 4 a b c
    # set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
    def op_eq(self):
        ref_a = self.memory[self.pc + 1]

        self.set_value(ref_a, self.get_value(self.pc + 2) == self.get_value(self.pc + 3))

        self.pc += 4

    # gt: 5 a b c
    # set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
    def op_gt(self):
        ref_a = self.memory[self.pc + 1]

        self.set_value(ref_a, self.get_value(self.pc + 2) > self.get_value(self.pc + 3))

        self.pc += 4

    # jmp: 6 a
    # jump to <a>
    def op_jmp(self):
        self.pc = self.get_value(self.pc + 1)

    # jt: 7 a b
    # if <a> is nonzero, jump to <b>
    def op_jt(self):
        if self.get_value(self.pc + 1) > 0:
            self.pc = self.get_value(self.pc + 2)
        else:
            self.pc += 3

    # jf: 8 a b
    # if <a> is zero, jump to <b>
    def op_jf(self):
        if self.get_value(self.pc + 1) == 0:
            self.pc = self.get_value(self.pc + 2)
        else:
            self.pc += 3
    
    # add: 9 a b c
    # assign into <a> the sum of <b> and <c> (modulo 32768)
    def op_add(self):
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, (self.get_value(self.pc + 2) + self.get_value(self.pc + 3)) % 32768)

        self.pc += 4

    # mult: 10 a b c
    # store into <a> the product of <b> and <c> (modulo 32768)
    def op_mult(self):
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, (self.get_value(self.pc + 2) * self.get_value(self.pc + 3)) % 32768)
        self.pc += 4

    # mod: 11 a b c
    # store into <a> the remainder of <b> divided by <c>
    def op_mod(self):
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, self.get_value(self.pc + 2) % self.get_value(self.pc + 3))
        self.pc += 4

    # and: 12 a b c
    # stores into <a> the bitwise and of <b> and <c>
    def op_and(self):
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, self.get_value(self.pc + 2) & self.get_value(self.pc + 3))
        self.pc += 4

    # or: 13 a b c
    # stores into <a> the bitwise or of <b> and <c>
    def op_or(self):
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, self.get_value(self.pc + 2) | self.get_value(self.pc + 3))
        self.pc += 4

    # not: 14 a b
    # stores 15-bit bitwise inverse of <b> in <a>
    def op_not(self):
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, ~self.get_value(self.pc + 2) & 0x7FFF)
        self.pc += 3

    # rmem: 15 a b
    # read memory at address <b> and write it to <a>
    def op_rmem(self):
        ref_a = self.memory[self.pc + 1]
        addr_b = self.get_value(self.pc + 2)
        mem_b = self.memory[addr_b]

        self.set_value(ref_a, mem_b)

        self.pc += 3

    # wmem: 16 a b
    # write the value from <b> into memory at address <a>
    def op_wmem(self):
        val_b = self.get_value(self.pc + 2)
        addr_a = self.get_value(self.pc + 1)

        self.set_value(addr_a, val_b)
        self.pc += 3

    # call: 17 a
    # write the address of the next instruction to the stack and jump to <a>
    def op_call(self):
        self.stack.append(self.pc + 2)
        self.pc = self.get_value(self.pc + 1)

    # ret: 18
    # remove the top element from the stack and jump to it; empty stack = halt
    def op_ret(self):
        if self.stack:
            self.pc = self.stack.pop()
        else:
            self.op_halt()
    
    # out: 19 a
    # write the character represented by ascii code <a> to the terminal
    def op_out(self):
        val_a = self.get_value(self.pc + 1)

        if not self.debug:
            print(chr(val_a), end='')

        self.pc += 2

    # in: 20 a
    # read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts
    # it will continue until a newline is encountered; this means that you can safely read whole lines from the
    # keyboard and trust that they will be fully read
    def op_in(self):
        console_input = sys.stdin.read(1)
        
        ref_a = self.memory[self.pc + 1]
        self.set_value(ref_a, ord(console_input))
        
        self.pc += 2
    
    # noop: 21
    # no operation
    def op_noop(self):
        self.pc += 1
