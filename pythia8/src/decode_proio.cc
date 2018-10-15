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

    struct rusage usageBefore;
    std::memset(&usageBefore, 0, sizeof(usageBefore));
    struct rusage usageAfter;
    std::memset(&usageAfter, 0, sizeof(usageAfter));
    getrusage(RUSAGE_SELF, &usageBefore);

    auto reader = new proio::Reader(inputPath);
    while (true) {
        if (!reader->Next(event)) break;

        for (auto entryID : event->AllEntries()) event->GetEntry(entryID);

        nEvents++;
    }

    getrusage(RUSAGE_SELF, &usageAfter);
    struct timeval diff;
    timersub(&usageAfter.ru_utime, &usageBefore.ru_utime, &diff);

    delete event;
    delete reader;

    struct stat buf;
    stat(inputPath.c_str(), &buf);

    std::cout << inputPath << ", " << buf.st_size << ", " << nEvents / (diff.tv_sec + diff.tv_usec * 1e-6)
              << std::endl;

    exit(EXIT_SUCCESS);
}
