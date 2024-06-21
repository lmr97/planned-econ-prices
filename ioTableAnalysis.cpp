// This is the main driver for the project

#include "ioTableAnalysis.hpp"
using namespace std;


// This function checks whether two numbers are equal within the given precision
// (number of decimal points)
bool precisionReached(unordered_map<long int, double>* prevIterPrices,
                      unordered_map<long int, double>* currIterPrices,
                      const int precision)
{
    if (currIterPrices->empty()) return false;

    double precisionUnit = pow(10, -precision); 
    for (const auto &[productKey, price] : *currIterPrices)
    {
        if (abs(price - prevIterPrices->at(productKey)) > precisionUnit) return false;
    }

    return true;
}


// These functions, calcPricesConstIter (1) and calcPricesPrec (2) calculate prices, 
// and return a reference to the prices unordered_map.
// They follow Cockshott and Cottrell's algorithm as laid out in Chapter 3 of
// Toward a New Socialism (1993), but have different stopping points.

// (1) This implementation stops after a given number of iterations has bee reached
unordered_map<long int, double>* calcPricesConstIter(unordered_map<ProdInputPair,double>* ioTable,
                                                     const int iterations)
{
    // initialize previous (in this case, initial) iteration prices list,
    // using only direct labor
    auto laborOnlyPrices = new unordered_map<long int, double>;
    auto prevIterPrices = new unordered_map<long int, double>;
    auto currIterPrices = new unordered_map<long int, double>;
    ProdInputPair PIpair{0,0};
    long int currentProduct{0};

    for (auto iter = ioTable->begin(); iter != ioTable->end(); ++iter)
    {   
        if (iter->first.product != currentProduct)                    // to prevent redundant statements
        {
            currentProduct = iter->first.product;
            PIpair.product = currentProduct;
            PIpair.input = 0;

            (*laborOnlyPrices)[currentProduct] = ioTable->at(PIpair);  // start with the labor value...

            // ...and divide it by quantity produced of that product
            PIpair.input = 1;
            (*laborOnlyPrices)[currentProduct] /= ioTable->at(PIpair);
        }
    }


    // constant-iteration algorithm
    // if iterations==0, this loop and following conditional will be 
    // skipped and will go to the precision-based iterations.
    cout << "\nNow running iterations." << endl;

    *prevIterPrices = *laborOnlyPrices;
    *currIterPrices = *laborOnlyPrices;
    for (int i = 0; i < iterations; i++)
    {
        for (auto iter = ioTable->begin(); iter != ioTable->end(); ++iter)
        {
            PIpair = iter->first;               // key loaded into local variable for speed

            // no need to consider the output quantity column (irrelevant)
            // or labor column (already accounted for)
            if(PIpair.input == 1 || PIpair.input == 0) continue;     
            
            // add prevIterPrices cost of the input to the current product's total cost, 
            // and divide by total units produced to get the per-unit cost of that input
            double priceAddition = ioTable->at(PIpair) * prevIterPrices->at(PIpair.input);
            PIpair.input = 1;
            priceAddition       /= ioTable->at(PIpair);

            currIterPrices->at(PIpair.product) += priceAddition;
        }

        *prevIterPrices = *currIterPrices;  // save last iteration's prices...
        *currIterPrices = *laborOnlyPrices; // ...and start fresh for current iteration

        cout << "iteration " << i+1 << " of " << iterations << " complete" << endl;
    }

    delete prevIterPrices;
    delete laborOnlyPrices;

    return currIterPrices;
}


// (2) This implementation stops after a certain precision has been reached
unordered_map<long int, double>* calcPricesPrec(unordered_map<ProdInputPair,double>* ioTable,
                                                const int precision)
{
    // initialize previous (in this case, initial) iteration prices list,
    // using only direct labor
    auto laborOnlyPrices = new unordered_map<long int, double>;
    auto prevIterPrices  = new unordered_map<long int, double>;
    auto currIterPrices  = new unordered_map<long int, double>;
    ProdInputPair PIpair{0,0};
    long int currentProduct{0};

    for (auto iter = ioTable->begin(); iter != ioTable->end(); ++iter)
    {   
        if (iter->first.product != currentProduct)                    // to prevent redundant statements
        {
            currentProduct = iter->first.product;
            PIpair.product = currentProduct;
            PIpair.input = 0;

            (*laborOnlyPrices)[currentProduct] = ioTable->at(PIpair);  // start with the labor value...

            // ...and divide it by quantity produced of that product
            PIpair.input = 1;
            (*laborOnlyPrices)[currentProduct] /= ioTable->at(PIpair);
        }
    }


    // precision-based algorithm
    cout << "Now iterating until precision == " << precision << endl;
    int iterCounter{1};

    *currIterPrices = *laborOnlyPrices;
    do 
    {
        auto start = chrono::high_resolution_clock::now();

        *prevIterPrices = *currIterPrices;  // save last iteration's prices...
        *currIterPrices = *laborOnlyPrices; // ...and start fresh for current iteration


        for (auto iter = ioTable->begin(); iter != ioTable->end(); ++iter)
        {
            PIpair = iter->first;               // key loaded into local variable for speed

            // no need to consider the output quantity column (irrelevant)
            // or labor column (already accounted for)
            if(PIpair.input == 1 || PIpair.input == 0) continue;     
            
            // add prevIterPrices cost of the input to the current product's total cost, 
            // and divide by total units produced to get the per-unit cost of that input
            double priceAddition = ioTable->at(PIpair) * prevIterPrices->at(PIpair.input);
            PIpair.input = 1;
            priceAddition       /= ioTable->at(PIpair);

            currIterPrices->at(PIpair.product) += priceAddition;
        }

        //printPrices(currIterPrices);
        cout << "iteration " << iterCounter << " complete" << endl;
        iterCounter++;

        auto stop = chrono::high_resolution_clock::now();
        auto timeTaken = chrono::duration_cast<chrono::milliseconds>(stop-start);
        cout << "completed in " << setprecision(4) << timeTaken.count()/1000.0 << " seconds" << endl;
    }
    while(!precisionReached(prevIterPrices, currIterPrices, precision));

    delete prevIterPrices;
    delete laborOnlyPrices;

    return currIterPrices;
}


// main can take the location of the .txt file
int main(int argc, char* argv[])
{
    auto start = chrono::high_resolution_clock::now();

    // get hash table of command line arguments
    unordered_map<string, char*> cla;
    string errorMsg = parseCmdOptions(argc, argv, cla);

    // exit the program when -h selected, since help already printed
    if (errorMsg == "help") return 0; 

    // crash if there were CLI errors
    try
    {
        if (errorMsg != "") throw invalid_argument(errorMsg);
    }
    catch (const invalid_argument& ia)
    {
        cerr << ia.what() << endl;
        return 0;
    }

    int precision{0};
    int iterations{0};
    if (cla["precision"])  precision  = atoi(cla["precision"]);
    if (cla["iterations"]) iterations = atoi(cla["iterations"]); 

    auto ioTable = new unordered_map<ProdInputPair,double>;
    bool ioTableLoaded = loadIOTable(cla["file"], ioTable);
    try
    {
        if (!ioTableLoaded) throw invalid_argument("FILE ERROR: invalid file");
    }
    catch (const invalid_argument& ia)
    {
        cerr << ia.what() << endl;
        delete ioTable;
        return 0;
    }


    unordered_map<long int, double>* prices;
    if (precision)  prices = calcPricesPrec(ioTable, precision);
    if (iterations) prices = calcPricesConstIter(ioTable, iterations);
    //printPrices(prices);

    if (cla["outputFile"]) savePricesToFile(prices, cla["outputFile"]);
    else printPrices(prices);
    
    delete prices;
    delete ioTable;

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop-start);

    cout << "\nTime taken (seconds): " << duration.count()/1000.0 << endl << endl;

    return 0;
}