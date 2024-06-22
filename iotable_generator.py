# this program generates an output that produces
# bugs in the target program (i.e. not adding an output value for exactly 1
# product, and adding exactly 1 wonky line with too many digits)

# this issues arise at line NUM_PRODUCTS in the output.

import random, math
import numpy as np

NUM_PRODUCTS = 10_000
MATRIX_DENSITY = 0.01
OUTPUT_WIDTH = 90
NUM_INPUTS = int(MATRIX_DENSITY * NUM_PRODUCTS**2)

upcs = [random.randrange(start=100_000_000_000, 
                         stop=999_999_999_999) 
        for i in range(NUM_PRODUCTS)]


def print_loading_bar(rows_now, total_rows):
    bar_width_now = math.ceil(OUTPUT_WIDTH * (rows_now+1)/total_rows)
    print("| ", "█" * bar_width_now, 
            (OUTPUT_WIDTH - bar_width_now) * " ", "|", 
            f"{(rows_now+1)/total_rows:.2%}",
            end = "\r")

with open("iotable-"+str(NUM_PRODUCTS)+".txt", "w") as iotable_writer:
    gen_iotable = np.empty((2*NUM_PRODUCTS + NUM_INPUTS,3), dtype=int)

    # load labor values
    print("\nLoading labor...")
    for i in range(NUM_PRODUCTS):
        gen_iotable[i] = [upcs[i], 0, random.randrange(start=100, stop=10_000)]
        print_loading_bar(i, NUM_PRODUCTS)


    # load random production inputs
    print("\n\nLoading random production inputs...")
    for i in range(NUM_INPUTS):
        upc1 = upcs[random.randrange(start=0, stop=NUM_PRODUCTS-1)]
        upc2 = upcs[random.randrange(start=0, stop=NUM_PRODUCTS-1)]

        # a product being used in its own production is assumed possible,
        # since this does happen with things like gasoline

        gen_iotable[NUM_PRODUCTS+i] = [upc1, upc2, random.randrange(start=10, stop=10_000)]

        print_loading_bar(i, NUM_INPUTS)


    # load output
    print("\n\nLoading output...")
    index_offset = NUM_PRODUCTS + NUM_INPUTS
    for i in range(NUM_PRODUCTS):
        
        total_units = np.sum(gen_iotable[:,2], where=(gen_iotable[:,0] == upcs[i]))
        gen_iotable[index_offset+i] = [upcs[i], 1, total_units+random.randrange(start=100, stop=1_000)]
        print_loading_bar(i, NUM_PRODUCTS)
        

    for line in gen_iotable:
        writing_line = str(line[0]) + "," + str(line[1]) + " " + str(line[2]) + "\n"
        iotable_writer.writelines(writing_line)


    print("\n\nGeneration complete!\n")
