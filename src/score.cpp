#include "pidginspec/score.hpp"

#include <cmath>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace pidginspec {
namespace {

std::optional<double> Ratio(std::size_t numerator, std::size_t denominator) {
    if (denominator == 0) {
        return std::nullopt;
    }
    return static_cast<double>(numerator) / static_cast<double>(denominator);
}

std::string FormatOptional(const std::optional<double>& value) {
    if (!value.has_value() || !std::isfinite(*value)) {
        return "null";
    }
    std::ostringstream output;
    output << std::fixed << std::setprecision(8) << *value;
    return output.str();
}

EvaluationCase ParseCase(const nlohmann::json& row, std::size_t line_number) {
    if (!row.is_object()) {
        throw std::invalid_argument("line " + std::to_string(line_number) + " must be an object");
    }
    EvaluationCase result;
    try {
        result.case_id = row.at("case_id").get<std::string>();
        for (const auto& item : row.at("gold_requirements")) {
            result.gold_requirements.push_back(GoldRequirement{
                item.at("id").get<std::string>(),
                item.at("critical").get<bool>()
            });
        }
        for (const auto& item : row.at("judgments")) {
            std::optional<std::string> gold_id;
            if (!item.at("gold_id").is_null()) {
                gold_id = item.at("gold_id").get<std::string>();
            }
            result.judgments.push_back(Judgment{
                item.at("prediction_id").get<std::string>(),
                gold_id,
                item.at("evidence_supported").get<bool>()
            });
        }
    } catch (const nlohmann::json::exception& error) {
        throw std::invalid_argument("line " + std::to_string(line_number) + " has an invalid schema: " + error.what());
    }
    return result;
}

void ValidateCase(const EvaluationCase& item) {
    if (item.case_id.empty()) {
        throw std::invalid_argument("case_id is required");
    }
    std::unordered_set<std::string> gold_ids;
    for (const auto& requirement : item.gold_requirements) {
        if (requirement.id.empty() || !gold_ids.insert(requirement.id).second) {
            throw std::invalid_argument("gold requirement ids must be non-empty and unique in case " + item.case_id);
        }
    }
    std::unordered_set<std::string> prediction_ids;
    for (const auto& judgment : item.judgments) {
        if (judgment.prediction_id.empty() || !prediction_ids.insert(judgment.prediction_id).second) {
            throw std::invalid_argument("prediction ids must be non-empty and unique in case " + item.case_id);
        }
    }
}

} // namespace

Counts ScoreCases(const std::vector<EvaluationCase>& cases) {
    Counts totals;
    std::unordered_set<std::string> case_ids;
    for (const auto& item : cases) {
        ValidateCase(item);
        if (!case_ids.insert(item.case_id).second) {
            throw std::invalid_argument("case ids must be unique: " + item.case_id);
        }
        ++totals.cases;
        std::unordered_map<std::string, bool> gold_critical;
        for (const auto& requirement : item.gold_requirements) {
            gold_critical.emplace(requirement.id, requirement.critical);
        }
        std::unordered_set<std::string> matched;
        for (const auto& judgment : item.judgments) {
            const bool valid_match = judgment.gold_id.has_value()
                && gold_critical.contains(*judgment.gold_id)
                && judgment.evidence_supported
                && !matched.contains(*judgment.gold_id);
            if (valid_match) {
                const auto& id = *judgment.gold_id;
                matched.insert(id);
                ++totals.true_positives;
                if (gold_critical.at(id)) {
                    ++totals.critical_true_positives;
                }
            } else {
                ++totals.false_positives;
            }
        }
        for (const auto& [id, critical] : gold_critical) {
            if (!matched.contains(id)) {
                ++totals.false_negatives;
                if (critical) {
                    ++totals.critical_false_negatives;
                }
            }
        }
    }
    return totals;
}

Metrics CalculateMetrics(const Counts& counts) {
    Metrics result;
    result.counts = counts;
    result.precision = Ratio(counts.true_positives, counts.true_positives + counts.false_positives);
    result.recall = Ratio(counts.true_positives, counts.true_positives + counts.false_negatives);
    if (result.precision.has_value() && result.recall.has_value()
        && *result.precision + *result.recall > 0.0) {
        result.f1 = 2.0 * *result.precision * *result.recall / (*result.precision + *result.recall);
    }
    result.critical_recall = Ratio(
        counts.critical_true_positives,
        counts.critical_true_positives + counts.critical_false_negatives);
    return result;
}

std::vector<EvaluationCase> ReadJsonLines(std::istream& input) {
    std::vector<EvaluationCase> cases;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }
        try {
            cases.push_back(ParseCase(nlohmann::json::parse(line), line_number));
        } catch (const nlohmann::json::parse_error&) {
            throw std::invalid_argument("line " + std::to_string(line_number) + " is not valid JSON");
        }
    }
    if (input.bad()) {
        throw std::runtime_error("failed while reading JSON Lines input");
    }
    return cases;
}

std::string FormatSummaryTsv(const Metrics& metrics) {
    std::ostringstream output;
    output << "cases\t" << metrics.counts.cases << '\n'
           << "true_positives\t" << metrics.counts.true_positives << '\n'
           << "false_positives\t" << metrics.counts.false_positives << '\n'
           << "false_negatives\t" << metrics.counts.false_negatives << '\n'
           << "critical_true_positives\t" << metrics.counts.critical_true_positives << '\n'
           << "critical_false_negatives\t" << metrics.counts.critical_false_negatives << '\n'
           << "precision\t" << FormatOptional(metrics.precision) << '\n'
           << "recall\t" << FormatOptional(metrics.recall) << '\n'
           << "f1\t" << FormatOptional(metrics.f1) << '\n'
           << "critical_recall\t" << FormatOptional(metrics.critical_recall) << '\n';
    return output.str();
}

} // namespace pidginspec
