#include "lib.h"

int main() {
    std::ifstream file("words.txt");
    std::string word;
    std::map<std::string, int64_t> wordToCount;

    while (file >> word) {
        ++wordToCount[word];
    }

    file.close();

    auto frequncyVector = FrequencyVector(wordToCount);
    frequncyVector.sort();
    frequncyVector.print();
}
