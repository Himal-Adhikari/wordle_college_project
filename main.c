#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>

#define LENGTH 5
// This is the size of the hash=table
#define WORDS_SIZE 4000 
#define VALID_WORDS_SIZE 20000

#define GUESSES 6
#define CORRECT 0
#define CLOSE 1
#define WRONG 2

// ANSI color codes for boxed in letters
#define GREEN   "\e[38;2;255;255;255;1m\e[48;2;106;170;100;1m"
#define YELLOW  "\e[38;2;255;255;255;1m\e[48;2;201;180;88;1m"
#define RED     "\e[38;2;255;255;255;1m\e[48;2;220;20;60;1m"
#define RESET   "\e[0;39m"

// Represents a node in a hash table, the pointer in it exists for dealing with collision which is handled using a linked list
typedef struct node {
  char word[LENGTH + 1];
  struct node* next;
}
node;

unsigned long hash(char* word, unsigned long size);
bool load(char* file, node** table, unsigned long size); 
bool exists(char word[], unsigned long table_size, node** table);
void print_word(char word[], int letters_correctness[]);
void load_word(unsigned long index, char guess[], node** table);
void check_word(char word[], char guess[], int letter_correctness[]);

int main() {
  node* table[WORDS_SIZE];
  node* valid_words[VALID_WORDS_SIZE];
  for (int i = 0; i < VALID_WORDS_SIZE; i++) {
    valid_words[i] = NULL;
  }

  // These are words that wordle selects the daily word from
  if (load("words.txt", table, WORDS_SIZE) == false) {
    fprintf(stderr, "Couldn't open file `words.txt`\n");
    return 1;
  }

  // These are words that wordle considers to be valid guess
  if (load("valid_words.txt", valid_words, VALID_WORDS_SIZE) == false) {
    fprintf(stderr, "Couldn't open file `valid_words.txt`\n");
    return 1;
  }

  srand(time(NULL));
  unsigned long index = rand() % WORDS_SIZE;
  while (table[index] == NULL) {
    index = rand() % WORDS_SIZE;
  }

  char guess[6];
  load_word(index, guess, table);

  int guesses = 0;

  printf(GREEN"Welcome to Wordle!\n"RESET);
  printf(YELLOW"You have %d guesses to guess the word\n"RESET, GUESSES);

  int letters_correctness[LENGTH];
  bool won = false;

  while (guesses < GUESSES) {
    char word[1024];
    printf("Enter a %d letter word: ", LENGTH);
    scanf(" %[^\n]s", word);
    int i = 0;
    while (word[i] != '\0') {
      word[i] = tolower(word[i]);
      i++;
    }
    if (strlen(word) != 5) {
      printf(RED"Please enter a five letter word\n"RESET);
      continue;
    }
    if (!exists(word, VALID_WORDS_SIZE, valid_words)) {
      printf(RED"Word Not Found\n"RESET);
      continue;
    }
    if (strcmp(word, guess) == 0) {
      won = true;
      break;
    }
    else {
      check_word(word, guess, letters_correctness);
      guesses += 1;
    }
    print_word(word, letters_correctness);
  }
  if (won) {
    printf(GREEN"That is the correct guess\n");
    printf("CONGRATULATIONS!!! You Win\n"RESET);
  }
  else {
    printf("The right answer was "GREEN"%s\n"RESET, guess);
  }
  return 0;
}

bool load(char* file, node** table, unsigned long size) {
  FILE* fptr;
  fptr = fopen(file, "r");
  if (fptr == NULL) {
    return false;
  }
   
  for (int i = 0; i < size; i++) {
    table[i] = NULL;
  }

  char word[LENGTH + 1];

  while (fscanf(fptr, "%s", word) != EOF) {
    unsigned long hash_value = hash(word, size);
    node* new = malloc(sizeof(node));
    strcpy(new->word, word);
    new->next = table[hash_value];
    table[hash_value] = new;
  }

  fclose(fptr);
  return true;
}

unsigned long hash(char* word, unsigned long size) {
  unsigned long i = 0, hash_value = 0;
  while (word[i] != '\0') {
    hash_value += word[i] * (i + 100);
    i++;
  }
  return hash_value % size;
} 

bool exists(char word[], unsigned long table_size, node** table) {
  unsigned long hash_value = hash(word, table_size);
  node* cursor = table[hash_value];
  if (cursor == NULL) {
    return false;
  }
  do {
    if (strcasecmp(cursor->word, word) == 0) {
      return true;
    }
    cursor = cursor->next;
  }while (cursor != NULL);
  return false;
}

void print_word(char word[], int letters_correctness[]) {
  for (int i = 0; i < LENGTH; i++) {
    if (letters_correctness[i] == CORRECT) {
      printf(GREEN"%c ", word[i]);
    }
    else if (letters_correctness[i] == CLOSE) {
      printf(YELLOW"%c ", word[i]);
    }
    else {
      printf(RED"%c ", word[i]);
    }
  }
  printf(RESET"\n");
}

void load_word(unsigned long index, char guess[], node** table) {
  if (table[index]->next == NULL) {
    strcpy(guess, table[index]->word);
  }
  else {
    node* cursor = table[index]->next;
    unsigned int words_in_linked_list = 1;

    do {
      words_in_linked_list++;
      cursor = cursor->next;
    } while (cursor != NULL);

    unsigned int element_index_in_linked_list = rand() % words_in_linked_list;

    cursor = table[index];
    for (int i = 0; i < element_index_in_linked_list; i++) {
      cursor = cursor->next;
    }
    strcpy(guess, cursor->word);
  }
}

void check_word(char word[], char guess[], int letter_correctness[]) {
  for (int i = 0; i < LENGTH; i++) {
    letter_correctness[i] = WRONG;
  }
  for (int i = 0; i < LENGTH; i++) {
    char letter = tolower(word[i]);
    if (letter == guess[i]) {
      letter_correctness[i] = CORRECT;
    }
  }
  for (int i = 0; i < LENGTH; i++) {
    if (letter_correctness[i] != WRONG) {
      continue;
    }
    bool another_letter_is_correct = false;
    bool letter_is_close = false;
    char letter = tolower(word[i]);
    for (int j = 0; j < LENGTH; j++) {
      if (letter == guess[j]) {
        letter_is_close = true;
        if (letter_correctness[j] == CORRECT) {
          another_letter_is_correct = true;
          break;
        }
      }
    }
    if ((!another_letter_is_correct) && letter_is_close) {
      letter_correctness[i] = CLOSE;
    }
  }
}
