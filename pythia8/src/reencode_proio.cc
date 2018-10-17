// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <proio/reader.h>
#include <proio/writer.h>

void printUsage(char **argv) {
    std::cerr << "Usage: " << argv[0] << " [-a algorithm] inputPath outputPath" << std::endl;
}

int main(int argc, char *argv[]) {
    std::string algorithm;

    int opt;
    while ((opt = getopt(argc, argv, "a:h")) != -1) {
        switch (opt) {
            case 'a':
                algorithm = optarg;
                break;
            default:
                printUsage(argv);
                exit(EXIT_FAILURE);
        }
    }

    std::string inputPath;
    std::string outputPath;
    if (optind == argc - 2) {
        inputPath = argv[optind];
        outputPath = argv[optind + 1];
    } else {
        printUsage(argv);
        exit(EXIT_FAILURE);
    }

    struct timespec procTimeBefore;
    struct timespec procTimeAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    struct rusage usageBefore;
    std::memset(&usageBefore, 0, sizeof(usageBefore));
    struct rusage usageAfter;
    std::memset(&usageAfter, 0, sizeof(usageAfter));
    getrusage(RUSAGE_SELF, &usageBefore);

    while (true) {
        if (!reader->Next(event)) break;
        writer->Push(event);
        nEvents++;
    }
    writer->Flush();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);

    delete event;
    delete writer;
    delete reader;

    struct stat buf;
    stat(outputPath.c_str(), &buf);

    std::cout << outputPath << ", " << buf.st_size << ", "
              << nEvents / double(procTimeAfter.tv_sec - procTimeBefore.tv_sec +
                                  (procTimeAfter.tv_nsec - procTimeBefore.tv_nsec) * 1e-9)
              << ", "
              << nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                  (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9)
              << std::endl;

    exit(EXIT_SUCCESS);
}
