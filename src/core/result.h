#pragma once

#include <QString>

#include <expected>
#include <utility>

// Explicit error handling without out-parameters: a fallible operation
// returns Result<T> and the caller decides what to do with the failure.
// Compared to the classic `bool f(..., QString* error)` shape this makes
// the success value and the error mutually exclusive by construction and
// impossible to ignore accidentally.
struct Error {
    QString message;
};

template <typename T>
using Result = std::expected<T, Error>;

// Shorthand for the failure branch: `return fail(tr("..."))`.
[[nodiscard]] inline std::unexpected<Error> fail(QString message)
{
    return std::unexpected(Error{ std::move(message) });
}
