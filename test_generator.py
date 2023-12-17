import random
from collections import Counter


def generate_words_file(n, filename="words.txt"):
    words = ["the", "walrus", "said", "to", "talk", "of", "many", "things", "time", "has", "come"]
    with open(filename, 'w') as file:
        for _ in range(n):
            word = random.choice(words)
            file.write(word + ' ')


def decode_and_sort_file(input_filename="words.txt", output_filename="decoded.txt"):
    with open(input_filename, 'r') as file:
        words = file.read().split()

    word_count = Counter(words)
    sorted_word_count = sorted(word_count.items(), key=lambda x: (-x[1], x[0]))

    with open(output_filename, 'w') as file:
        for word, count in sorted_word_count:
            file.write(f"{count} {word}\n")

if __name__ == "__main__":
    n = 100
    generate_words_file(n)
    decode_and_sort_file()
