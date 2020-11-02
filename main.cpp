#include <bits/stdc++.h>
#include "src/MESIRunner.h"

using namespace std;

typedef pair<int, int> CoreOps;

vector<vector<CoreOps>> readBenchmark(string benchmark) {
    ///assuming execute from project root directory
    int numberOfCores = 4;
    vector<vector<CoreOps>> coreOps(4, vector<CoreOps>());

    string relPath = "benchmarks/" + benchmark + "_four/";


    for(int core = 0; core < numberOfCores; core++) {
        /// hard-code direction
        string fileName = benchmark + "_" + to_string(core) + ".data";
        string relFile = relPath + fileName;
        FILE* file = fopen(relFile.c_str(), "r");

        if (file == NULL) {
            cerr << "File not exist";
            exit(-1);
        }

        /// read line-by-line
        int opsType, addr;
        while (fscanf(file, "%d %x", &opsType, &addr) != EOF) {
            coreOps[core].push_back(CoreOps(opsType, addr));
        }

        fclose(file);
    }

    return coreOps;
}


void simulate(string protocol,
    int cacheSize, int assoc, int blockSize,
    vector<vector<CoreOps>> ops) {

    Runner* runner;
    if (protocol == "mesi")
        runner = new MESIRunner(cacheSize, assoc, blockSize, ops);
    runner->simulate();
    delete(runner);
}
int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << ("Expect argument of form: protocol benchmark cache_size associativity block_size");
        exit(-1);
    }
    string protocol(argv[1]);
    string benchmark(argv[2]);

    int cacheSize = strtol(argv[3], NULL, 0);
    int assoc = strtol(argv[4], NULL, 0);
    int blockSize = strtol(argv[5], NULL, 0);
    vector<vector<CoreOps>> ops = readBenchmark(benchmark);

    simulate(protocol, cacheSize, assoc, blockSize, ops);

    return 0;
}
