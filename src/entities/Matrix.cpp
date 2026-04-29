#include "Matrix.h"

#include <cmath>
#include <stdexcept>
#include <vector>

// validators
void Matrix::check_index(size_t r) const
{
    if (r >= rows)
    {
        throw std::out_of_range("Matrix row index out of range");
    }
}
void Matrix::check_index(size_t r, size_t c) const
{
    if (r >= rows || c >= cols)
    {
        throw std::out_of_range("Matrix index out of range");
    }
}
void Matrix::check_matrix_dim(const Matrix& other) const
{
    if (rows != other.rows || cols != other.cols)
    {
        throw std::invalid_argument("Matrix dimensions mismatch");
    }
}
void Matrix::check_division(double n) const
{
    constexpr double kEpsilon = 1e-12;

    if (std::fabs(n) < kEpsilon)
    {
        throw std::invalid_argument("Division by zero or near-zero value");
    }
}

// constructor
Matrix::Matrix(size_t r, size_t c) : rows(r), cols(c)
{
    data.resize(rows, std::vector<double>(cols, 0.0));
}

// element access
double Matrix::get_value(size_t r, size_t c) const
{
    check_index(r, c);
    return data[r][c];
}
void Matrix::set_value(size_t r, size_t c, double val)
{
    check_index(r, c);
    data[r][c] = val;
}

// row access
std::vector<double> Matrix::get_row(size_t r) const
{
    check_index(r);
    return data[r];
}
void Matrix::push_back_row(const std::vector<double>& row)
{
    if (cols != 0 && row.size() != cols)
    {
        throw std::invalid_argument(
            "Row size does not match with the column dimension of the matrix");
    }

    data.push_back(row);
    rows += 1;

    if (cols == 0)
    {
        cols = row.size();
    }
}

// arthmetic operators
Matrix Matrix::operator+(const Matrix& other) const
{
    check_matrix_dim(other);

    Matrix result(rows, cols);

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            result.data[i][j] = data[i][j] + other.data[i][j];
        }
    }

    return result;
}
Matrix Matrix::operator-(const Matrix& other) const
{
    check_matrix_dim(other);

    Matrix result(rows, cols);

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            result.data[i][j] = data[i][j] - other.data[i][j];
        }
    }

    return result;
}
Matrix Matrix::operator/(double n) const
{
    check_division(n);

    Matrix result(rows, cols);

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            result.data[i][j] = data[i][j] / n;
        }
    }

    return result;
}

// statistical operations (column-wise)
std::vector<double> Matrix::calculate_mean() const
{
    if (rows == 0)
    {
        throw std::runtime_error("Cannot calculate mean: matrix has no rows");
    }

    std::vector<double> means(cols, 0.0);

    for (size_t c = 0; c < cols; ++c)
    {
        double sum = 0.0;
        for (size_t r = 0; r < rows; ++r)
        {
            sum += data[r][c];
        }
        means[c] = sum / rows;
    }

    return means;
}
std::vector<double> Matrix::calculate_std() const
{
    if (rows < 2)
    {
        throw std::runtime_error(
            "Cannot calculate standard deviation: at least 2 rows are required");
    }

    const std::vector<double> means = calculate_mean();
    std::vector<double> stds(cols, 0.0);

    for (size_t c = 0; c < cols; ++c)
    {
        double sum_sq_diff = 0.0;
        for (size_t r = 0; r < rows; ++r)
        {
            const double diff = data[r][c] - means[c];
            sum_sq_diff += diff * diff;
        }
        stds[c] = std::sqrt(sum_sq_diff / (rows - 1));
    }

    return stds;
}