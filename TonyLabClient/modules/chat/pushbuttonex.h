#pragma once

#include <QPushButton>
#include <QVariant>

class CPushButtonEx : public QPushButton
{
    Q_OBJECT

public:
    explicit CPushButtonEx(QWidget* parent = nullptr);

    void SetData(const QVariant& data);
    QVariant Data() const;

    void SetAspectRatio(double ratio);

signals:
    void SignalClicked();
    void SignalClickedExArea();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QVariant m_data;
    double m_aspectRatio = 0.0; // width / height
};
