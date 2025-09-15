# read id, l, r and generate a tree input with identificador equal to id and with keys in range (l, r)

identificator, begin, end = map(int, input().split())

for i in range(begin, end+1):
    print(1, identificator, i)