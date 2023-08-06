#pragma once

#include <fstream>

#include "Activation.h"

class LayerDense {
 private:
  Matrix m_weights;
  Matrix m_biases;
  Activation m_activation;

 public:
  Matrix* output = nullptr;

  // Constructor
  LayerDense(size_t inputSize, size_t outputSize, ActivationMethod activation);
  ~LayerDense();

  // Forward pass
  void forward(const Matrix& input);

  // Save/Load
  void save(std::ofstream& file) const;
  void load(std::ifstream& file);

  // Setters
  void setWeights(const Matrix& weights);
  void setBiases(const Matrix& biases);
  void setActivation(ActivationMethod activation);
  void setOutput(Matrix* output);

  // Getters
  void print() const;
};