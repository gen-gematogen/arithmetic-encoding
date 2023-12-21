import random 
f = 'sample_data.txt'
al = ['a', 'b']
N = 500
s = ''

for i in range(N):
    s += random.choice(al)

with open(f, 'w') as out_f:
    out_f.write(s)
