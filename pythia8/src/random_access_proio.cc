// By D. Blyth

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cstdlib>

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
    auto reader = new proio::Reader(inputPath);
    long maxNEvents = reader->Skip(LONG_MAX);
    delete reader;
    int nEvents = 0;

    struct rusage usageBefore;
    struct rusage usageAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    getrusage(RUSAGE_SELF, &usageBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    reader = new proio::Reader(inputPath);
    while (true) {
        reader->Skip(long(maxNEvents * rand() / double(RAND_MAX)));
        reader->Next();
        nEvents++;
        if (nEvents == 100)
            break;
        else
            reader->SeekToStart();
    }

    getrusage(RUSAGE_SELF, &usageAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);
    struct timeval udiff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &udiff);

    delete event;
    delete reader;

    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", "
              << nEvents / double(udiff.tv_sec + udiff.tv_usec * 1e-6) << ", "
              << nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                  (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9) << std::endl;

    exit(EXIT_SUCCESS);
}
