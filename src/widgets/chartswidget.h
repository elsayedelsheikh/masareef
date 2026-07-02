#pragma once

#include <QWidget>

class QChartView;

// Two charts, rebuilt on refresh(): a 12-month spending bar chart and a
// current-month category pie (slice colors follow the category colors).
class ChartsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartsWidget(QWidget* parent = nullptr);

    void refresh();

private:
    void rebuildBarChart();
    void rebuildPieChart();

    QChartView* m_barView = nullptr;
    QChartView* m_pieView = nullptr;
};
