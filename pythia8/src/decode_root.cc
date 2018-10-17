// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
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

    struct timespec procTimeBefore;
    struct timespec procTimeAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    TFile file(inputPath.c_str());
    TTree *tree = (TTree *)file.Get("particles");
    auto branchList = tree->GetListOfBranches();
    for (int i = 0; i < branchList->GetEntries(); i++)
        ((TBranch *)branchList->At(i))->SetAddress(branchBuf + i * 0x100000);

    int nEvents = tree->GetEntries();
    for (int i = 0; i < nEvents; i++) {
        tree->GetEntry(i, 1);
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);

    delete branchBuf;
    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", "
              << nEvents / double(procTimeAfter.tv_sec - procTimeBefore.tv_sec +
                                  (procTimeAfter.tv_nsec - procTimeBefore.tv_nsec) * 1e-9)
              << nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                  (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9)
              << std::endl;

    exit(EXIT_SUCCESS);
}
