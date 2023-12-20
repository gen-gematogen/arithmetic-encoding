eof = 'E'
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
        
def to_cum_prob(prob):
    cum_prob = dict()
    
    cum_prob[0] = 0
    
    for i in range(1, 129):
        cum_prob[i] = cum_prob[i - 1] + (prob[i - 1] if i - 1 in prob else 0) 
        
    return cum_prob

def encode(msg, prob):
    min = 0
    max = whole
    s = 0
    
    buf = data_buf()
    
    cum_prob = to_cum_prob(prob)
    
    for l in msg:
        w = max - min
        max = min + w * cum_prob[ord(l) + 1] // R
        min = min + w * cum_prob[ord(l)] // R
        
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
                
                if c == ord(eof):
                    return ans
                
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
    prob = dict()
                
    for c in range(ord('a'), ord('z')+1):
        prob[c] = R >> 5;
    prob[ord(eof)] = R - 26 * (R >> 5)
    
    with open("input.txt", "r") as f:
        s = f.read()
    s += eof
    
    print("Started encoding")
    
    res = encode(s, prob)
    
    print("Finished encoding")
    print("Started decoding")
    
    s2 = decode(res, prob)
    
    print("Finished decoding")
    
    print(s, s2, s == s2)
    
if __name__ == '__main__':
    main()