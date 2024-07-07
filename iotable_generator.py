# this program generates an output that produces
# bugs in the target program (i.e. not adding an output value for exactly 1
# product, and adding exactly 1 wonky line with too many digits)

# this issues arise at line num_products in the output.

from argparse import ArgumentParser
from shutil import get_terminal_size
import random, math
import numpy as np

num_products = 10_000
matrix_density = 0.01


def print_loading_bar(rows_now, total_rows):
    output_width = get_terminal_size(fallback=(80,25))[0]-13
    bar_width_now = math.ceil(output_width * (rows_now+1)/total_rows)

    print("| ", "█" * bar_width_now, 
            (output_width - bar_width_now) * " ", "|", 
            f"{(rows_now+1)/total_rows:.2%}",
            end = "\r")


def generateTable(num_products: int, matrix_density: float):

    upcs = [random.randrange(start=100_000_000_000, 
                         stop=999_999_999_999) 
            for i in range(num_products)]
    
    num_inputs = int(matrix_density * num_products**2)  

    with open("iotable-"+str(num_products)+".txt", "w") as iotable_writer:
        gen_iotable = np.empty((2*num_products + num_inputs,3), dtype=int)

        # load labor values
        print("\nLoading labor...")
        for i in range(num_products):
            gen_iotable[i] = [upcs[i], 0, random.randrange(start=100, stop=10_000)]
            print_loading_bar(i, num_products)


        # load random production inputs
        print("\n\nLoading random production inputs...")
        for i in range(num_inputs):
            upc1 = upcs[random.randrange(start=0, stop=num_products-1)]
            upc2 = upcs[random.randrange(start=0, stop=num_products-1)]

            # a product being used in its own production is assumed possible,
            # since this does happen with things like gasoline

            gen_iotable[num_products+i] = [upc1, upc2, random.randrange(start=10, stop=10_000)]

            print_loading_bar(i, num_inputs)


        # load output
        print("\n\nLoading output...")
        index_offset = num_products + num_inputs
        for i in range(num_products):
            
            total_units = np.sum(gen_iotable[:,2], where=(gen_iotable[:,0] == upcs[i]))
            gen_iotable[index_offset+i] = [upcs[i], 1, total_units+random.randrange(start=100, stop=1_000)]
            print_loading_bar(i, num_products)
            

        for line in gen_iotable:
            writing_line = str(line[0]) + "," + str(line[1]) + " " + str(line[2]) + "\n"
            iotable_writer.writelines(writing_line)


def parse_CLI_args():
    arg_parser = ArgumentParser(description="Generates a random input-output table of arbitrary length and density. The file will be titled 'iotable-[number of products].txt', writted to the working directory,")

    arg_parser.add_argument('-n',
                            nargs=1, 
                            type=int, 
                            required=True, 
                            help="(Required) The number of unique products in the table."
                            )
    
    arg_parser.add_argument('-d', 
                            nargs=1, 
                            type=float,
                            required=False, 
                            default=[0.01],
                            help="(Optional) The density of the input-output table (defaults to 0.01)."
                            )
    
    return vars(arg_parser.parse_args())


def main():
    cli_args = parse_CLI_args()

    generateTable(cli_args['n'][0], cli_args['d'][0])
   
    print("\n\nGeneration complete!\n")

if (__name__ == "__main__"):
    main()
