from synacor import Synacor
if __name__ == "__main__":
    syn = Synacor(debug=False)
    syn.load_rom("data/challenge.bin", disassemble=False)
    syn.run()