#include "pidginspec/score.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void ScoresSupportedMatchesAndCriticalOmissions() {
    const std::vector<pidginspec::EvaluationCase> cases{
        {
            "case-a",
            {{"r1", true}, {"r2", false}},
            {{"p1", std::string("r1"), true}, {"p2", std::nullopt, true}}
        },
        {
            "case-b",
            {{"r3", true}},
            {{"p3", std::string("r3"), false}}
        }
    };
    const auto result = pidginspec::CalculateMetrics(pidginspec::ScoreCases(cases));
    Require(result.counts.cases == 2, "case count mismatch");
    Require(result.counts.true_positives == 1, "true positive count mismatch");
    Require(result.counts.false_positives == 2, "false positive count mismatch");
    Require(result.counts.false_negatives == 2, "false negative count mismatch");
    Require(result.counts.critical_true_positives == 1, "critical true positive mismatch");
    Require(result.counts.critical_false_negatives == 1, "critical false negative mismatch");
    Require(std::abs(*result.precision - (1.0 / 3.0)) < 1e-9, "precision mismatch");
    Require(std::abs(*result.recall - (1.0 / 3.0)) < 1e-9, "recall mismatch");
    Require(std::abs(*result.critical_recall - 0.5) < 1e-9, "critical recall mismatch");
}

void RejectsDuplicateGoldIds() {
    const std::vector<pidginspec::EvaluationCase> cases{
        {"duplicate", {{"r1", true}, {"r1", false}}, {}}
    };
    bool threw = false;
    try {
        (void)pidginspec::ScoreCases(cases);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw, "duplicate gold IDs should be rejected");
}

void ReadsJsonLinesAndFormatsUndefinedRates() {
    std::istringstream input(
        "{\"case_id\":\"synthetic\",\"gold_requirements\":[],\"judgments\":[]}\n");
    const auto rows = pidginspec::ReadJsonLines(input);
    const auto summary = pidginspec::FormatSummaryTsv(pidginspec::CalculateMetrics(pidginspec::ScoreCases(rows)));
    Require(summary.find("precision\tnull\n") != std::string::npos, "zero denominator must be explicit");
    Require(summary.find("cases\t1\n") == 0, "case count missing");
}

} // namespace

int main() {
    try {
        ScoresSupportedMatchesAndCriticalOmissions();
        RejectsDuplicateGoldIds();
        ReadsJsonLinesAndFormatsUndefinedRates();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
