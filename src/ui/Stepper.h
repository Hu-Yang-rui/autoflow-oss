#pragma once
#include <QWidget>

class QLineEdit;

namespace autoflow {

class IconButton;

// 步进器：[−] 数值 [+]（加号在右、减号在左，数值居中，4px 间距）
class Stepper : public QWidget {
    Q_OBJECT
public:
    explicit Stepper(QWidget* parent = nullptr);

    void setRange(double min, double max);
    void setDecimals(int d);
    void setSingleStep(double step);
    void setValue(double v);
    double value() const { return m_value; }
    int decimals() const { return m_decimals; }
    double minimum() const { return m_min; }
    double maximum() const { return m_max; }

    // 越界/非法输入的红色边框标记（供参数面板就地校验使用）
    void setError(bool on);
    bool hasRangeError() const { return m_rangeError; }

signals:
    void valueChanged(double v);
    void rangeErrorChanged(bool on);   // 用户在输入框里键入了越界/非法数值

private:
    void commitEdit();
    QString displayValue(double v) const;

    IconButton* m_minus = nullptr;
    IconButton* m_plus = nullptr;
    QLineEdit* m_edit = nullptr;
    double m_min = -1000000.0;
    double m_max = 1000000.0;
    double m_value = 0.0;
    double m_step = 1.0;
    int m_decimals = 0;
    bool m_rangeError = false;
};

} // namespace autoflow
