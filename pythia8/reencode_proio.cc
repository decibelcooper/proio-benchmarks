// Adapted by D. Blyth from work by J. Blomer

#include <getopt.h>
#include <stdio.h>

#include <proio/reader.h>
#include <proio/writer.h>

void printUsage(char **argv) {
    std::cerr << "Usage: " << argv[0] << " [-a algorithm] inputPath outputPath" << std::endl;
}

int main(int argc, char *argv[]) {
    std::string algorithm;

    int opt;
    while ((opt = getopt(argc, argv, "a:l:h")) != -1) {
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
    if (optind < argc - 1) {
        inputPath = argv[optind];
        outputPath = argv[optind + 1];
    } else {
        printUsage(argv);
        exit(EXIT_FAILURE);
    }

    auto reader = new proio::Reader(inputPath);
    auto writer = new proio::Writer(outputPath);
    if (algorithm.compare("gzip") == 0)
        writer->SetCompression(proio::GZIP);
    else if (algorithm.compare("lz4") == 0)
        writer->SetCompression(proio::LZ4);
    else if (algorithm.compare("none") == 0)
        writer->SetCompression(proio::UNCOMPRESSED);
    auto event = new proio::Event();
    while (true) {
        if (!reader->Next(event)) break;
        writer->Push(event);
    }
    delete event;
    delete writer;
    delete reader;

    exit(EXIT_SUCCESS);
}
