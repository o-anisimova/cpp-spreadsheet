#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <iostream>

using namespace std::literals;

const SheetInterface& GetSheetInterface(Sheet& sheet) {
    return sheet;
}

Sheet::~Sheet() {}

void Sheet::CheckPosValid(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }
}

void Sheet::ResizeSheet(Position pos) {
    if (int(cells_.size()) < pos.row + 1) {
        cells_.resize(pos.row + 1);
        max_printable_size_.rows = pos.row + 1;
    }
    if (int(cells_[0].size()) < pos.col + 1) {
        for (size_t i = 0; i < cells_.size(); ++i) {
            cells_[i].resize(pos.col + 1);
        }
        max_printable_size_.cols = pos.col + 1;
    }
    else {
        for (size_t i = 0; i < cells_.size(); ++i) {
            cells_[i].resize(cells_[0].size());
        }
    }
}

void Sheet::SetMaxPrintableSize() {
    max_printable_size_ = { 0, 0 };
    for (int i = 0; i < int(cells_.size()); ++i) {
        for (int j = 0; j < int(cells_[i].size()); ++j) {
            if (cells_[i][j] != nullptr) {
                if (i + 1 > max_printable_size_.rows) {
                    max_printable_size_.rows = i + 1;
                }
                if (j + 1 > max_printable_size_.cols) {
                    max_printable_size_.cols = j + 1;
                }
            }
        }
    }
}

void Sheet::CheckCircularReferences(const CellInterface* cell, std::unordered_set<Position, PositionHasher>& passed_cells) const {
    for (Position ref_pos : cell->GetReferencedCells()) {
        if (passed_cells.count(ref_pos) > 0) {
            throw CircularDependencyException("Cell cuicular dependency found"s);
        }
        passed_cells.insert(ref_pos);
        if (IsPosInSheet(ref_pos)) {
            CheckCircularReferences(GetCell(ref_pos), passed_cells);
        }
    }
}

void Sheet::CheckCircularReferences(const CellInterface* cell, Position pos) const {
    std::unordered_set<Position, PositionHasher> passed_cells;
    passed_cells.insert(pos);
    CheckCircularReferences(cell, passed_cells);
}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPosValid(pos);
    ResizeSheet(pos);
    std::unique_ptr<Cell> cell_ptr = std::make_unique<Cell>(*this, text);
    //Проверяем кольцевые зависимости
    CheckCircularReferences(cell_ptr.get(), pos);
    cells_[pos.row][pos.col] = std::move(cell_ptr);
    for (Position ref_pos : cells_[pos.row][pos.col]->GetReferencedCells()) {
        //Если ячейка, на которую ссылаемся - пустая, инициализируем ячейку
        //Это нужно, чтобы потом записать обратные зависимости
        if (GetCell(ref_pos) == nullptr) {
            SetCell(ref_pos, "");
        }
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPosValid(pos);
    return cells_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPosValid(pos);
    if (pos.row >= max_printable_size_.rows || pos.col >= max_printable_size_.cols) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    CheckPosValid(pos);
    if (pos.row >= max_printable_size_.rows || pos.col >= max_printable_size_.cols) {
        return;
    }
    cells_[pos.row][pos.col].release();
    SetMaxPrintableSize();
}

Size Sheet::GetPrintableSize() const {
    return max_printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < max_printable_size_.rows; ++i) {
        for (int j = 0; j < max_printable_size_.cols; ++j) {
            if (cells_[i][j] != nullptr) {
                auto val = cells_[i][j]->GetValue();
                if (std::holds_alternative<double>(val)) {
                    output << std::get<double>(val);
                }
                else if (std::holds_alternative<std::string>(val)) {
                    output << std::get<std::string>(val);
                }
                else if (std::holds_alternative<FormulaError>(val)) {
                    output << std::get<FormulaError>(val);
                }

            }
            if (j != max_printable_size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < max_printable_size_.rows; ++i) {
        for (int j = 0; j < max_printable_size_.cols; ++j) {
            if (cells_[i][j] != nullptr) {
                output << cells_[i][j]->GetText();
            }
            if (j != max_printable_size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

bool Sheet::IsPosInSheet(Position pos) const {
    if (pos.row + 1 > max_printable_size_.rows || pos.col + 1 > max_printable_size_.cols) {
        return false;
    }
    return GetCell(pos) != nullptr;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}