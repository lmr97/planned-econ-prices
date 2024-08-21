// This is the main driver for the project

#include "ioTableAnalysis.hpp"
using namespace std;


// This function checks whether two numbers are equal within the given precision
// (number of decimal points)
bool precisionReached(unordered_map<long int, double>& prevIterPrices,
                      unordered_map<long int, double>& currIterPrices,
                      const int precision)
{
    if (currIterPrices.empty()) return false;

    double precisionUnit = pow(10, -precision); 
    for (const auto &[productKey, price] : currIterPrices)
    {
        if (abs(price - prevIterPrices.at(productKey)) > precisionUnit) return false;
    }

    return true;
}


// These functions, calcPricesConstIter (1) and calcPricesPrec (2) calculate prices, 
// and return a reference to the prices unordered_map.
// They follow Cockshott and Cottrell's algorithm as laid out in Chapter 3 of
// Toward a New Socialism (1993), but have different stopping points.

// (1) This implementation stops after a given number of iterations has bee reached
void calcPricesConstIter(unordered_map<ProdInputPair,double>& ioTable,
                         unordered_map<long, double>& prices,
                         const int iterations)
{
    // initialize previous (in this case, initial) iteration prices list,
    // using only direct labor
    unordered_map<long int, double> laborOnlyPrices;
    unordered_map<long int, double> prevIterPrices;
    ProdInputPair PIpair{0,0};
    long int currentProduct{0};

    for (auto iter = ioTable.begin(); iter != ioTable.end(); ++iter)
    {   
        if (iter->first.product != currentProduct)                    // to prevent redundant statements
        {
            currentProduct = iter->first.product;
            PIpair.product = currentProduct;
            PIpair.input = 0;

            laborOnlyPrices[currentProduct] = ioTable.at(PIpair);  // start with the labor value...

            // ...and divide it by quantity produced of that product
            PIpair.input = 1;
            laborOnlyPrices[currentProduct] /= ioTable.at(PIpair);
        }
    }


    // constant-iteration algorithm
    // if iterations==0, this loop and following conditional will be 
    // skipped and will go to the precision-based iterations.
    cout << "\nNow running iterations." << endl;

    prevIterPrices = laborOnlyPrices;
    prices = laborOnlyPrices;
    for (int i = 0; i < iterations; i++)
    {
        for (auto iter = ioTable.begin(); iter != ioTable.end(); ++iter)
        {
            PIpair = iter->first;               // key loaded into local variable for speed

            // no need to consider the output quantity column (irrelevant)
            // or labor column (already accounted for)
            if(PIpair.input == 1 || PIpair.input == 0) continue;     
            
            // add prevIterPrices cost of the input to the current product's total cost, 
            // and divide by total units produced to get the per-unit cost of that input
            double priceAddition = ioTable.at(PIpair) * prevIterPrices.at(PIpair.input);
            PIpair.input = 1;
            priceAddition       /= ioTable.at(PIpair);

            prices.at(PIpair.product) += priceAddition;
        }

        prevIterPrices = prices;          // save last iteration's prices...
        prices = laborOnlyPrices;         // ...and start fresh for current iteration

        cout << "iteration " << i+1 << " of " << iterations << " complete" << endl;
    }
}


// (2) This implementation stops after a certain precision has been reached
void calcPricesPrec(unordered_map<ProdInputPair,double>& ioTable,
                    unordered_map<long, double>& prices,
                    const int precision)
{
    // initialize previous (in this case, initial) iteration prices list,
    // using only direct labor
    unordered_map<long int, double> laborOnlyPrices;
    unordered_map<long int, double> prevIterPrices;
    ProdInputPair PIpair{0,0};
    long int currentProduct{0};

    for (auto iter = ioTable.begin(); iter != ioTable.end(); ++iter)
    {   
        if (iter->first.product != currentProduct)                    // to prevent redundant statements
        {
            currentProduct = iter->first.product;
            PIpair.product = currentProduct;
            PIpair.input = 0;

            laborOnlyPrices[currentProduct] = ioTable.at(PIpair);  // start with the labor value...

            // ...and divide it by quantity produced of that product
            PIpair.input = 1;
            laborOnlyPrices[currentProduct] /= ioTable.at(PIpair);
        }
    }


    // precision-based algorithm
    cout << "Now iterating until precision == " << precision << endl;
    int iterCounter{1};

    prices = laborOnlyPrices;
    do 
    {
        prevIterPrices = prices;     // save last iteration's prices...
        prices = laborOnlyPrices;    // ...and start fresh for current iteration


        for (auto iter = ioTable.begin(); iter != ioTable.end(); ++iter)
        {
            PIpair = iter->first;               // key loaded into local variable for speed

            // no need to consider the output quantity column (irrelevant)
            // or labor column (already accounted for)
            if(PIpair.input == 1 || PIpair.input == 0) continue;     
            
            // add prevIterPrices cost of the input to the current product's total cost, 
            // and divide by total units produced to get the per-unit cost of that input
            double priceAddition = ioTable.at(PIpair) * prevIterPrices.at(PIpair.input);
            PIpair.input = 1;
            priceAddition       /= ioTable.at(PIpair);

            prices.at(PIpair.product) += priceAddition;
        }

        //printPrices(currIterPrices);
        cout << "iteration " << iterCounter << " complete" << endl;
        iterCounter++;
    }
    while(!precisionReached(prevIterPrices, prices, precision));

}


// main can take the location of the .txt file
int main(int argc, char* argv[])
{
    auto start = chrono::high_resolution_clock::now();

    char* fileLoc{nullptr};
    int precision{0};
    int iterations{0};
    char* outputFile{nullptr};

    // crash if there were CLI errors
    try
    {
        bool helpPrinted = parseCmdOptions(argc, argv, fileLoc, precision, iterations, outputFile);
    }
    catch (const exception& e)
    {
        printHelp(argv[0]);
        cerr << e.what() << endl;
        return 0;
    }
    

    // load table
    unordered_map<ProdInputPair,double> ioTable;
    try
    {
        bool ioTableLoaded = loadIOTable(fileLoc, ioTable);
    }
    catch (const bad_file& bf)
    {
        cerr << bf.what() << endl;
        return 0;
    }


    unordered_map<long int, double> prices;
    if (precision)  calcPricesPrec(ioTable, prices, precision);
    if (iterations) calcPricesConstIter(ioTable, prices, iterations);

    if (outputFile) savePricesToFile(prices, outputFile);
    else printPrices(prices);


    auto stop     = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop-start);

    cout << setprecision(5) << "\nTime taken (seconds): " << duration.count()/1000.0 << endl << endl;

    return 0;
}