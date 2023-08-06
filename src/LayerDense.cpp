#include "LayerDense.h"

#include <iostream>
#include <random>
#include <stdexcept>

LayerDense::LayerDense(size_t n_inputs, size_t n_neurons,
                       ActivationMethod activation)
    : m_weights(n_inputs, n_neurons),
      m_biases(1, n_neurons),
      m_activation(activation),
      output(nullptr) {
  // Initialize the weights with random values between -1 and 1.
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> distribution(-1.0, 1.0);

  for (size_t i = 0; i < n_inputs; ++i) {
    for (size_t j = 0; j < n_neurons; ++j) {
      m_weights(i, j) = distribution(gen);
    }
  }

  // Initialize the biases with zeros.
  for (size_t j = 0; j < n_neurons; ++j) {
    m_biases(0, j) = 0.0;
  }
}

LayerDense::~LayerDense() {
  if (output != nullptr) {
    delete output;
    output = nullptr;
  }
}

void LayerDense::forward(const Matrix& inputs) {
  if (inputs.numColumns() != m_weights.numRows()) {
    throw std::invalid_argument(
        "Input size does not match the layer's input size.");
  }

  // If output has been allocated and its dimensions match the expected
  // dimensions
  if (output != nullptr && output->numRows() == inputs.numRows() &&
      output->numColumns() == m_biases.numColumns()) {
    // Just overwrite the values
  } else {
    // Delete previous matrix and allocate new memory
    delete output;
    output = new Matrix(inputs.numRows(), m_biases.numColumns());
  }

  *output = m_activation.forward(inputs * m_weights + m_biases);
}

void LayerDense::setWeights(const Matrix& weights) {
  if (weights.numRows() != m_weights.numRows() ||
      weights.numColumns() != m_weights.numColumns()) {
    throw std::invalid_argument(
        "Weights matrix size does not match the layer's weights matrix size.");
  }

  m_weights = weights;
}

void LayerDense::setBiases(const Matrix& biases) {
  if (biases.numRows() != m_biases.numRows() ||
      biases.numColumns() != m_biases.numColumns()) {
    throw std::invalid_argument(
        "Biases matrix size does not match the layer's biases matrix size.");
  }

  m_biases = biases;
}

void LayerDense::setActivation(ActivationMethod activation) {
  m_activation.setActivationMethod(activation);
}

void LayerDense::save(std::ofstream& file) const {
  file.write("\x10", 1);

  // Extract the ActivationMethod enum from the Activation object
  ActivationMethod method = m_activation.getActivationMethod();

  // Now cast the enum to int32_t
  int32_t activationValue = static_cast<int32_t>(method);
  file.write(reinterpret_cast<const char*>(&activationValue),
             sizeof(activationValue));

  int32_t rows = static_cast<int32_t>(m_weights.numRows());
  int32_t cols = static_cast<int32_t>(m_weights.numColumns());
  file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
  file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
  file.write(reinterpret_cast<const char*>(m_weights.data()),
             rows * cols * sizeof(double));

  rows = static_cast<int32_t>(m_biases.numRows());
  file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
  file.write(reinterpret_cast<const char*>(m_biases.data()),
             rows * sizeof(double));
}

void LayerDense::load(std::ifstream& file) {
  char identifier;
  file.read(&identifier, 1);
  if (identifier != '\x10') {
    throw std::invalid_argument("Invalid dense layer identifier.");
  }

  int32_t activationValue;
  file.read(reinterpret_cast<char*>(&activationValue), sizeof(activationValue));

  // Cast back to ActivationMethod and set it to the Activation object
  ActivationMethod method = static_cast<ActivationMethod>(activationValue);
  m_activation.setActivationMethod(method);

  int32_t rows, cols;
  file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
  file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
  m_weights = Matrix(rows, cols);
  file.read(reinterpret_cast<char*>(m_weights.data()),
            rows * cols * sizeof(double));

  file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
  m_biases = Matrix(rows, 1);
  file.read(reinterpret_cast<char*>(m_biases.data()), rows * sizeof(double));
}

void LayerDense::print() const {
  std::cout << "Layer Dense" << std::endl;
  std::cout << "Activation: " << m_activation.toString() << std::endl;
  std::cout << "Weights: " << std::endl;
  m_weights.print();
  std::cout << "Biases: " << std::endl;
  m_biases.print();
}