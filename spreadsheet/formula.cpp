#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
    :ast_(ParseFormulaAST(expression)){
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        return ast_.Execute(sheet);
    }

    std::string GetExpression() const override {
        std::stringstream st;
        ast_.PrintFormula(st);
        return st.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::forward_list<Position> cells = ast_.GetCells();
        std::vector<Position> result = { cells.begin(), cells.end() };
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}