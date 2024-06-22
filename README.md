# Cockshott and Cottrell's price approximation algorithm

## Introduction
Some have objected to a planned economy on the basis of the staggeringly large computation required to determine prices, a part of the objections collectively known as [the economic calculation problem](https://en.wikipedia.org/wiki/Economic_calculation_problem). For much of the history of computing, this calculation has been completely unfeasible, at least when the problem is pursued using input-output tables and Gaussian elimination. But Paul Cockshott and Allin Cottrell, in their book, [*Towards a New Socialism*](https://users.wfu.edu/cottrell/socialism_book/new_socialism.pdf), propose a simple algorithm which, given an input-output table, will approximate prices in terms of labor-value (hours per unit of product), and will run in linear time complexity (as opposed to Gaussian elimination's $O(n^3)$ time complexity). This is proposed to demonstrate the feasibility of centralized economic planning. Their complete planning model has more components than this, so this is only a part of the whole model they develop in the book. Dr. Cockshott programmed an implementation of the full model himself, using Java and Pascal, available [here](https://github.com/wc22m/5yearplan) on GitHub.

Note that my implementation here is still in development, and will still need some fine-tuning.

## The algorithm
The algorithm works iteratively. Since the algorithm exists to find prices in terms of labor, and labor is an input in the input-output table, the prices list is initialized with only the labor values from the table for each product. Then, the per-unit cost of each product is calculated using this prices list, giving an updated list of prices. Then this updated list of prices is used to calculate unit costs again to get a different, slightly higher prices list. This is then repeated to obtain the desired degree of accuracy. In short:

  1. Initialize list of prices with only labor input from input-output table.
  2. Calculate the cost of each product using the initialized list.
  3. Use these costs to calculate new list of costs for each product.
  4. Repeat step 4 to arbitrary accuracy.

## Program installation 
You can use CMake to build and install the program on your machine with the following commands, after navigating to the directory where you cloned this repository:

```
cmake .
cmake --build .
cmake --build . --target install
```

The resulting executable is named `plecpr` (for ***pl***anned ***ec***onomy ***pr***ices) and is installed in the working directory.

## CLI usage
`plecpr` has the following options:

Option | Meaning
--- | ---
`-f file_path` | (*required*) File path to input-output table as a `.txt` file, formatted as shown above.
`-i iterations` | (*optional, if* `-p` *given*) Number of iterations to use in applying the algorithm.
`-p precision` | (*optional, if* `-i` *given*) The precision at which the algorithm is to stop iterating, in terms of decimal places (an integer).
`-o output_file` | (*optional*) File path to `.csv` file for writing calculated prices to. If not provided, the prices will be printed to the console.
`-h` | Display help/usage.

## Implementation details
Pursuant to Cockshott and Cottrell's observation that an input-output table is a sparse matrix, this program implements a sparse matrix with C++'s STL `std::unordered_map` object, functioning as a hash table, with the index being a struct of the matrix coordinates. One `std::unordered_map` represents an input-output table for the whole national economy over some definite unit of time, and a second represents the hash table of prices, hashing on the UPCs of each product.

### Definitions and expected data formats

#### Coordinates
The coordinates are intended to represent UPC barcodes, as suggested in the book. The exceptions to this are that 0 denotes the labor input column, and 1 denotes the column where the quantity produced is recorded. 

#### Pre-processed Data Table Format
The code expects a `.txt` file with each line having the universal product code UPC of the product produced, a comma, the UPC of the input, a space, and the quantity of the input. Since the input-output table is a sparse matrix, this format will save lots of space that would otherwise just be holding zeros. An example line in the file would be:

&ensp;`101010282293,882872662923 239.7`

This line encodes that, in making product 101010282293, 239.7 units of product 882872662923 were used. Consistent with earlier definitions, when the first number after the comma is 0, the number after the space represents the number of person-hours (directly) expended to produce the units of the product. And when the number after the comma is 1, the number after the space is the units produced of the product whose UPC is at the beginning of the line. An example of each of these would be:

&ensp;`011010282293,0 40112.23`

&ensp;`011010282293,1 76234.60`

The first encodes the usage of 40112.23 person-hours of labor in the making of product 011010282293, and the second encodes that 76234.60 units of product 011010282293 were produced. 


## Time complexity analysis
Cockshott and Cottrell argue that their algorithm has the following time complexity for $n$ products:

$$
O(nfi)
$$

where $f$ is the average number of other products used in the production of each good, and $i$ is the number of iterations. 

In testing this implementation, seems to have a higher-order time complexity than the linear complexity the authors claim. For $n$ products (using the `iotable` files available in the repository, and bigger product files that were too large for GitHub), I got the following runtimes, each for 10 iterations. Each is compared as a ratio to the last, and compared with the runtime ratio the authors' time complexity would imply (given by $(n_k f_k)/(n_{k-1} f_{k-1})$, for row $2 < k < 5$):

$n$ | $f$ | Runtime | Expected ratio to previous row's run | Measured ratio to previous row's run
--- | --- | --- | --- | ---
14 | 1.57 | 9 ms | (n/a) | (n/a)
100 | 1 | 23 ms | 4.55 | 2.55
1,000 | 10 | 277 ms | 100 | 12.04
10,000 | 100 | 28,155 ms | 100 | 101.64
20,000 | 200 | 111,640 ms | 4 | 3.97
50,000 | 50 | 70,981 ms | 0.625 | 0.636

This implementation far outstrips projections for smaller numbers, but the gap is closes up as $n$ grows. The closeness of measured ratios to expected ratios at higher $n$ values indicates that the time does in fact vary proportional to $n$ and $f$. This suggests the time complexity the authors propose is correct, and that the implementation is a decent one.

## Miscellaneous files
There are a few randomly generated input-output tables with 15, 100, and 1,000 products in them. I also included the Python script I used to generate tables of arbitrary size and density, as `iotable_generator.py`. In that script, you can tweak the number of products as well as the matrix density. `iotable_spreadsheet.xlsx` is a full input-output table for the 15-product file as a Excel spreadsheet, so you can see what the matrix would look like fully expanded in a simple format. And `prices.csv` is an example of what the program's output would look like. 
