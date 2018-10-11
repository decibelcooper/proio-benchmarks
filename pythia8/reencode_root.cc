// Adapted by D. Blyth from work by J. Blomer

#include <getopt.h>
#include <stdio.h>

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
    if (optind < argc - 1) {
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
    if (level >= 0) snapshotOptions.fCompressionLevel = level;
    ROOT::RDataFrame rdf("particles", inputPath);
    rdf.Snapshot("particles", outputPath, "", snapshotOptions);

    exit(EXIT_SUCCESS);
}
