#include "widgets/chartswidget.h"

#include "models/expensemodel.h"
#include "utils/appconfig.h"
#include "utils/palette.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QPieSeries>
#include <QPieSlice>
#include <QValueAxis>

#include <cmath>

namespace {

QChart* makeChart(const QString& title)
{
    auto* chart = new QChart;
    chart->setTitle(title);
    chart->setBackgroundBrush(QColor(Palette::kSurface));
    chart->setTitleBrush(QColor(Palette::kPrimaryInk));
    chart->setAnimationOptions(QChart::NoAnimation);
    return chart;
}

void styleAxis(QAbstractAxis* axis)
{
    axis->setLabelsBrush(QColor(Palette::kMutedInk));
    axis->setLinePen(QPen(QColor(Palette::kAxisLine), 1));
    axis->setGridLinePen(QPen(QColor(Palette::kGridline), 1));
    axis->setTruncateLabels(false); // Qt Charts elides labels by default
}

} // namespace

ChartsWidget::ChartsWidget(QWidget* parent)
    : QWidget(parent)
{
    m_barView = new QChartView(this);
    m_pieView = new QChartView(this);
    m_barView->setRenderHint(QPainter::Antialiasing);
    m_pieView->setRenderHint(QPainter::Antialiasing);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(m_barView, 3);
    layout->addWidget(m_pieView, 2);
}

void ChartsWidget::refresh()
{
    rebuildBarChart();
    rebuildPieChart();
}

void ChartsWidget::rebuildBarChart()
{
    const QList<ExpenseModel::MonthTotal> months = ExpenseModel::monthlyTotals(12);

    auto* set = new QBarSet(tr("Spending"));
    set->setColor(QColor(Palette::kCategorical[0]));
    set->setBorderColor(QColor(Palette::kSurface));

    QStringList labels;
    double maxValue = 0.0;
    for (const auto& month : months) {
        const double value = double(month.total) / 100.0;
        *set << value;
        maxValue = qMax(maxValue, value);
        labels << month.month.toString(QStringLiteral("MMM yy"));
    }

    auto* series = new QBarSeries;
    series->append(set);
    series->setBarWidth(0.6);

    QChart* chart = makeChart(tr("Monthly spending — last 12 months (%1)")
                                  .arg(AppConfig::currencySymbol()));
    chart->addSeries(series);
    chart->legend()->setVisible(false); // single series: the title names it

    auto* xAxis = new QBarCategoryAxis;
    xAxis->append(labels);
    styleAxis(xAxis);
    xAxis->setGridLineVisible(false);
    xAxis->setLabelsAngle(-60); // 12 labels don't fit horizontally
    chart->addAxis(xAxis, Qt::AlignBottom);
    series->attachAxis(xAxis);

    auto* yAxis = new QValueAxis;
    yAxis->setRange(0, maxValue > 0 ? maxValue * 1.1 : 100.0);
    yAxis->setLabelFormat(QStringLiteral("%.0f"));
    styleAxis(yAxis);
    chart->addAxis(yAxis, Qt::AlignLeft);
    series->attachAxis(yAxis);

    QChart* previous = m_barView->chart();
    m_barView->setChart(chart);
    delete previous;
}

void ChartsWidget::rebuildPieChart()
{
    const QDate today = QDate::currentDate();
    const QDate monthStart(today.year(), today.month(), 1);
    const QDate monthEnd = monthStart.addMonths(1).addDays(-1);
    const QList<ExpenseModel::CategoryTotal> totals =
        ExpenseModel::totalsByCategory(monthStart, monthEnd);

    QChart* chart = makeChart(tr("This month by category"));

    if (totals.isEmpty()) {
        chart->setTitle(tr("This month by category — no expenses yet"));
    } else {
        qint64 grandTotal = 0;
        for (const auto& t : totals)
            grandTotal += t.total;

        auto* series = new QPieSeries;
        series->setPieSize(0.62);
        for (const auto& t : totals) {
            QPieSlice* slice = series->append(t.name, double(t.total) / 100.0);
            if (!t.color.isEmpty())
                slice->setColor(QColor(t.color));
            slice->setBorderColor(QColor(Palette::kSurface));
            slice->setBorderWidth(2);
            // Direct labels: identity + value never rely on color alone
            slice->setLabelVisible(true);
            const int percent =
                grandTotal > 0 ? int(std::round(100.0 * double(t.total) / double(grandTotal))) : 0;
            slice->setLabel(QStringLiteral("%1 %2%").arg(t.name).arg(percent));
        }
        // Labels inside the big slices (outside ones get clipped at the
        // chart edge); ink picked per slice luminance
        const auto slices = series->slices();
        for (QPieSlice* slice : slices) {
            if (slice->percentage() >= 0.10) {
                slice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
                slice->setLabelBrush(qGray(slice->color().rgb()) > 140
                                         ? QColor(Palette::kPrimaryInk)
                                         : QColor(Qt::white));
            } else {
                slice->setLabelPosition(QPieSlice::LabelOutside);
                slice->setLabelBrush(QColor(Palette::kPrimaryInk));
            }
        }
        chart->addSeries(series);
        chart->legend()->setVisible(false); // slices carry direct labels
    }

    QChart* previous = m_pieView->chart();
    m_pieView->setChart(chart);
    delete previous;
}
