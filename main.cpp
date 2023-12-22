#include "lib.h"
#include "CLI/CLI.hpp"

int32_t THREAD_COUNT = 1;

int main(int argc, char** argv) {
    CLI::App app{"Word Frequency Counter"};

    static std::string inputFileName;
    static std::string outputFileName;
    bool multithreaded = false;

    app.add_option("-i,--input", inputFileName, "Input file name")->required();
    app.add_option("-o,--output", outputFileName, "Output file name")->required();
    app.add_flag("--multithreaded", multithreaded, "Enable multithreading solution");

    CLI11_PARSE(app, argc, argv);

    if (multithreaded == true) { // TODO remove
        THREAD_COUNT = 20;
    }

    std::vector<Borders> threadBorders = CountThreadBorders(inputFileName);
    std::vector<std::thread> threads;
    WordCounterContainer container(threadBorders.size());

    for (int16_t i = 0; i < threadBorders.size(); ++i) {
        Borders& currentBorders = threadBorders[i];

        if (currentBorders.end <= currentBorders.start) {
            continue;
        }

        threads.emplace_back(
            ProcessChunk,
            std::ref(inputFileName), i, std::ref(currentBorders), std::ref(container)
        );
    }

    for (auto& t : threads) {
        t.join();
    }

    WordCounter globalWordCounter;
    for (const auto& wordCounter: container) {
        globalWordCounter.Absorb(*wordCounter);
    }

    FrequencyVector frequencyVector(globalWordCounter);
    frequencyVector.sort();

    std::ofstream outputFile(outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open the output file." << std::endl;
        return 1;
    }
    frequencyVector.print(outputFile);
}

size_t GetFileSize(const std::string& inputFileName) {
    std::ifstream initialFile(inputFileName);
    if (!initialFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 0;
    }

    initialFile.seekg(0, std::ios::end);
    size_t fileSize = initialFile.tellg();
    initialFile.close();
    return fileSize;
}

std::vector<Borders> CountThreadBorders(const std::string& inputFileName) {
    size_t fileSize = GetFileSize(inputFileName);

    int64_t chunkSize = fileSize / THREAD_COUNT;
    std::vector<Borders> threadBorders;
    threadBorders.reserve(THREAD_COUNT);

    for (int16_t i = 0; i < THREAD_COUNT; ++i) {
        Borders currentBorder;
        currentBorder.start = i * chunkSize;
        currentBorder.end = (i == THREAD_COUNT - 1) ? fileSize : (currentBorder.start + chunkSize);

        if (i != 0) {
            if (!TrySetToNearestSpace(inputFileName, fileSize, &currentBorder.start)) {
                currentBorder.start = fileSize;
            }
        }
        if (currentBorder.end != fileSize) {
            if (!TrySetToNearestSpace(inputFileName, fileSize, &currentBorder.end)) {
                currentBorder.end = fileSize;
            }
        }

        if (currentBorder.start >= currentBorder.end) {
            continue;
        }
        threadBorders.emplace_back(currentBorder);
    }

    return threadBorders;
}

void ProcessChunk(const std::string& inputFileName, int16_t index, Borders& borders, WordCounterContainer& container) {
    std::ifstream file(inputFileName);
    file.seekg(borders.start);

    std::unique_ptr<WordCounter> wordCounter = std::make_unique<WordCounter>();
    std::string word;
    while (file >> word) {
        if (file.tellg() >= borders.end) {
            break;
        }
        wordCounter->Count(word);
    }
    file.close();

    container.setWordCounter(index, std::move(wordCounter));
}

bool TrySetToNearestSpace(const std::string& inputFileName, const size_t fileSize, size_t* position) {
    std::ifstream file(inputFileName);
    file.seekg(*position);

    char c;
    while (file.get(c) && c != ' ' && file.tellg() < fileSize) {
        // pass
    }

    if (file.tellg() >= fileSize || c != ' ') {
        file.close();
        return false;
    }

    *position = file.tellg();
    file.close();
    return true;
}

