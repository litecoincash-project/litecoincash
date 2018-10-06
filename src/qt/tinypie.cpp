// LitecoinCash: Hexagon pie :)

#include <qt/tinypie.h>

TinyPie::TinyPie(QWidget *parent) : QWidget(parent) {
    normalisedVal = 0;
    foregroundCol = QColor(247,213,33);
    backgroundCol = QColor(42,182,67);
    borderCol = Qt::white;
}

void TinyPie::paintEvent(QPaintEvent *event) {
    QPainter painter;
    painter.begin(this);
    painter.setRenderHints(QPainter::Antialiasing, true);

    // Background
    painter.setPen(QPen(backgroundCol, 1));
    painter.setBrush(backgroundCol);
    painter.drawEllipse(0, 0, width(), height());

    // Filled slice
    if (normalisedVal > 0) {
        double localVal = normalisedVal;
        if (localVal > 1.0)
            localVal = 1.0;
        painter.setPen(QPen(foregroundCol, 1));
        painter.setBrush(foregroundCol);
        painter.drawPie(0, 0, width(), height(), 90 * 16, -localVal * 16 * 360);
    }

    // Hex borders
    painter.setPen(QPen(borderCol, 4));
    QColor trans = Qt::black;
    trans.setAlphaF(0);
    painter.setBrush(trans);
    QPainterPath path = HexPath(0.011, width()/2, height()/2);
    painter.drawPath(path);

    painter.setPen(QPen(Qt::black, 1));
    path = HexPath(0.009, width()/2, height()/2);
    painter.drawPath(path);

    QWidget::paintEvent(event);
    painter.end();
}

QPainterPath TinyPie::HexPath(double scale, double centerX, double centerY) {
    double a = 500 * scale,
        b = 866 * scale;

    QPainterPath path;

    path.moveTo(centerX, centerY - 2 * a);
    path.lineTo(centerX - b, centerY - a);
    path.lineTo(centerX - b, centerY + a);
    path.lineTo(centerX, centerY + 2 * a);
    path.lineTo(centerX + b, centerY + a);
    path.lineTo(centerX + b, centerY - a);
    path.lineTo(centerX, centerY - 2 * a);

    return path;
}