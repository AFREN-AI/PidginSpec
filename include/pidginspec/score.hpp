#pragma once

#include <cstddef>
#include <istream>
#include <optional>
#include <string>
#include <vector>

namespace pidginspec {

struct GoldRequirement {
    std::string id;
    bool critical{};
};

struct Judgment {
    std::string prediction_id;
    std::optional<std::string> gold_id;
    bool evidence_supported{};
};

struct EvaluationCase {
    std::string case_id;
    std::vector<GoldRequirement> gold_requirements;
    std::vector<Judgment> judgments;
};

struct Counts {
    std::size_t cases{};
    std::size_t true_positives{};
    std::size_t false_positives{};
    std::size_t false_negatives{};
    std::size_t critical_true_positives{};
    std::size_t critical_false_negatives{};
};

struct Metrics {
    Counts counts;
    std::optional<double> precision;
    std::optional<double> recall;
    std::optional<double> f1;
    std::optional<double> critical_recall;
};

Counts ScoreCases(const std::vector<EvaluationCase>& cases);
Metrics CalculateMetrics(const Counts& counts);
std::vector<EvaluationCase> ReadJsonLines(std::istream& input);
std::string FormatSummaryTsv(const Metrics& metrics);

} // namespace pidginspec
