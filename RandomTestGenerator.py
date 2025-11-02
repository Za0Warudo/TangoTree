"""
 Generate a random input of Tango Tree Search.

 Given the size of the Tango Tree and the number of queries, generates random queries over tree keys.

"""

import random as rnd

def main ():
    n = int(input()) # tango tree size
    q = int(input()) # queries quantity

    print(n)
    for i in range(q):
        print(f'1 {rnd.randint(1, n)}')

if __name__ == "__main__":
    main()