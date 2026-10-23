#include "pidginspec/score.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int Run(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) != "score") {
        std::cerr << "usage: pidginspec score --input reviewed-cases.jsonl [--output summary.tsv]\n";
        return 2;
    }
    std::string input_path;
    std::string output_path;
    for (int i = 2; i < argc; ++i) {
        const std::string option = argv[i];
        if ((option == "--input" || option == "--output") && i + 1 < argc) {
            if (option == "--input") {
                input_path = argv[++i];
            } else {
                output_path = argv[++i];
            }
        } else {
            std::cerr << "pidginspec: unknown or incomplete option\n";
            return 2;
        }
    }
    if (input_path.empty()) {
        std::cerr << "pidginspec: --input is required\n";
        return 2;
    }
    try {
        std::ifstream input(input_path);
        if (!input) {
            throw std::runtime_error("could not open input file");
        }
        const auto cases = pidginspec::ReadJsonLines(input);
        const auto metrics = pidginspec::CalculateMetrics(pidginspec::ScoreCases(cases));
        const auto summary = pidginspec::FormatSummaryTsv(metrics);
        if (output_path.empty()) {
            std::cout << summary;
        } else {
            std::ofstream output(output_path);
            if (!output) {
                throw std::runtime_error("could not open output file");
            }
            output << summary;
            if (!output) {
                throw std::runtime_error("could not write output file");
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "pidginspec: " << error.what() << '\n';
        return 2;
    }
}

} // namespace

int main(int argc, char** argv) {
    return Run(argc, argv);
}
