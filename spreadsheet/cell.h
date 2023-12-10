#pragma once

#include "common.h"
#include "formula.h"

#include <unordered_set>
#include <optional>

class Sheet;

const SheetInterface& GetSheetInterface(Sheet& sheet);

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet, std::string text);
    ~Cell();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    class NumberImpl;

    std::unique_ptr<Impl> impl_;
};