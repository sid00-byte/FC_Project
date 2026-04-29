#pragma once

#include <cstddef>
#include <vector>

class Matrix
{
    private:
        std::vector<std::vector<double>> data;
        size_t rows;
        size_t cols;

        // dimension validation
        void check_index(size_t r) const;
        void check_index(size_t r, size_t c) const;
        void check_matrix_dim(const Matrix& other) const;
        void check_division(double n) const;

    public:
        // constructors
        Matrix() : rows(0), cols(0) {}
        Matrix(size_t r, size_t c);

        // dimension info
        size_t get_rows() const { return rows; }
        size_t get_cols() const { return cols; }

        // element access
        double get_value(size_t r, size_t c) const;
        void set_value(size_t r, size_t c, double val);

        // row access
        std::vector<double> get_row(size_t r) const;
        void push_back_row(const std::vector<double>& row);

        // arithmetic operators
        Matrix operator+(const Matrix& other) const;
        Matrix operator-(const Matrix& other) const;
        Matrix operator/(double n) const; // Hadmard Division

        // statistical operations (column-wise)
        std::vector<double> calculate_mean() const;
        std::vector<double> calculate_std() const;
};