// header file with class definitions, utility functions, and other #includes

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unistd.h>
#include <stdexcept>
#include <chrono>
using namespace std;

const int PRECISION_MAX = 15;   // long type chosen to avoid error on cast to char*

/*///////////////////////
       CLASSES
///////////////////////*/


// This object stores an ordered pair of products,
// with the first being the UPC of product being produced, 
// and the second being the UPC of the product being used as input,
// except when ProdInputPair.input==1, which refers to the quantity 
// of the product produced. 
// It also overloads the < operator so that that comparison is
// defined for the object (it is used for insertion into the std::unordered)
class ProdInputPair
{
    public:
        long int product;
        long int input;    // the production input, unless it's the output column (input==1)
        
        // requires < overload so that key uniqueness can be checked
        bool operator<(const ProdInputPair &other) const
        {
            if (product <  other.product) return true;
            if (product >  other.product) return false;
            if (product == other.product) 
            {
                if (input <  other.input) return true;
                if (input >  other.input) return false;
                if (input == other.input) return false;
            }

            // if somehow none of the above conditionals run, 
            // then default to true (a difference is assumed)
            return true;   
        }

        bool operator==(const ProdInputPair &other) const
        {
            if (product == other.product && input == other.input) return true;
            return false;
        }
};


// this defines hashing for the ProductInputPair class,
// simply hashing the sum of the product and input attributes
// 
// The structure of this section comes from this page in 
// the C++ documentation: https://en.cppreference.com/w/cpp/utility/hash/operator()
namespace std
{
    template <>
    class hash<ProdInputPair>
    {
        public:
            uint64_t operator()(const ProdInputPair &pip) const
            {
                return hash<long>{}(pip.product + pip.input);
            }
    };
}





/*///////////////////////
    UTILITY FUNCTIONS
///////////////////////*/


void printHelp(char* executableName)
{
    cout << "\nUsage: " << executableName << " -f input_file_path {-i iterations | -p precision} [-o output_file]" << endl << endl;
    cout << "Options:" << endl << endl;
    cout << "    -f file_path         <required> Path to a .txt file containing the input-output table, " << endl;
    cout << "                         with each line containing the UPC of the output, a comma" << endl;
    cout << "                         (no space), the UPC of the input, a space, then the quantity" << endl;
    cout << "                         of the input for the production of the product with the line's" << endl;
    cout << "                         first UPC. For example:" << endl << endl;
    cout << "                         \t101010282293,882872662923 239.7" << endl << endl;
    cout << "                         This line encodes the fact that 239.7 units of UPC 882872662923" << endl;
    cout << "                         were used in the production of UPC 101010282293 during the table's" << endl;
    cout << "                         production timeframe. " << endl;
    cout << "                         The only exceptions to this are when the UPC in the second position" << endl;
    cout << "                         is 0 or 1: when it's 0, the rightmost number is the person-hours" << endl;
    cout << "                         used in the production of the product whose UPC is first, and" << endl;
    cout << "                         when it's 1, the right-most number is the number of units" << endl;
    cout << "                         produced over the production period. " << endl << endl;
    cout << "    -i iterations        [optional if -p given] The number of iterations the alogorithm will run. " << endl << endl;
    cout << "    -p precision         [optional if -i given] The precision at which the algorithm is to stop" << endl; 
    cout << "                         iterating, given as the number of decimal digits to the right of the" << endl;
    cout << "                         decimal point. " << endl << endl;
    cout << "    -o output_file       [optional] Path to a .csv file where the calculated prices are to be " << endl;
    cout << "                         saved to. " << endl << endl;
    cout << "    -h                   Print this list of options. " << endl << endl;
}


string parseCmdOptions(const int argc, 
                       char* *argv,
                       unordered_map<string, char*> &arguments)
{
    string msg{""};
    arguments["file"] = nullptr;
    arguments["iterations"] = nullptr;
    arguments["precision"] = nullptr;
    arguments["outputFile"] = nullptr;

    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
                case 'f':
                    arguments["file"] = argv[i+1];
                    break;
                case 'i':
                    arguments["iterations"] = argv[i+1];
                    break;
                case 'p': 
                    arguments["precision"] = argv[i+1];
                    break;
                case 'o':
                    arguments["outputFile"] = argv[i+1];
                    break;
                case 'h':
                    printHelp(argv[0]);     
                    return "help";           // no need to parse further, just print help and quit
                case '-':                    // '--' denotes end of arguments (no long args here)
                    goto Process_args;
                default:
                    msg = "\nINPUT ERROR: invalid command line option \'";
                    msg.append((string)argv[i]);
                    msg.append("\'\nUse -h to show help.\n");
                    return msg;
            }
        }
    }

    Process_args:
    if (!(arguments["iterations"]) && !(arguments["precision"]))
    {
        msg = "\nINPUT ERROR: either iterations or precision must be specified (with -i or -p respecively)\n";
    }

    // you can have up to 6 decimal digits of precision. I doubt more is needed,
    // so the program defaults to 6 if more is selected

    if (arguments["precision"])
    {
        if (atoi(arguments["precision"]) > PRECISION_MAX)
        {
            cout << "\nMaximum precision is " << PRECISION_MAX;
            cout << " decimal places, resetting precision to " << PRECISION_MAX << "." << endl;
            string precMax = to_string(PRECISION_MAX);
            arguments["precision"] = &precMax[0];  // non-const conversion to char*
        }
    }

    return msg;
};



// load the input-output table into an instance of std::unordered_map
bool loadIOTable(const char* fileLoc, 
                 unordered_map<ProdInputPair,double>& ioTable)
{
    ifstream fin(fileLoc, ios::in);
    ProdInputPair PIpair{0,0};
    string file_line{""}; 
    double ioQuant{0};               // the actual input or output value

    if (!fin.good()) return false;

    cout << "\rLoading data..." << endl;
    while (!fin.eof())
    {
        getline(fin, file_line, '\n');

        if (file_line != "")
        {
            PIpair.product = stol(file_line.substr(0, file_line.find(",")));
            PIpair.input = stol(file_line.substr(file_line.find(",")+1, file_line.find(" ")));

            ioQuant = stod(file_line.substr(file_line.find(" "), file_line.back()));
            
            ioTable[PIpair] = ioQuant;
        }
    }

    fin.close();
    return true;
}


// writes for CSV format (with header)
void savePricesToFile(unordered_map<long int, double>& prices, const char* outputFile)
{
    ofstream fout(outputFile, ios::out);
    cout << "\nSaving data..." << endl;

    try
    {
        if (!fout.good()) 
        {
            fout.close();
            throw std::invalid_argument("File cannot be written to.");
        }
    }
    catch (const std::invalid_argument& ia) 
    {
        cerr << ia.what() << endl << endl;
    }

    fout << "ProductUPC,Price" << endl;
    for (auto itr = prices.begin(); itr != prices.end(); ++itr)
    {
        fout << itr->first << "," << itr->second << endl;
    }
    
    cout << "Prices data saved to: " << outputFile << endl << endl;
}   


// for programmer validation that the data was properly retrieved 
// and returned to main(). Optional function.
void printIOtable(unordered_map<ProdInputPair,double> &ioTable)
{
    int counter{0};
    for (auto itr = ioTable.begin(); itr != ioTable.end(); ++itr)
    {
        if (itr->second)
        {
            counter++;
            cout << "Product: " << itr->first.product << endl;
            if (itr->first.input == 0) 
            {
                cout << "Labor:  " << itr->second << endl << endl;
                continue;
            }
            else if (itr->first.input == 1)
            {
                cout << "Output: " << itr->second << endl << endl;
                continue;
            }
            else 
            {
                cout << "Input: " << itr->first.input << endl;
                cout << "Quanitity: " << itr->second << endl << endl;
            }
        }
    }

    cout << "Value count: " << counter << endl;
}


void printKeys(unordered_map<ProdInputPair,double>* ioTable)
{
    int keyCount{1};
    for (auto iter = ioTable->begin(); iter != ioTable->end(); ++iter)
    {
        cout << "Key " << keyCount << endl;
        cout << "  Product:      " << iter->first.product << endl;
        cout << "  Input/output: " << iter->first.input << endl;
        keyCount++;
    }
}


void printPrices(unordered_map<long int,double>& prices)
{
    for (auto iter = prices.begin(); iter != prices.end(); ++iter)
    {
        cout << iter->first << ": " << iter->second << " lh/unit" << endl;
    }
}