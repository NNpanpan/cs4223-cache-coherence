#include <bits/stdc++.h>
#include "src/MESIRunner.h"
#include "src/DragonRunner.h"

using namespace std;

typedef pair<int, int> CoreOps;

vector<vector<CoreOps>> readBenchmark(string benchmark) {
    // Assumes execution from project root directory
    int numberOfCores = 4;
    vector<vector<CoreOps>> coreOps(4, vector<CoreOps>());

    string relPath = "benchmarks/" + benchmark + "_four/";

    for(int core = 0; core < numberOfCores; core++) {
        // Hard-code file path
        string fileName = benchmark + "_" + to_string(core) + ".data";
        string relFile = relPath + fileName;
        FILE* file = fopen(relFile.c_str(), "r");

        if (file == NULL) {
            // Try to locate file again if in sub-directory, e.g. /scripts
            relFile = "../" + relFile;
            file = fopen(relFile.c_str(), "r");
        }

        if (file == NULL) {
            cerr << "Trace file " << fileName << " does not exist!\n\n";
            exit(-1);
        }

        // Read line-by-line
        int opsType, addr;
        while (fscanf(file, "%d %x", &opsType, &addr) != EOF) {
            coreOps[core].push_back(CoreOps(opsType, addr));
        }

        fclose(file);
    }

    return coreOps;
}


void simulate(string protocol, int cacheSize, int assoc, int blockSize,
        vector<vector<CoreOps>> ops) {

    Runner* runner;
    if (protocol == "MESI" || protocol == "mesi") {
        runner = new MESIRunner(cacheSize, assoc, blockSize, ops);
    } else if (protocol == "Dragon" || protocol == "dragon") {
        runner = new DragonRunner(cacheSize, assoc, blockSize, ops);
    } else {
        cerr << "Unknown coherence protocol (should be MESI or Dragon): " << protocol << "\n\n";
        exit(-1);
    }

    runner->simulate();
    delete(runner);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << "Expect arguments in form: protocol benchmark cache_size associativity block_size\n\n";
        exit(-1);
    }

    string protocol(argv[1]);
    string benchmark(argv[2]);

    int cacheSize = strtol(argv[3], NULL, 0);
    int assoc = strtol(argv[4], NULL, 0);
    int blockSize = strtol(argv[5], NULL, 0);

    // Cache and block sizes should be multiples of word size (4 bytes)
    // This ensures word-aligned accesses
    assert(cacheSize > 0 && cacheSize % 4 == 0);
    assert(blockSize > 0 && blockSize % 4 == 0);

    // Associativity should divide cache into integer number of sets
    assert(assoc > 0 && cacheSize % (assoc * blockSize) == 0);

    vector<vector<CoreOps>> ops = readBenchmark(benchmark);

    simulate(protocol, cacheSize, assoc, blockSize, ops);
}

