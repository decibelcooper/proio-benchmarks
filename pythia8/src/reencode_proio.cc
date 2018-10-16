// By D. Blyth

#include <getopt.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
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
        writer->SetCompression(proio::GZIP, 9);
    else if (algorithm.compare("lz4") == 0)
        writer->SetCompression(proio::LZ4, 9);
    else if (algorithm.compare("none") == 0)
        writer->SetCompression(proio::UNCOMPRESSED);
    auto event = new proio::Event();
    int nEvents = 0;

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

    getrusage(RUSAGE_SELF, &usageAfter);
    struct timeval diff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &diff);

    delete event;
    delete writer;
    delete reader;

    struct stat buf;
    stat(outputPath.c_str(), &buf);

    std::cout << outputPath << ", " << buf.st_size << ", " << nEvents / (diff.tv_sec + diff.tv_usec * 1e-6)
              << std::endl;

    exit(EXIT_SUCCESS);
}
