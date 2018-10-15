// Adapted by D. Blyth from work by J. Blomer

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <ROOT/RDataFrame.hxx>
#include <ROOT/RSnapshotOptions.hxx>

void printUsage(char **argv) {
    std::cerr << "Usage: " << argv[0] << " [-a algorithm] [-l level] inputPath outputPath" << std::endl;
}

int main(int argc, char *argv[]) {
    std::string algorithm;
    int level = -1;

    int opt;
    while ((opt = getopt(argc, argv, "a:l:h")) != -1) {
        switch (opt) {
            case 'a':
                algorithm = optarg;
                break;
            case 'l':
                level = atoi(optarg);
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
    if (algorithm.compare("zlib") == 0)
        snapshotOptions.fCompressionAlgorithm = ROOT::kZLIB;
    else if (algorithm.compare("lzma") == 0)
        snapshotOptions.fCompressionAlgorithm = ROOT::kLZMA;
    else if (algorithm.compare("lz4") == 0)
        snapshotOptions.fCompressionAlgorithm = ROOT::kLZ4;
    else if (algorithm.compare("none") == 0)
        snapshotOptions.fCompressionLevel = 0;
    if (level >= 0) snapshotOptions.fCompressionLevel = level;
    ROOT::RDataFrame rdf("particles", inputPath);

    struct rusage usageBefore;
    std::memset(&usageBefore, 0, sizeof(usageBefore));
    struct rusage usageAfter;
    std::memset(&usageAfter, 0, sizeof(usageAfter));
    getrusage(RUSAGE_SELF, &usageBefore);

    auto nEvents = rdf.Snapshot("particles", outputPath, "", snapshotOptions)->Count();

    getrusage(RUSAGE_SELF, &usageAfter);
    struct timeval diff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &diff);

    struct stat buf;
    stat(outputPath.c_str(), &buf);

    std::cout << outputPath << ", " << buf.st_size << ", " << *nEvents / (diff.tv_sec + diff.tv_usec * 1e-6)
              << std::endl;

    exit(EXIT_SUCCESS);
}