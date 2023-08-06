#pragma once

#include <vector>

class Matrix {
 private:
  std::vector<double> m_data;
  size_t m_rows;
  size_t m_cols;

 public:
  // Constructors
  Matrix(size_t rows, size_t cols);
  Matrix(const std::vector<double>& values);
  Matrix(const std::initializer_list<double>& values);
  Matrix(const std::vector<std::vector<double>>& values);
  Matrix(const std::initializer_list<std::initializer_list<double>>& values);

  // Accessors
  size_t numRows() const;
  size_t numColumns() const;
  void print() const;

  // Element access
  double& operator()(size_t col);
  const double& operator()(size_t col) const;
  double& operator()(size_t row, size_t col);
  const double& operator()(size_t row, size_t col) const;

  // operators
  static Matrix dotProduct(const Matrix& lhs, const Matrix& rhs);
  static Matrix add(const Matrix& lhs, const Matrix& rhs);
  Matrix operator+(const Matrix& rhs) const;
  Matrix operator*(const Matrix& rhs) const;
  Matrix transpose() const;

  // Getters
  const double* data() const;
  double* data();
};
