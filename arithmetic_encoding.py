from tqdm import tqdm

eof = chr(0)
prec = 30
whole = 1<<prec
half = whole>>1
quart = whole>>2
R = 1<<30


eng_alphabet = "abcdefghijklmnopqrstuvwxyz"
digits = ""
for i in range(1, 128):
    if not chr(i).isalpha():
        digits += chr(i)
alphabet = eng_alphabet + eng_alphabet.upper() + digits + eof
eng_freq = {
    'a': 0.08167,
    'b': 0.01492,
    'c': 0.02782,
    'd': 0.04253,
    'e': 0.12702,
    'f': 0.02228,
    'g': 0.02015,
    'h': 0.06094,
    'i': 0.06966,
    'j': 0.00153,
    'k': 0.00772,
    'l': 0.04025,
    'm': 0.02406,
    'n': 0.06749,
    'o': 0.07507,
    'p': 0.01929,
    'q': 0.00095,
    'r': 0.05987,
    's': 0.06327,
    't': 0.09056,
    'u': 0.02758,
    'v': 0.00978,
    'w': 0.02360,
    'x': 0.00150,
    'y': 0.01974,
    'z': 0.00074
}

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
    
    for l in tqdm(msg):
        if l > 128:
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
                
    summ = 0
    # 80% of frequency is english lovercase letters
    for c in eng_alphabet:
        cur = int(eng_freq[c] * R // 5 * 4)
        prob[ord(c)] = cur
        summ += cur
    # 10% of frequency is english uppercase letters
    for c in eng_alphabet.upper():
        cur = int(eng_freq[c.lower()] * R // 10)
        prob[ord(c)] = cur
        summ += cur        
    # almost 10% of frequency is digits
    for c in digits:
        cur = int(R // (11 * len(digits)))
        prob[ord(c)] = cur
        summ += cur
        
    prob[ord(eof)] = R - summ
    
    with open("input_ascii.txt", "rb") as f:
        s = f.read()
    
    print("Started encoding")
    print("Input size:", len(s))
    
    res = encode(s + eof.encode(), prob)
    
    print("Finished encoding")
    
    with open("output.bin", "wb") as f:
        f.write(bytes(res.get()))
    
    print("Started decoding")
    
    s2 = decode(res, prob)
    s2 = s2[:-1]
    
    print("Finished decoding")
    print("Output size:", len(s2))
    
    if len(s) != len(s2):
        print("Lengths are not equal")
        return
    print("Lengths are equal, checking for differences")
    for i in range(len(s)):
        if s[i] != ord(s2[i]):
            print(i, s[i], ord(s2[i]))
            raise Exception("Strings are not equal")
    print("Strings are equal!")
    
    with open("output_ascii.txt", "wb") as f:
        f.write(s2.encode())
    
if __name__ == '__main__':
    main()
