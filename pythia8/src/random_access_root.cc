// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
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
    struct rusage usageAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    getrusage(RUSAGE_SELF, &usageBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

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
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);
    struct timeval udiff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &udiff);

    delete file;
    delete branchBuf;
    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", "
              << nEvents / double(udiff.tv_sec + udiff.tv_usec * 1e-6) << ", "
              << nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                  (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9) << std::endl;

    exit(EXIT_SUCCESS);
}
