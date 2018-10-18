// Adapted by D. Blyth from work by J. Blomer

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
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

    struct timespec procTimeBefore;
    struct timespec procTimeAfter;
    struct timespec monoTimeBefore;
    struct timespec monoTimeAfter;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeBefore);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeBefore);

    auto nEvents = rdf.Snapshot("particles", outputPath, "", snapshotOptions)->Count();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &procTimeAfter);
    clock_gettime(CLOCK_MONOTONIC, &monoTimeAfter);

    struct stat buf;
    stat(outputPath.c_str(), &buf);

    std::cout << outputPath << ", " << buf.st_size << ", "
              << *nEvents / double(procTimeAfter.tv_sec - procTimeBefore.tv_sec +
                                   (procTimeAfter.tv_nsec - procTimeBefore.tv_nsec) * 1e-9) << ", "
              << *nEvents / double(monoTimeAfter.tv_sec - monoTimeBefore.tv_sec +
                                   (monoTimeAfter.tv_nsec - monoTimeBefore.tv_nsec) * 1e-9) << std::endl;

    exit(EXIT_SUCCESS);
}
