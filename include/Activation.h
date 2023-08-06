#pragma once

#include "Matrix.h"

enum class ActivationMethod { ReLU, Sigmoid, Softmax, NONE };

class Activation {
 private:
  ActivationMethod m_activation;
  Matrix* m_output;

 public:
  // Constructor - corrected the constructor name and syntax
  Activation(ActivationMethod activation)
      : m_activation(activation), m_output(nullptr) {}

  // Destructor - corrected the destructor name
  ~Activation();

  // Forward pass - corrected the function name and added the return type
  Matrix forward(const Matrix& input);

  // Getter - activation method
  ActivationMethod getActivationMethod() const { return m_activation; }
  ActivationMethod setActivationMethod(ActivationMethod activation);
  // to string
  std::string toString() const;
};