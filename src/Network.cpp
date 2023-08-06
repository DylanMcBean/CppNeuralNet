#include "Network.h"

#include <iostream>
#include <stdexcept>

Network::Network(Matrix* inputs, int batchSize)
    : m_inputs(inputs), m_batchSize(batchSize) {
  outputs = nullptr;
}

void Network::AddLayer(LayerDense* layer) { m_layers.emplace_back(layer); }

void Network::Forward() {
  // Check if the inputs pointers are valid
  if (!m_inputs) {
    throw std::invalid_argument(
        "Invalid inputs pointer. Make sure to provide valid inputs before "
        "calling Forward.");
  }

  Matrix* input = m_inputs;
  for (size_t i = 0; i < m_layers.size(); i++) {
    m_layers[i]->forward(*input);
    input = m_layers[i]->output;
  }

  outputs = input;
}

void Network::SetInputs(Matrix* inputs) { m_inputs = inputs; }

void Network::Save(std::ofstream& file) const {
  file.write("\x00", 1);

  file.write(reinterpret_cast<const char*>(&m_batchSize), sizeof(m_batchSize));

  int32_t num_layers = static_cast<int32_t>(m_layers.size());
  file.write(reinterpret_cast<const char*>(&num_layers), sizeof(num_layers));

  for (auto& layer : m_layers) {
    layer->print();
    layer->save(file);
  }
}

void Network::Load(std::ifstream& file) {
  char identifier;
  file.read(&identifier, 1);
  if (identifier != '\x00') {
    throw std::invalid_argument("Invalid network identifier.");
  }

  file.read(reinterpret_cast<char*>(&m_batchSize), sizeof(m_batchSize));

  int32_t num_layers;
  file.read(reinterpret_cast<char*>(&num_layers), sizeof(num_layers));

  m_layers.clear();
  for (int32_t i = 0; i < num_layers; i++) {
    LayerDense* layer = new LayerDense(0, 0, ActivationMethod::NONE);
    layer->load(file);
    layer->print();
    m_layers.emplace_back(layer);
  }

  SetInputs(nullptr);
}