#ifdef _WIN32

#include <Windows.h>

double get_wall_time() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return 0;
    }
    if (!QueryPerformanceCounter(&time)) {
        return 0;
    }
    return (double) time.QuadPart / freq.QuadPart;
}

double get_cpu_time() {
    FILETIME a, b, c, d;
    if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
        return (double) (d.dwLowDateTime | ((unsigned long long) d.dwHighDateTime << 32)) * 0.0000001;
    } else {
        return 0;
    }
}

#else
#include <time.h>
#include <sys/time.h>
double get_wall_time(){
	struct timeval time;
	if (gettimeofday(&time,NULL)){
		//  Handle error
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
	return (double)clock() / CLOCKS_PER_SEC;
}
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "KB.h"

using namespace std;

int main() {
    cout << "Working..." << endl;

    // Record start time
    double startTime = get_wall_time();

    // Initialize variables
    int numQueries = 0, numSentences = 0;
    Knowledgebase knowledgeBase;
    string tempString;
    vector<string> queries;

    // Read input file
    ifstream inputFile("input.txt");
    if (inputFile.is_open()) {
        getline(inputFile, tempString);
        numQueries = stoi(tempString);

        queries = vector<string>((unsigned) numQueries);
        for (int i = 0; i < numQueries; i++) {
            getline(inputFile, tempString);
            queries[i] = tempString;
        }

        getline(inputFile, tempString);
        numSentences = stoi(tempString);

        for (int i = 0; i < numSentences; i++) {
            getline(inputFile, tempString);
            knowledgeBase.tell(tempString);
        }
        inputFile.close();
    } else {
        cout << "Input file failed to load" << endl;
    }

    // Process queries
    string output = "";
    for (int i = 0; i < numQueries; i++) {
        bool result = knowledgeBase.ask(queries[i]);
        output += result ? "VALID" : "INVALID";
        output += "\n";
    }

    // Write output file
    ofstream outputFile("output.txt");
    if (outputFile.is_open()) {
        outputFile << output;
        cout << "Done" << endl;
        outputFile.close();
    } else {
        cout << "Output file failed to load" << endl;
    }

    // Record end time (for efficiency testing)
    double endTime = get_wall_time();

    return 0;
}