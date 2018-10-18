// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
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

    auto reader = new proio::Reader(inputPath);
    auto writer = new proio::Writer(outputPath);
    if (algorithm.compare("gzip") == 0)
        writer->SetCompression(proio::GZIP, 7);
    else if (algorithm.compare("lz4") == 0)
        writer->SetCompression(proio::LZ4, 9);
    else if (algorithm.compare("none") == 0)
        writer->SetCompression(proio::UNCOMPRESSED);
    auto event = new proio::Event();
    int nEvents = 0;

    struct rusage usageBefore;
    struct rusage usageAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    getrusage(RUSAGE_SELF, &usageBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    while (true) {
        if (!reader->Next(event)) break;
        writer->Push(event);
        nEvents++;
    }
    delete writer;

    getrusage(RUSAGE_SELF, &usageAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);
    struct timeval udiff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &udiff);

    delete event;
    delete reader;

    struct stat buf;
    stat(outputPath.c_str(), &buf);

    std::cout << outputPath << ", " << buf.st_size << ", "
              << nEvents / double(udiff.tv_sec + udiff.tv_usec * 1e-6) << ", "
              << nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                  (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9) << std::endl;

    exit(EXIT_SUCCESS);
}
