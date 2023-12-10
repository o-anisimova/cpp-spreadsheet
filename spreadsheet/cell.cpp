#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>

class Cell::Impl {
public:
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class Cell::EmptyImpl : public Cell::Impl {
public:
    Value GetValue() const override {
        return 0.0;
    }
    std::string GetText() const override {
        return "";
    }
    std::vector<Position> GetReferencedCells() const override {
        return {};
    }
};

class Cell::TextImpl : public Cell::Impl {
public:
    TextImpl(std::string text)
        :text_(text) {
    }
    Value GetValue() const override {
        if (text_[0] == '\'') {
            return text_.substr(1, text_.size() - 1);
        }
        return text_;
    }
    std::string GetText() const override {
        return text_;
    }
    std::vector<Position> GetReferencedCells() const override {
        return {};
    }
private:
    std::string text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    FormulaImpl(Sheet& sheet, std::string text)
        :formula_(ParseFormula(text)), sheet_(GetSheetInterface(sheet)) {

    }
    Value GetValue() const override {
        auto result = formula_->Evaluate(sheet_);
        if (std::holds_alternative<double>(result)) {
            return std::get<double>(result);
        }
        return std::get<FormulaError>(result);
    }
    std::string GetText() const override {
        return '=' + formula_->GetExpression();
    }
    std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }
private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface& sheet_;
};

class Cell::NumberImpl : public Cell::Impl {
public:
    NumberImpl(std::string value)
        :value_(value) {
    }

    Value GetValue() const override {
        return std::stod(value_);
    }
    std::string GetText() const override {
        return value_;
    }
    std::vector<Position> GetReferencedCells() const override {
        return {};
    }
private:
    std::string value_;
};

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet, std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>(EmptyImpl{});
    }
    else if (text[0] == '=' && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(sheet, text.substr(1, text.size() - 1));
    }
    else if (text.find_first_not_of("0123456789") == text.npos) {
        impl_ = std::make_unique<NumberImpl>(text);
    }
    else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

Cell::~Cell() {
}

Cell::Value Cell::GetValue() const {
    try {
        return impl_->GetValue();
    }
    catch (const FormulaError& exc) {
        return exc;
    }
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}
