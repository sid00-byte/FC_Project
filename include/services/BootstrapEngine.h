#pragma once

#include <map>
#include <string>
#include <vector>

#include "IEventStudyEngine.h"
#include "MarketManager.h"
#include "Matrix.h"

class BootstrapEngine: public IEventStudyEngine
{
    private:
        // external resources (read-only)
        const MarketManager& manager;
        const std::map<std::string, std::vector<double>>& benchmark_return_map;

        // hyperparameters
        size_t n;
        size_t repetitions;
        size_t sample_size;

        // intermediate buffer and final results
        std::map<std::string, std::vector<double>> abnormal_return_map;
        Matrix expected_aar;
        Matrix aar_std;
        Matrix expected_caar;
        Matrix caar_std;

        // input checks
        void validate_inputs() const;

        // group label -> matrix row
        size_t group_name_to_row(const std::string& group_name) const;

        // abnormal return computation
        std::vector<double> compute_abnormal_returns(
            const std::vector<double>& stock_returns,
            const std::vector<double>& benchmark_returns) const;
        void prepare_abnormal_returns();

        // sampling and aggregation
        std::vector<std::string> sample_from_a_group(
            const std::vector<std::string>& group) const;
        std::vector<double> compute_group_aar(
            const std::vector<std::string>& sample) const;
        std::vector<double> compute_caar_from_aar(
            const std::vector<double>& aar) const;

        // per-group bootstrap pipeline
        void run_group_bootstrap(const std::string& group_name);

    public:
        BootstrapEngine(const MarketManager& mng,
                        const std::map<std::string, std::vector<double>>& brp);

        // run the full event-study bootstrap with the given parameters.
        void run(size_t event_window_n, size_t repetitions, size_t sample_size);

        // result getters (aligned with IEventStudyEngine)
        const Matrix& get_expected_aar() const { return expected_aar; }
        const Matrix& get_aar_std() const { return aar_std; }
        const Matrix& get_expected_caar() const { return expected_caar; }
        const Matrix& get_caar_std() const { return caar_std; }
};