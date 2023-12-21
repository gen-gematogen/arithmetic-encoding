from tqdm import tqdm
import argparse
import json

eof = 0
prec = 30
whole = 1<<prec
half = whole>>1
quart = whole>>2
R = 1<<30

class data_buf:
    def __init__(self):
        self.w_bit = 0
        self.r_bit = 0
        self.r_len = 0
        self.buf = []
        
    def add(self, v):
        if self.w_bit == 0:
            self.buf.append(0)
        
        self.buf[-1] |= v<<self.w_bit
        
        self.w_bit += 1
        self.w_bit %= 8
        
    def get(self):
        return self.buf
    
    def size(self):
        return len(self.buf)
    
    def append(self, ch):
        self.buf.append(ch)
       
    def insert(self, s):
        self.buf = s

    def next(self):
        if self.r_len >= len(self.buf):
            return 0
        
        c = (self.buf[self.r_len] >> self.r_bit) & 1
        self.r_bit += 1
        
        if self.r_bit == 8:
            self.r_bit = 0
            self.r_len += 1
            
        return c
    
    def reset_read(self):
        self.r_bit = self.r_len = 0

def rescale_prob(symb_weights):
    prob = dict()
    weight_sum = sum(symb_weights.values())
    cur_sum = 0

    for i in range(128):
        prob[i] =  symb_weights[i] * R // weight_sum
        cur_sum += prob[i]
    prob[eof] += R - cur_sum

    return prob

def to_cum_prob(prob):
    cum_prob = dict()
    
    cum_prob[0] = 0
    
    for i in range(1, 129):
        cum_prob[i] = cum_prob[i - 1] + (prob[i - 1] if i - 1 in prob else 0) 
        
    return cum_prob

def encode(msg, prob):
    global symb_weights
    min = 0
    max = whole
    s = 0
    
    buf = data_buf()
    
    cum_prob = to_cum_prob(prob)
   
    for l in tqdm(msg):
        if l >= 128:
            raise Exception(f"Symbol {l=} not in alphabet")
        w = max - min
        max = min + w * cum_prob[l + 1] // R
        min = min + w * cum_prob[l] // R
        
        while max < half or min > half:
            if max < half:
                min = min<<1
                max = max<<1
                
                buf.add(0)
                for i in range(s):
                    buf.add(1)

                s = 0
                
            elif min > half:
                min = (min - half)<<1
                max = (max - half)<<1
                
                buf.add(1)
                for i in range(s):
                    buf.add(0)
                    
                s = 0
                
        while (min > quart and max < 3 * quart):
            min = (min - quart)<<1
            max = (max - quart)<<1
            s += 1

        if args.mode == 'dynamic':
            symb_weights[l] += 1 
            prob = rescale_prob(symb_weights)
            
    s += 1
    
    if min <= quart:
        buf.add(0)
        
        for i in range(s):
            buf.add(1)
            
    else:
        buf.add(1)
        
        for i in range(s):
            buf.add(0)

    return buf

def decode(data, prob):
    global symb_weights
    min = 0
    max = whole
    value = 0
    ans = ""
    
    cum_prob = to_cum_prob(prob)
    
    for i in range(prec):
        if (data.next()):
            value += 1<<(prec - i - 1)
            
    while True:
        for c in range(127):
            w = max - min
            max_cur = min + w * cum_prob[c + 1] // R
            min_cur = min + w * cum_prob[c] // R 
            
            if min_cur <= value and value < max_cur:
                ans += chr(c)
                min = min_cur
                max = max_cur
                
                if c == eof:
                    return ans
        
                if args.mode == 'dynamic':
                    symb_weights[c] += 1 
                    prob = rescale_prob(symb_weights)

                break
            
        while max < half or min > half:
            if max < half:
                min = min<<1
                max = max<<1
                value = value<<1
                
            elif min > half:
                min = (min - half)<<1
                max = (max - half)<<1
                value = (value - half)<<1
                
            if data.next():
                value += 1
                
        while min > quart and max < 3 * quart:
            min = (min - quart)<<1
            max = (max - quart)<<1
            value = (value - quart)<<1
            
            if data.next():
                value += 1
                
    return ans

def main():
    global args, symb_weights
    parser = argparse.ArgumentParser(description='Process some integers.')
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-e', '--encode', action = 'store_const', const = True)
    group.add_argument('-d', '--decode', action = 'store_const', const = True)
    parser.add_argument('-m', '--mode', choices = ['static', 'dynamic'])
    parser.add_argument(nargs = 1, dest = 'in_f', metavar = 'Input_name', help = 'Enter name of input file')
    parser.add_argument(nargs = 1, dest = 'out_f', metavar = 'Output_name', help = 'Enter name of output file')

    args = parser.parse_args()

    prob = dict()
    if args.mode == 'dynamic':
        symb_weights = dict()
        for i in range(128):
            symb_weights[i] = 1 
        prob = rescale_prob(symb_weights)
    else:
        with open('prob.cfg', 'r') as in_f:
            cfg = in_f.read()
        cfg = json.loads(cfg)
        
        for i in cfg:
            prob[int(i)] = cfg[i]

    if args.encode:
        with open(args.in_f[0], "rb") as f:
            s = f.read()
    
        print("Started encoding")
        print("Input size:", len(s))
    
        res = encode(s + eof.to_bytes(1), prob)
    
        print("Finished encoding")
        print("Writing data to the file")

        with open(args.out_f[0], "wb") as f:
            f.write(bytes(res.get()))

        print("Done!")
    
    elif args.decode:
        with open(args.in_f[0], "rb") as f:
            s = f.read()
        
        print("Started decoding")
  
        buf = data_buf()
        buf.insert(s)

        res = decode(buf, prob)[:-1]
    
        print("Finished decoding")
        print("Output size:", len(res))
        print("Writing data to the file")

        with open(args.out_f[0], 'w') as f:
            f.write(res)

        print("Done!")
        
if __name__ == '__main__':
    main()
