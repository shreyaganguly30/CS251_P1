#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

/*
 * Print instructions for using the program.
 */
void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher (File)" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

int main() {
  Random::seed(time(NULL));
  string command;

  // Load dictionary
  ifstream fileOpener;
  string nextWord;
  vector<string> dictionary;
  fileOpener.open("dictionary.txt");
  if (fileOpener.is_open()) {
    while (!fileOpener.eof()) {
      fileOpener >> nextWord;
      dictionary.push_back(nextWord);
    }
  }
  fileOpener.close();
  
  // Create quadgram scorer
  string input;
  int num;
  vector<string> quadgrams;
  vector<int> counts;
  fileOpener.open("english_quadgrams.txt");

  if (fileOpener.is_open()) {
    while (!fileOpener.eof()) {
      fileOpener >> input;
      quadgrams.push_back(input.substr(0, 4));
      num = stoi(input.substr(5, input.length() - 5));
      counts.push_back(num);
    }
  }

  fileOpener.close();
  // Declare file output function
  void decryptFile(const QuadgramScorer& scorer);
  // Declare quadgram function
  QuadgramScorer scorer(quadgrams, counts);

  // Print the Ciphers introduction
  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";

    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;
    // All user commands
    if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    } else if (command == "C" || command == "c") {
      runCaesarEncrypt();
    } else if (command == "D" || command == "d") {
      runCaesarDecrypt(dictionary);
    } else if (command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    } else if (command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    } else if (command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    } else if (command == "F" || command == "f") {
      decryptFile(scorer);
    }
    cout << endl;
  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

// Rotates one char by amount
char rot(char c, int amount) {
  int letterNum = ALPHABET.find(c);
  while (amount + letterNum >= 26 && amount >= 0) {
    amount -= 26;
  }
  return (char)(amount + c);
}

// Rotates all chars in a string by amount
string rot(const string& line, int amount) {
  string newWord = "";
  char letter;
  for (size_t i = 0; i < line.length(); i++) {
    letter = line.at(i);
    if (isalpha(letter)) {
      letter = toupper(letter);
      newWord += rot(letter, amount);
    } else if (isspace(letter)) {
      newWord += letter;
    }
  }
  return newWord;
}

// Calls rot() for chars or strings
void runCaesarEncrypt() {
  string encryptText;
  string rotateBy;
  string result;
  cout << "Enter the text to Caesar encrypt: " << endl;
  getline(cin, encryptText);
  cout << "Enter the number of characters to rotate by: " << endl;
  getline(cin, rotateBy);
  int rotateNum = stoi(rotateBy);
  result = rot(encryptText, rotateNum);
  cout << result << endl;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

// Rotates all strings in a vector by amount
void rot(vector<string>& strings, int amount) {
  for (string& current : strings) {
    current = rot(current, amount);
  }
}

// Removes all non alphabetical characters and capitalizes alphabetical
// characters
string clean(const string& s) {
  string newString;
  for (size_t i = 0; i < s.length(); i++) {
    if (isalpha(s.at(i))) {
      newString += toupper(s.at(i));
    }
  }
  return newString;
}

// Every word separated by a space will individually get placed in a vector
vector<string> splitBySpaces(const string& s) {
  vector<string> strings;
  istringstream input(s);
  string tempString;
  while (input >> tempString) {
    strings.push_back(tempString);
  }
  return strings;
}

// Join every word in a vector with one space in between
string joinWithSpaces(const vector<string>& words) {
  string spacedWords = "";
  for (size_t i = 0; i < words.size(); i++) {
    if (i != words.size() - 1) {
      spacedWords += words.at(i) + " ";
    } else {
      spacedWords += words.at(i);
    }
  }
  return spacedWords;
}

// Returns the number of dictionary words in the vector words
int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  int count = 0;
  for (size_t i = 0; i < words.size(); i++) {
    for (size_t j = 0; j < dict.size(); j++) {
      if (words.at(i) == dict.at(j)) {
        count++;
      }
    }
  }
  return count;
}

// Cleans a string to decrypt, decripts it, and returns it with normal spacing
void runCaesarDecrypt(const vector<string>& dict) {
  string decryptText;
  cout << "Enter the text to Caesar decrypt: " << endl;
  getline(cin, decryptText);
  vector<string> words = splitBySpaces(decryptText);
  for (size_t i = 0; i < words.size(); i++) {
    words.at(i) = clean(words.at(i));
  }
  int numWordDict = 0;
  int numFound = 0;
  for (size_t i = 0; i < 26; i++) {
    numWordDict = numWordsIn(words, dict);
    if (numWordDict > words.size() / 2) {
      cout << joinWithSpaces(words) << endl;
      numFound++;
    }
    rot(words, 1);
  }
  if (numFound == 0) {
    cout << "No good decryptions found" << endl;
  }
}

#pragma endregion CaesarDec

#pragma region SubstEnc

// Applies the cipher code to a string
string applySubstCipher(const vector<char>& cipher, const string& s) {
  string newString = "";
  int val;
  for (size_t i = 0; i < s.size(); i++) {
    if (isalpha(s.at(i))) {
      val = ALPHABET.find(toupper(s.at(i)));
      newString += cipher[val];
    } else {
      newString += s.at(i);
    }
  }
  return newString;
}

// Calls the applySubstCipher function and prints the encrypted word
void applyRandSubstCipherCommand() {
  string textToEncrypt, upperText;
  vector<char> cipher = genRandomSubstCipher();
  cout << "Enter the text to substitution-cipher encrypt: " << endl;
  getline(cin, textToEncrypt);
  string encryptedWord = applySubstCipher(cipher, textToEncrypt);
  cout << encryptedWord << endl;
}

#pragma endregion SubstEnc

#pragma region SubstDec

// Sum the Englishness score for each quadgram a returns it
double scoreString(const QuadgramScorer& scorer, const string& s) {
  double quadScore = 0.0;
  for (size_t i = 0; i < s.length() - 3; i++) {
    quadScore += scorer.getScore(s.substr(i, 4));
  }
  return quadScore;
}

// Prints the Englishness score for a string
void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  string input;
  string cleanInput;
  cout << "Enter a string to score: " << endl;
  getline(cin, input);
  cleanInput = clean(input);
  double score = scoreString(scorer, cleanInput);
  cout << score << endl;
}

// Finds the best cipher key to decrypt a string and returns it
vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& ciphertext) {
  int num1, num2;
  int numTrials = 0;
  char temp;
  // Declare and initalize a best cipher key
  vector<char> bestCipherKey = genRandomSubstCipher();
  string bestCipherText = applySubstCipher(bestCipherKey, ciphertext);
  double bestScore = scoreString(scorer, bestCipherText);
  // Declare a runs and test cipher, text, and score
  vector<char> runsCipherKey;
  string runsCipherText;
  double runsCipherScore;
  vector<char> testCipherKey;
  string testCipherText;
  double testCipherScore;
  // Iterate through 20 runs and 1500 trials
  for (size_t i = 0; i < 20; i++) {
    // For every run, reinitaize runsCipherKey
    runsCipherKey = genRandomSubstCipher();
    runsCipherText = applySubstCipher(runsCipherKey, ciphertext);
    runsCipherScore = scoreString(scorer, runsCipherText);
    // For every trial, set testCipherKey = runsCipherKey
    // Then swap and score testCipherKey
    numTrials = 0;
    while (numTrials < 1500) {
      testCipherKey = runsCipherKey;
      num1 = Random::randInt(25);
      num2 = Random::randInt(25);
      while (num1 == num2) {
        num2 = Random::randInt(25);
      }
      temp = testCipherKey.at(num1);
      testCipherKey.at(num1) = testCipherKey.at(num2);
      testCipherKey.at(num2) = temp;
      testCipherText = applySubstCipher(testCipherKey, ciphertext);
      testCipherScore = scoreString(scorer, testCipherText);
      // If the new score is better than the old score, replace them, and
      // reset the trials. Else, increment the trials
      if (testCipherScore > runsCipherScore) {
        runsCipherKey = testCipherKey;
        runsCipherScore = testCipherScore;
        numTrials = 0;
      } else {
        numTrials++;
      }
    }
    // After the loop of 1500 times, set your best cipher
    if (runsCipherScore > bestScore) {
      bestCipherKey = runsCipherKey;
      bestScore = runsCipherScore;
    }
  }
  return bestCipherKey;
}

// Decrypts a user inputted string
void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  string decryptText;
  string cleanDecryptText = "";
  cout << "Enter text to substitution-cipher decrypt: " << endl;
  getline(cin, decryptText);
  // Return the best string
  vector<char> cipherKey = decryptSubstCipher(scorer, clean(decryptText));
  string decryptedString = applySubstCipher(cipherKey, decryptText);
  cout << decryptedString << endl;
}

#pragma endregion SubstDec

// Decrypts the contents of a file
void decryptFile(const QuadgramScorer& scorer) {
  string inputFilename;
  string outputFilename;
  cout << "Enter filename with text to substitution-cipher decrypt: " << endl;
  getline(cin, inputFilename);
  cout << "Enter filename to write results to: " << endl;
  getline(cin, outputFilename);
  ifstream inFS;      // cin from file
  ostringstream oss;  // like cout
  inFS.open(inputFilename);
  string decodeText;
  // While there is another line in the file, output it to the output
  // stringstream
  while (getline(inFS, decodeText)) {
    oss << decodeText << endl;
  }
  inFS.close();
  // Decrypt the strings from the user inputted file
  string cipherText = oss.str();  //  convert oss to string
  vector<char> cipherKey = decryptSubstCipher(scorer, clean(cipherText));
  string decryptedString = applySubstCipher(cipherKey, cipherText);
  istringstream iss(decryptedString);  // like cin
  ofstream outFS(outputFilename);      // like cout
  string line;
  // While there is another line in the in stringstream, send it to the
  // ofstream, which outputs the content
  while (getline(iss, line)) {
    outFS << line << endl;
  }
  outFS.close();
}