// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#include <TFile.h>
#include <TTree.h>

void printUsage(char **argv) { std::cerr << "Usage: " << argv[0] << " inputPath" << std::endl; }

int main(int argc, char *argv[]) {
    std::string algorithm;
    int level = -1;

    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            default:
                printUsage(argv);
                exit(EXIT_FAILURE);
        }
    }

    std::string inputPath;
    if (optind == argc - 1) {
        inputPath = argv[optind];
    } else {
        printUsage(argv);
        exit(EXIT_FAILURE);
    }

    auto branchBuf = new unsigned char[0x1000000];

    struct rusage usageBefore;
    memset(&usageBefore, 0, sizeof(usageBefore));
    struct rusage usageAfter;
    memset(&usageAfter, 0, sizeof(usageAfter));
    getrusage(RUSAGE_SELF, &usageBefore);

    TFile file(inputPath.c_str());
    TTree *tree = (TTree *)file.Get("particles");
    auto branchList = tree->GetListOfBranches();
    for (int i = 0; i < branchList->GetEntries(); i++)
        ((TBranch *)branchList->At(i))->SetAddress(branchBuf + i * 0x100000);

    int nEvents = tree->GetEntries();
    for (int i = 0; i < nEvents; i++) {
        tree->GetEntry(i, 1);
    }

    getrusage(RUSAGE_SELF, &usageAfter);
    struct timeval diff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &diff);

    delete branchBuf;
    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", " << nEvents / (diff.tv_sec + diff.tv_usec * 1e-6)
              << std::endl;

    exit(EXIT_SUCCESS);
}
