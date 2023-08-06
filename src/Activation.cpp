#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "LayerDense.h"

Activation::~Activation() {
  // Deallocate memory for the output matrix
  if (m_output != nullptr) {
    delete m_output;
    m_output = nullptr;
  }
}

Matrix Activation::forward(const Matrix& inputs) {
  // Check dimensions and allocate memory if necessary
  if (m_output == nullptr || m_output->numRows() != inputs.numRows() ||
      m_output->numColumns() != inputs.numColumns()) {
    delete m_output;
    m_output = new Matrix(inputs.numRows(), inputs.numColumns());
  }

  switch (m_activation) {
    case ActivationMethod::ReLU:
      for (size_t i = 0; i < inputs.numRows(); ++i) {
        for (size_t j = 0; j < inputs.numColumns(); ++j) {
          double value = inputs(i, j);
          (*m_output)(i, j) = std::max(0.0, value);
        }
      }
      break;

    case ActivationMethod::Sigmoid:
      for (size_t i = 0; i < inputs.numRows(); ++i) {
        for (size_t j = 0; j < inputs.numColumns(); ++j) {
          double value = inputs(i, j);
          (*m_output)(i, j) = 1.0 / (1.0 + std::exp(-value));
        }
      }
      break;

    case ActivationMethod::Softmax:
      for (size_t i = 0; i < inputs.numRows(); ++i) {
        double maxVal = -std::numeric_limits<double>::infinity();
        double sumExp = 0.0;
        // Find the max value for numerical stability
        for (size_t j = 0; j < inputs.numColumns(); ++j) {
          maxVal = std::max(maxVal, inputs(i, j));
        }
        // Compute the sum of exponentials with the max subtracted for stability
        for (size_t j = 0; j < inputs.numColumns(); ++j) {
          (*m_output)(i, j) = std::exp(inputs(i, j) - maxVal);
          sumExp += (*m_output)(i, j);
        }
        // Normalize the row
        for (size_t j = 0; j < inputs.numColumns(); ++j) {
          (*m_output)(i, j) /= sumExp;
        }
      }
      break;

    default:
      throw std::invalid_argument("Unsupported activation method.");
  }

  return *m_output;
}

ActivationMethod Activation::setActivationMethod(ActivationMethod activation) {
  m_activation = activation;
  if (m_output) {
    delete m_output;
  }
  m_output = nullptr;
  return m_activation;
}

// toString getter
std::string Activation::toString() const {
  switch (m_activation) {
    case ActivationMethod::ReLU:
      return "ReLU";
    case ActivationMethod::Sigmoid:
      return "Sigmoid";
    case ActivationMethod::Softmax:
      return "Softmax";
    default:
      return "Unknown";
  }
}