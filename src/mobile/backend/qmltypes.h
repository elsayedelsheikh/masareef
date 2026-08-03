#pragma once

// Registrations the Masareef QML module needs but cannot declare inline.
//
// Everything else in this module registers itself through QML_ELEMENT. The
// Recurrence enum cannot: it lives in masareef_core, which is a plain
// library with no metatype output for qmltyperegistrar to read, so it has
// to be registered by hand. This lives here rather than in the app's
// main() so that anything else loading the module — the QML tests, above
// all — gets the same types the app does.
namespace MasareefQml {

void registerTypes();

} // namespace MasareefQml
