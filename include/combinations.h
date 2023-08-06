#ifndef COMBINATIONS_H
#define COMBINATIONS_H

#include <string>
#include <vector>

class CombinationsGenerator {
 private:
  const std::vector<char>& letters;
  int k;
  std::vector<int> indices;
  bool hasNextCombination;

 public:
  CombinationsGenerator(const std::vector<char>& letters, int k)
      : letters(letters), k(k), hasNextCombination(true) {
    indices.reserve(k);
    for (int i = 0; i < k; ++i) indices.push_back(0);
  }

  std::string getNextCombination() {
    std::string combination = "";
    for (int i = 0; i < k; i++) {
      combination += letters.at(indices.at(i));
    }

    // increment the indices
    for (int i = k - 1; i >= 0; i--) {
      indices.at(i)++;
      if (indices.at(i) == letters.size()) {
        indices.at(i) = 0;
        if (i == 0) {
          hasNextCombination = false;
          break;
        }
      } else {
        break;
      }
    }

    return combination;
  }

  bool hasMoreCombinations() const { return hasNextCombination; }
};

#endif  // COMBINATIONS_H