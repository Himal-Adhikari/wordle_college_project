#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

// Constants
constexpr int LENGTH = 5;
constexpr int WORDS_SIZE = 4000;
constexpr int VALID_WORDS_SIZE = 20000;
constexpr int GUESSES = 6;
constexpr int CORRECT = 0;
constexpr int CLOSE = 1;
constexpr int WRONG = 2;

// ANSI color codes for boxed-in letters
const std::string GREEN = "\e[38;2;255;255;255;1m\e[48;2;106;170;100;1m";
const std::string YELLOW = "\e[38;2;255;255;255;1m\e[48;2;201;180;88;1m";
const std::string RED = "\e[38;2;255;255;255;1m\e[48;2;220;20;60;1m";
const std::string RESET = "\e[0;39m";

// Hash Table Node Class
class Node {
public:
  std::string word;
  Node *next;

  Node(std::string &w) : word(w), next(nullptr) {}
};

// Hash Table Class
class HashTable {
private:
  std::vector<Node *> table;
  unsigned long size;

  unsigned long hash(const std::string &word) const {
    unsigned long hash_value = 0;
    for (size_t i = 0; i < word.size(); ++i) {
      hash_value += word[i] * (i + 100);
    }
    return hash_value % size;
  }

public:
  HashTable(unsigned long s) : size(s) { table.resize(size, nullptr); }

  ~HashTable() {
    for (Node *head : table) {
      Node *cursor = head;
      while (cursor) {
        Node *temp = cursor;
        cursor = cursor->next;
        delete temp;
      }
    }
  }

  bool load(const std::string &file) {
    std::ifstream fptr(file);
    if (!fptr)
      return false;

    std::string word;
    while (fptr >> word) {
      unsigned long hash_value = hash(word);
      Node *new_node = new Node(word);
      new_node->next = table[hash_value];
      table[hash_value] = new_node;
    }
    return true;
  }

  bool exists(const std::string &word) {
    unsigned long hash_value = hash(word);
    Node *cursor = table[hash_value];
    while (cursor) {
      if (cursor->word == word) {
        return true;
      }
      cursor = cursor->next;
    }
    return false;
  }

  std::string get_word(unsigned long index) {
    Node *cursor = table[index];
    if (cursor == nullptr)
      return "";

    std::vector<std::string> words;
    while (cursor) {
      words.push_back(cursor->word);
      cursor = cursor->next;
    }

    if (words.empty())
      return "";

    unsigned int element_index = rand() % words.size();
    return words[element_index];
  }
};

void print_word(std::string &word, std::vector<int> &letters_correctness) {
  for (int i = 0; i < word.size(); i++) {
    if (letters_correctness[i] == CORRECT) {
      std::cout << GREEN << word[i] << ' ' << RESET;
    } else if (letters_correctness[i] == CLOSE) {
      std::cout << YELLOW << word[i] << ' ' << RESET;
    } else {
      std::cout << RED << word[i] << ' ' << RESET;
    }
  }
  std::cout << '\n';
}

void check_word(std::string &word, std::string &guess,
                std::vector<int> &letter_correctness) {
  std::fill(letter_correctness.begin(), letter_correctness.end(), WRONG);

  // Check for correct letters
  for (int i = 0; i < LENGTH; i++) {
    if (word[i] == guess[i]) {
      letter_correctness[i] = CORRECT;
    }
  }

  // Check for close letters
  for (int i = 0; i < LENGTH; i++) {
    if (letter_correctness[i] != WRONG)
      continue;

    bool letter_is_close = false;
    for (int j = 0; j < LENGTH; j++) {
      if (word[i] == guess[j] && letter_correctness[j] != CORRECT) {
        letter_is_close = true;
        break;
      }
    }
    if (letter_is_close) {
      letter_correctness[i] = CLOSE;
    }
  }
}

// Main Function
int main() {
  std::srand(std::time(nullptr));

  HashTable words_table(WORDS_SIZE);
  HashTable valid_words_table(VALID_WORDS_SIZE);

  if (!words_table.load("words.txt")) {
    std::cerr << "Couldn't open file `words.txt`\n";
    return 1;
  }

  if (!valid_words_table.load("valid_words.txt")) {
    std::cerr << "Couldn't open file `valid_words.txt`\n";
    return 1;
  }

  unsigned long index = std::rand() % WORDS_SIZE;
  while (words_table.get_word(index).empty()) {
    index = std::rand() % WORDS_SIZE;
  }

  std::string guess = words_table.get_word(index);
  std::string input_word;
  int guesses = 0;
  bool won = false;

  std::cout << GREEN << "Welcome to Wordle!" << RESET << std::endl;
  std::cout << YELLOW << "You have " << GUESSES << " guesses to guess the word"
            << RESET << std::endl;
  std::cout << "The correct letter in the right position is represented by "
            << GREEN << "green color" << RESET << std::endl;
  std::cout << "The correct letter in the wrong position is represented by "
            << YELLOW << "yellow color" << RESET << std::endl;
  std::cout << "The wrong letter represented by " << RED << "red color" << RESET
            << std::endl;

  std::vector<int> letters_correctness(LENGTH);

  while (guesses < GUESSES) {
    std::cout << "Enter a " << LENGTH << " letter word: ";
    std::getline(std::cin, input_word);
    std::transform(input_word.begin(), input_word.end(), input_word.begin(),
                   ::tolower);

    if (input_word.size() != LENGTH) {
      std::cout << RED << "Please enter a five letter word\n" << RESET;
      continue;
    }

    if (!valid_words_table.exists(input_word)) {
      std::cout << RED << "Word Not Found\n" << RESET;
      continue;
    }

    if (input_word == guess) {
      won = true;
      break;
    } else {
      check_word(input_word, guess, letters_correctness);
      ++guesses;
    }
    print_word(input_word, letters_correctness);
  }

  if (won) {
    std::cout << GREEN << "That is the correct guess\n";
    std::cout << "CONGRATULATIONS!!! You Win\n" << RESET;
  } else {
    std::cout << "The right answer was " << GREEN << guess << RESET << '\n';
  }

  return 0;
}
