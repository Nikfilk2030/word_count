#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

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

private:
    int64_t count_ = 0;
    std::string word_ = "";
};

class WordCounter {
public:
    WordCounter() = default;

    void Count(const std::string& word, size_t count = 1) {
        data_[word] += count;
    }

    std::map<std::string, size_t>& GetData() {
        return data_;
    }

    void Absorb(WordCounter& other) {
        auto& otherData = other.GetData();
        for (const auto& [word, count] : otherData) {
            this->Count(word, count);
        }
    }

private:
    std::map<std::string, size_t> data_;
};

class WordCounterContainer {
public:
    WordCounterContainer(int16_t size) {
        container_.resize(size);
    }

    ~WordCounterContainer() {}

    void setWordCounter(int16_t index, std::unique_ptr<WordCounter> fv) {
        std::lock_guard<std::mutex> lock(mutex_);
        container_[index] = std::move(fv);
    }

    const WordCounter& getWordCounter(int16_t index) const {
        return *container_[index];
    }

    int16_t size() const {
        return container_.size();
    }

    class Iterator : public std::iterator<std::input_iterator_tag, std::unique_ptr<WordCounter>> {
    public:
        Iterator(typename std::vector<std::unique_ptr<WordCounter>>::iterator it) : it_(it) {}

        Iterator& operator++() {
            ++it_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const {
            return it_ == other.it_;
        }

        bool operator!=(const Iterator& other) const {
            return it_ != other.it_;
        }

        std::unique_ptr<WordCounter>& operator*() {
            return *it_;
        }

    private:
        typename std::vector<std::unique_ptr<WordCounter>>::iterator it_;
    };

    Iterator begin() {
        return Iterator(container_.begin());
    }

    Iterator end() {
        return Iterator(container_.end());
    }

private:
    std::vector<std::unique_ptr<WordCounter>> container_;
    mutable std::mutex mutex_;
};

class FrequencyVector {
public:
    FrequencyVector() = default;

    FrequencyVector(WordCounter& wordCounter) {
        auto& wordToCount = wordCounter.GetData();
        data_.reserve(wordToCount.size());

        for (const auto& [word, count] : wordToCount) {
            data_.emplace_back(FrequencyPair(word, count));
        }
    }

    ~FrequencyVector() {}

    void sort() {
        std::sort(data_.begin(), data_.end());
    }

    void print() const {
        for (const FrequencyPair& frequencyPair : data_) {
            frequencyPair.print();
        }
    }

private:
    std::vector<FrequencyPair> data_;
};

struct Borders {
    size_t start = 0;
    size_t end = 0;
};

size_t GetFileSize();

std::vector<Borders> CountThreadBorders();

void ProcessChunk(int16_t index, Borders& borders, WordCounterContainer& container);

bool TrySetToNearestSpace(const size_t fileSize, size_t* position);


