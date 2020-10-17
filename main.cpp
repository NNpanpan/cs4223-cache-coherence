#include <bits/stdc++.h>
#include "src/Bus.h" ///test include subdir header

using namespace std;

typedef pair<int, int> CoreCmd;

vector<vector<CoreCmd>> readBenchmark(string benchmark) {
    ///assuming execute from project root directory
    int numberOfCores = 4;
    vector<vector<CoreCmd>> coreCmd(4, vector<CoreCmd>());

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
        int cmdType, addr;
        while (fscanf(file, "%d %x", &cmdType, &addr) != EOF) {
            coreCmd[core].push_back(CoreCmd(cmdType, addr));
        }

        fclose(file);
    }

    return coreCmd;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << ("Expect argument of form: protocol benchmark cache_size associativity block_size");
        exit(-1);
    }
    string protocolName(argv[1]);
    string benchmark(argv[2]);

    int cacheSize = strtol(argv[3], NULL, 0);
    int assoc = strtol(argv[4], NULL, 0);
    int blockSize = strtol(argv[5], NULL, 0);

    vector<vector<CoreCmd>> coreCmd = readBenchmark(benchmark);

}
