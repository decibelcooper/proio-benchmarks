// Adapted by D. Blyth from work by J. Blomer

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <ROOT/RDataFrame.hxx>
#include <ROOT/RSnapshotOptions.hxx>

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

    ROOT::RDF::RSnapshotOptions snapshotOptions;
    // snapshotOptions.fAutoFlush = 1000;
    if (algorithm.compare("zlib") == 0) {
        snapshotOptions.fCompressionAlgorithm = ROOT::kZLIB;
        snapshotOptions.fCompressionLevel = 7;
    } else if (algorithm.compare("lzma") == 0) {
        snapshotOptions.fCompressionAlgorithm = ROOT::kLZMA;
        snapshotOptions.fCompressionLevel = 9;
    } else if (algorithm.compare("lz4") == 0) {
        snapshotOptions.fCompressionAlgorithm = ROOT::kLZ4;
        snapshotOptions.fCompressionLevel = 9;
    } else if (algorithm.compare("none") == 0)
        snapshotOptions.fCompressionLevel = 0;
    ROOT::RDataFrame rdf("particles", inputPath);

    struct rusage usageBefore;
    struct rusage usageAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    getrusage(RUSAGE_SELF, &usageBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    auto nEvents = rdf.Snapshot("particles", outputPath, "", snapshotOptions)->Count();

    getrusage(RUSAGE_SELF, &usageAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);
    struct timeval udiff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &udiff);

    struct stat buf;
    stat(outputPath.c_str(), &buf);

    std::cout << outputPath << ", " << buf.st_size << ", "
              << *nEvents / double(udiff.tv_sec + udiff.tv_usec * 1e-6) << ", "
              << *nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                   (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9) << std::endl;

    exit(EXIT_SUCCESS);
}
