// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
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
    auto *file = new TFile(inputPath.c_str());
    TTree *tree = (TTree *)file->Get("particles");
    long maxNEvents = tree->GetEntries();
    delete file;
    int nEvents = 0;

    struct rusage usageBefore;
    memset(&usageBefore, 0, sizeof(usageBefore));
    struct rusage usageAfter;
    memset(&usageAfter, 0, sizeof(usageAfter));
    getrusage(RUSAGE_SELF, &usageBefore);

    file = new TFile(inputPath.c_str());
    tree = (TTree *)file->Get("particles");
    auto branchList = tree->GetListOfBranches();
    for (int i = 0; i < branchList->GetEntries(); i++)
        ((TBranch *)branchList->At(i))->SetAddress(branchBuf + i * 0x100000);

    while (true) {
        tree->GetEntry(long(maxNEvents * rand() / double(RAND_MAX)), 1);
        nEvents++;
        if (nEvents == 100) break;
    }

    getrusage(RUSAGE_SELF, &usageAfter);
    struct timeval udiff;
    struct timeval sdiff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &udiff);
    timersub(&usageAfter.ru_stime, &usageBefore.ru_stime, &sdiff);
    struct timeval total;
    timeradd(&udiff, &sdiff, &total);

    delete file;
    delete branchBuf;
    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", " << nEvents / (total.tv_sec + total.tv_usec * 1e-6)
              << std::endl;

    exit(EXIT_SUCCESS);
}
