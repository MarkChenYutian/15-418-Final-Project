import random

class Trace:
    def __init__(self, op, key, expect):
        self.op, self.key, self.expect = op, key, expect
    
    def __str__(self):
        if self.op == "BARRIER": return self.op
        elif self.expect is None: return f"{self.op},{self.key},NONE"
        return f"{self.op},{self.key},{self.expect}"

class CaseGenerator:
    OP = ["I", "D", "G", "BARRIER"]
    
    def __init__(self, length, key_low, key_high, fileName, weight):
        self.trace    = []
        self.fileName = fileName
        self.length   = length
        self.weight   = weight
        self.ref      = set()
        self.key_lo   = key_low
        self.key_hi   = key_high

    def generateLine(self):
        lineOp = random.choices(self.OP, weights=self.weight, k=1)[0]
        key    = random.randint(self.key_lo, self.key_hi)
        expect = None
        if (lineOp == "G" and key in self.ref): expect = key
        if (lineOp == "D" and key in self.ref): expect = key
        return Trace(lineOp, key, expect)
    
    def execLine(self, trace):
        if trace.op == "I":
            self.ref.add(trace.key)
        elif trace.op == "D":
            self.ref.discard(trace.key)
    
    def generate(self):
        while len(self.trace) < self.length:
            proposed = self.generateLine()
            if (proposed.op == "I") and (proposed.key in self.ref): continue
            
            self.trace.append(str(proposed))
            self.execLine(proposed)
        
        with open(self.fileName, "w") as f:
            f.write("\n".join(self.trace))

for i in range(10, 20):
    # CaseGenerator(1000, 10, 50, f"small_{i}.case", [.5, .2, .2, .1]).generate()
    CaseGenerator(1000000, 1000, 50000, f"large_{i}.case", [.6, .29, .1, .01]).generate()
# CaseGenerator(10000, 0, 10000, f"IAndG_0.case", [.8, 0., .19, .01]).generate()
