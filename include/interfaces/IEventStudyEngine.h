#pragma once

#include <cstddef>

class Matrix;
class IEventStudyEngine
{
    public:
        virtual void run(size_t n, size_t repetitions, size_t sample_size) = 0;
        virtual const Matrix& get_expected_aar() const = 0;
        virtual const Matrix& get_aar_std() const = 0;
        virtual const Matrix& get_expected_caar() const = 0;
        virtual const Matrix& get_caar_std() const = 0;

        virtual ~IEventStudyEngine() = default;
};