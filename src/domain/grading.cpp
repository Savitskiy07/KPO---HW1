#include "pos/domain/grading.hpp"

#include <algorithm>

namespace pos {

Result GradingService::calculate(const Journal& journal, Id student,
                                 const CompletionPolicy& policy) const {
    const auto& grades = journal.student(student).grades;
    const auto& criteria = journal.rubric().criteria();
    Result result{0, criteria.size(), std::nullopt};
    double largestWeight = 0;
    for (const auto& criterion : criteria) {
        if (grades[criterion.id - 1]) {
            ++result.graded;
            largestWeight = std::max(largestWeight, criterion.weight);
        }
    }
    if (!policy.ready(result.graded, result.total) || result.graded == 0) {
        return result;
    }

    long double weighted = 0;
    long double weights = 0;
    for (const auto& criterion : criteria) {
        if (const auto& mark = grades[criterion.id - 1]) {
            const long double weight = static_cast<long double>(criterion.weight) / largestWeight;
            weights += weight;
            weighted += weight * (static_cast<long double>(mark->value()) / criterion.maximum);
        }
    }
    result.percent = static_cast<double>(100 * weighted / weights);
    return result;
}

} // namespace pos
