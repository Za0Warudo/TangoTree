"""
 Generate a random input of Tango Tree Search.

 Given the size of the Tango Tree and the number of queries, generates random queries over tree keys.

 the randomness is defined by the distribution select between the uniform distribution and the normal distribution.

"""

import numpy as np
import sys

def main ():
    args = sys.argv
    try:
        n = int(args[1])    # tango tree size
        q = int(args[2])    # queries quantity
        d = args[3]         # distribution (uniform : u , gaussian : g)

        mean = n/2
        std_dev = n/4

        if d == "u":
            queries = np.random.randint(1, n, q)
        elif d == "g":
            queries = np.clip(np.random.normal(mean, std_dev, q), 1, n).astype(int)
        else:
            raise ValueError

        print(n)
        for query in queries:
            print(f'1 {query}')

    except Exception as e:
        print(f'Use: python3 {args[0]} n q <u|g>')


if __name__ == "__main__":
    main()