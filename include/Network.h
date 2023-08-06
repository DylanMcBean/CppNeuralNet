#pragma once

#include "LayerDense.h"

class Network {
 private:
  Matrix* m_inputs;
  std::vector<LayerDense*> m_layers;
  int m_batchSize;

 public:
  Matrix* outputs;

  Network(Matrix* inputs, int batchSize);
  void AddLayer(LayerDense* layer);
  void Forward();
  void SetInputs(Matrix* inputs);

  void Save(std::ofstream& file) const;
  void Load(std::ifstream& file);
};