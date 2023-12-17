#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

class FrequencyPair {
public:
    FrequencyPair(const std::string& word, const int64_t count)
        : word_(word)
        , count_(count)
    {}

    bool operator<(const FrequencyPair& other) const {
        if (count_ == other.count_) {
            return word_ < other.word_;
        }
        return count_ > other.count_;
    }

    void print() const {
        std::cout << count_ << ' ' << word_ << '\n';
    }

protected:
    int64_t count_ = 0;
    std::string word_ = "";
};

class FrequencyVector {
public:
    FrequencyVector(const std::map<std::string, int64_t> wordToCount) {
        data_.reserve(wordToCount.size());

        for (const auto& [word, count] : wordToCount) {
            data_.emplace_back(FrequencyPair(word, count));
        }
    }

    void sort() {
        std::sort(data_.begin(), data_.end());
    }

    void print() const {
        for (const FrequencyPair& frequencyPair : data_) {
            frequencyPair.print();
        }
    }

protected:
    std::vector<FrequencyPair> data_;
};
