// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <proio/model/example/example.pb.h>
#include <proio/reader.h>
#include <proio/writer.h>

void printUsage(char **argv) { std::cerr << "Usage: " << argv[0] << " inputPath" << std::endl; }

int main(int argc, char *argv[]) {
    std::string algorithm;

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

    auto event = new proio::Event();
    int nEvents = 0;

    struct timespec procTimeBefore;
    struct timespec procTimeAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    auto reader = new proio::Reader(inputPath);
    while (true) {
        if (!reader->Next(event)) break;

        for (auto entryID : event->AllEntries()) event->GetEntry(entryID);

        nEvents++;
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);

    delete event;
    delete reader;

    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", "
              << nEvents / double(procTimeAfter.tv_sec - procTimeBefore.tv_sec +
                                  (procTimeAfter.tv_nsec - procTimeBefore.tv_nsec) * 1e-9) << ", "
              << nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                  (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9) << std::endl;

    exit(EXIT_SUCCESS);
}
