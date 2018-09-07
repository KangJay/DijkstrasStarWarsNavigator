#include <iostream> 
#include <fstream> 
#include <string> 
#include <map>
#include <utility>
#include "galaxy.h"

using namespace std;

void printStats(string plan1, string plan2, int hours) {
    cout << "START: " << plan1 << ", END: " << plan2 << ", TIME: " << hours << endl;
}

int main(int argc, char* argv[]) {
        if (argc != 3){
		exit(EXIT_FAILURE); 
	}
	ifstream inFile(argv[1]);
	ifstream flights(argv[2]);
	//ifstream inFile("conduits.txt");
	//ifstream flights("ship_routes.txt");
	//ifstream flights("asd.txt");
	Reader read(inFile, flights);
	Galaxy* starWars = read.load();
    //starWars->dump();
	starWars->search();
    return 0;
}
