#include "lib.h"

static const int THREAD_COUNT = 20;
static const std::string FILENAME = "words.txt";

int main() {
    std::vector<Borders> threadBorders = CountThreadBorders();
    std::vector<std::thread> threads;
    WordCounterContainer container(threadBorders.size());

    for (int16_t i = 0; i < threadBorders.size(); ++i) {
        Borders& currentBorders = threadBorders[i];

        if (currentBorders.end <= currentBorders.start) {
            continue;
        }

        threads.emplace_back(
            ProcessChunk,
            i, std::ref(currentBorders), std::ref(container)
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
    frequencyVector.print();
}

size_t GetFileSize() {
    std::ifstream initialFile(FILENAME);
    if (!initialFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 0;
    }

    initialFile.seekg(0, std::ios::end);
    size_t fileSize = initialFile.tellg();
    initialFile.close();
    return fileSize;
}

std::vector<Borders> CountThreadBorders() {
    size_t fileSize = GetFileSize();

    int64_t chunkSize = fileSize / THREAD_COUNT;
    std::vector<Borders> threadBorders;
    threadBorders.reserve(THREAD_COUNT);

    for (int16_t i = 0; i < THREAD_COUNT; ++i) {
        Borders currentBorder;
        currentBorder.start = i * chunkSize;
        currentBorder.end = (i == THREAD_COUNT - 1) ? fileSize : (currentBorder.start + chunkSize);

        if (i != 0) {
            if (!TrySetToNearestSpace(fileSize, &currentBorder.start)) {
                currentBorder.start = fileSize;
            }
        }
        if (currentBorder.end != fileSize) {
            if (!TrySetToNearestSpace(fileSize, &currentBorder.end)) {
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

void ProcessChunk(int16_t index, Borders& borders, WordCounterContainer& container) {
    std::ifstream file(FILENAME);
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

bool TrySetToNearestSpace(const size_t fileSize, size_t* position) {
    std::ifstream file(FILENAME);
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

