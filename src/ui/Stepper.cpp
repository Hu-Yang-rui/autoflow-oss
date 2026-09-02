#include "Stepper.h"
#include "IconButton.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QStyle>

namespace autoflow {

Stepper::Stepper(QWidget* parent) : QWidget(parent) {
    m_minus = new IconButton(IconButton::Minus, this);
    m_plus = new IconButton(IconButton::Plus, this);

    m_edit = new QLineEdit(this);
    m_edit->setAlignment(Qt::AlignCenter);
    m_edit->setValidator(new QDoubleValidator(-1000000.0, 1000000.0, 4, m_edit));

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);   // 按钮与数值之间 4px 间距
    lay->addWidget(m_minus);
    lay->addWidget(m_edit, 1);
    lay->addWidget(m_plus);

    setFixedHeight(30);

    connect(m_minus, &QAbstractButton::clicked, this, [this] {
        setValue(m_value - m_step);
        setError(false);
        emit rangeErrorChanged(false);
        emit valueChanged(m_value);
    });
    connect(m_plus, &QAbstractButton::clicked, this, [this] {
        setValue(m_value + m_step);
        setError(false);
        emit rangeErrorChanged(false);
        emit valueChanged(m_value);
    });
    connect(m_edit, &QLineEdit::editingFinished, this, &Stepper::commitEdit);

    setValue(0);
}

void Stepper::setRange(double min, double max) {
    m_min = min;
    m_max = max;
    setValue(m_value);
}

void Stepper::setDecimals(int d) {
    m_decimals = d;
    delete m_edit->validator();
    m_edit->setValidator(new QDoubleValidator(m_min, m_max, d, m_edit));
    setValue(m_value);
}

void Stepper::setSingleStep(double step) { m_step = step; }

QString Stepper::displayValue(double v) const {
    if (m_decimals <= 0) return QString::number((qlonglong)qRound(v));
    return QString::number(v, 'f', m_decimals);
}

void Stepper::setValue(double v) {
    m_value = qBound(m_min, v, m_max);
    m_edit->setText(displayValue(m_value));
}

void Stepper::commitEdit() {
    bool ok = false;
    double v = m_edit->text().toDouble(&ok);
    if (!ok) {
        // 非数值输入：回退显示并标红提示
        m_edit->setText(displayValue(m_value));
        setError(true);
        emit rangeErrorChanged(true);
        return;
    }
    const bool outOfRange = (v < m_min || v > m_max);
    v = qBound(m_min, v, m_max);
    if (v != m_value) {
        m_value = v;
        emit valueChanged(m_value);
    }
    m_edit->setText(displayValue(m_value));
    setError(outOfRange);
    emit rangeErrorChanged(outOfRange);
}

void Stepper::setError(bool on) {
    m_rangeError = on;
    const QColor err = Palette::stop(ThemeManager::instance().effectiveDark());
    m_edit->setStyleSheet(on ? QStringLiteral("border: 1px solid %1;").arg(err.name()) : QString());
    m_edit->style()->unpolish(m_edit);
    m_edit->style()->polish(m_edit);
    update();
}

} // namespace autoflow
