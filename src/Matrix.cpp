#include "Matrix.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

Matrix::Matrix(size_t rows, size_t cols)
    : m_data(rows * cols), m_rows(rows), m_cols(cols) {}

Matrix::Matrix(const std::vector<std::vector<double>>& values)
    : m_rows(values.size()), m_cols(values[0].size()) {
  m_data.reserve(m_rows * m_cols);
  for (const auto& row : values)
    for (const auto& val : row) m_data.push_back(val);
}

Matrix::Matrix(
    const std::initializer_list<std::initializer_list<double>>& values)
    : m_rows(values.size()), m_cols(values.begin()->size()) {
  m_data.reserve(m_rows * m_cols);
  for (const auto& row : values)
    for (const auto& val : row) m_data.push_back(val);
}

Matrix::Matrix(const std::initializer_list<double>& values)
    : m_rows(1), m_cols(values.size()) {
  m_data.reserve(m_cols);
  for (const auto& val : values) m_data.push_back(val);
}

Matrix::Matrix(const std::vector<double>& values)
    : m_rows(1), m_cols(values.size()) {
  m_data = values;  // directly assign to vector
}

size_t Matrix::numRows() const { return m_rows; }

size_t Matrix::numColumns() const { return m_cols; }

void Matrix::print() const {
  std::vector<size_t> max_width(numColumns(), 0);
  bool negativePresent = false;

  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      double num = (*this)(i, j);
      size_t width =
          std::to_string(static_cast<long long>(std::abs(num))).length();
      max_width[j] = std::max(max_width[j], width);
      negativePresent |= (num < 0);
    }
  }

  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      double num = (*this)(i, j);
      std::string numStr = (negativePresent && num >= 0) ? " " : "";
      numStr += std::to_string(num);
      std::cout << std::left << std::setw(max_width[j]) << numStr;
      if (j < numColumns() - 1) {
        std::cout << "  ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

double& Matrix::operator()(size_t row, size_t col) {
  return m_data[row * m_cols + col];
}

double& Matrix::operator()(size_t col) {
  if (numRows() > 1) {
    throw std::invalid_argument(R"(
        2D Matrices cannot be accessed with a single parameter. 
        Use operator()(size_t row, size_t col) for 2D matrices.
    )");
  }
  return m_data[col];
}

const double& Matrix::operator()(size_t row, size_t col) const {
  return m_data[row * m_cols + col];
}

const double& Matrix::operator()(size_t col) const {
  if (numRows() > 1) {
    throw std::invalid_argument(R"(
        2D Matrices cannot be accessed with a single parameter. 
        Use operator()(size_t row, size_t col) for 2D matrices.
    )");
  }
  return m_data[col];
}

Matrix Matrix::dotProduct(const Matrix& lhs, const Matrix& rhs) {
  // Check if the matrices are compatible for multiplication
  if (lhs.numColumns() != rhs.numRows()) {
    throw std::invalid_argument(
        "Number of columns in the left matrix must be equal to the number of "
        "rows in the right matrix.");
  }

  // Initialize the result matrix based on the input matrices' dimensions
  size_t resultRows = lhs.numRows();
  size_t resultCols = rhs.numColumns();
  Matrix result(resultRows, resultCols);

  // Perform matrix multiplication or dot product
  for (size_t i = 0; i < resultRows; ++i) {
    for (size_t j = 0; j < resultCols; ++j) {
      double dotProduct = 0.0;
      for (size_t k = 0; k < lhs.numColumns(); ++k) {
        dotProduct += lhs(i, k) * rhs(k, j);
      }
      result(i, j) = dotProduct;
    }
  }

  return result;
}

Matrix Matrix::operator*(const Matrix& rhs) const {
  return dotProduct(*this, rhs);
}

Matrix Matrix::add(const Matrix& lhs, const Matrix& rhs) {
  if ((rhs.numRows() != 1) && (lhs.numRows() != rhs.numRows() ||
                               lhs.numColumns() != rhs.numColumns())) {
    throw std::invalid_argument(
        "Matrices must have the same dimensions for addition.");
  }

  size_t rows = lhs.numRows();
  size_t cols = lhs.numColumns();
  Matrix result(rows, cols);

  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      result(i, j) = lhs(i, j) + rhs(rhs.numRows() > 1 ? i : 0, j);
    }
  }

  return result;
}

Matrix Matrix::operator+(const Matrix& rhs) const { return add(*this, rhs); }

Matrix Matrix::transpose() const {
  Matrix result(m_cols, m_rows);

  for (size_t i = 0; i < m_rows; ++i) {
    for (size_t j = 0; j < m_cols; ++j) {
      result(j, i) = (*this)(i, j);
    }
  }

  return result;
}

const double* Matrix::data() const { return m_data.data(); }

double* Matrix::data() { return m_data.data(); }