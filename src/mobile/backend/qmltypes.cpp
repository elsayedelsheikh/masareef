#include "backend/qmltypes.h"

#include "core/recurrence.h"

#include <QtQml/qqml.h>

namespace MasareefQml {

void registerTypes()
{
    qmlRegisterUncreatableMetaObject(RecurrenceNS::staticMetaObject,
                                     "Masareef", 1, 0, "Recurrence",
                                     QStringLiteral("Recurrence is an enum namespace"));
}

} // namespace MasareefQml
