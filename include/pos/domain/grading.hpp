#pragma once

#include "pos/domain/journal.hpp"

namespace pos {

struct Result {
    std::size_t graded;
    std::size_t total;
    std::optional<double> percent;
};

class CompletionPolicy {
public:
    virtual ~CompletionPolicy() = default;
    virtual bool ready(std::size_t graded, std::size_t total) const = 0;
};

class AllCriteria final : public CompletionPolicy {
public:
    bool ready(std::size_t graded, std::size_t total) const override {
        return total > 0 && graded == total;
    }
};

class GradedCriteria final : public CompletionPolicy {
public:
    bool ready(std::size_t graded, std::size_t) const override { return graded > 0; }
};

class GradingService {
public:
    Result calculate(const Journal& journal, Id student, const CompletionPolicy& policy) const;
};

} // namespace pos
