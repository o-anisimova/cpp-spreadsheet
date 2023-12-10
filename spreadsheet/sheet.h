#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_set>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами
    bool IsPosInSheet(Position pos) const override;

private:
	// Можете дополнить ваш класс нужными полями и методами
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    Size max_printable_size_;

    void CheckPosValid(Position pos) const;
    void ResizeSheet(Position pos);
    void SetMaxPrintableSize();
    void CheckCircularReferences(const CellInterface* cell, Position pos) const;
    void CheckCircularReferences(const CellInterface* cell, std::unordered_set<Position, PositionHasher>& passed_cells) const;
};
