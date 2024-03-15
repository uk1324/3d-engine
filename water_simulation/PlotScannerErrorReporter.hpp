#pragma once

#include <dependencies/math-compiler/src/scannerMessageReporter.hpp>
#include <vector>

struct ListScannerMessageReporter : ScannerMessageReporter {
	void onError(const ScannerError& error) override;

	std::vector<ScannerError> errors;
};