#include <iostream>
#include <getopt.h>
#include <string>
#include <fstream>


using namespace std;

void printHelp() {
    cout << "Usage: gentxtsrccode [options] INPUTFILE.TXT\n"
         << "Options:\n"
         << "  --help             Display this help message\n"
         << "  --input <filename>     Specify input file\n"
         << "  --output <filename>    Specify output file\n"
         << "  --outputtype <type>    Specify output file type (C or CPP)\n"
         << "  --headerdir <dir>      Specify header file directory\n"
         << "  --sourcedir <dir>      Specify source file directory\n"
         << "  --namespace <name>     Specify namespace\n"
         << "  --signperline <num>    Specify number of characters per line\n";
}

void processLine(string line) {

}

void openFile(string filename) {
    ifstream file(filename);

    if (!file) {
        cout << "Failed to open file: " << filename << '\n';
        return;
    }

    string line;
    while (getline(file, line)) {
        processLine(line);
    }
    file.close();

    return;
}


int main(int argc, char *argv[]) {
    const char* const shortOpts = "h";
    const option longOpts[] = {
            {"help", no_argument, nullptr, 'h'},
            {"input", required_argument, nullptr, 0},
            {"output", required_argument, nullptr, 0},
            {"outputtype", required_argument, nullptr, 0},
            {"headerdir", required_argument, nullptr, 0},
            {"sourcedir", required_argument, nullptr, 0},
            {"namespace", required_argument, nullptr, 0},
            {"signperline", required_argument, nullptr, 0},
            {nullptr, no_argument, nullptr, 0}
    };

    int opt = 0;
    int longIndex = 0;

    while ((opt = getopt_long(argc, argv, shortOpts, longOpts, &longIndex)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 0:
                // Long option
                if (std::string(longOpts[longIndex].name) == "input") {
                    // Handle --input option
                    std::cout << "Input file: " << optarg << '\n';
                } else if (std::string(longOpts[longIndex].name) == "output") {
                    // Handle --output option
                    std::cout << "Output file: " << optarg << '\n';
                } else if (std::string(longOpts[longIndex].name) == "outputtype") {
                    // Handle --outputtype option
                    std::cout << "Output file type: " << optarg << '\n';
                } else if (std::string(longOpts[longIndex].name) == "headerdir") {
                    // Handle --headerdir option
                    std::cout << "Header directory: " << optarg << '\n';
                } else if (std::string(longOpts[longIndex].name) == "sourcedir") {
                    // Handle --sourcedir option
                    std::cout << "Source directory: " << optarg << '\n';
                } else if (std::string(longOpts[longIndex].name) == "namespace") {
                    // Handle --namespace option
                    std::cout << "Namespace: " << optarg << '\n';
                } else if (std::string(longOpts[longIndex].name) == "signperline") {
                    // Handle --signperline option
                    std::cout << "Characters per line: " << optarg << '\n';
                }
                break;
            default:
                // Invalid option
                return 1;
        }
    }

    // Process remaining arguments (files)
    for (int i = optind; i < argc; i++) {
        std::cout << "File: " << argv[i] << '\n';
    }

    return 0;
}
