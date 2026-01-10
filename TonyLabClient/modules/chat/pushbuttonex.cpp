#include "pushbuttonex.h"

#include <QResizeEvent>

CPushButtonEx::CPushButtonEx(QWidget* parent)
    : QPushButton(parent)
{
    connect(this, &QPushButton::clicked, this, &CPushButtonEx::SignalClicked);
    connect(this, &QPushButton::clicked, this, &CPushButtonEx::SignalClickedExArea);
}

void CPushButtonEx::SetData(const QVariant& data)
{
    m_data = data;
}

QVariant CPushButtonEx::Data() const
{
    return m_data;
}

void CPushButtonEx::SetAspectRatio(double ratio)
{
    m_aspectRatio = ratio;
    updateGeometry();
}

void CPushButtonEx::resizeEvent(QResizeEvent* event)
{
    QPushButton::resizeEvent(event);

    if (m_aspectRatio <= 0.0) {
        return;
    }

    // Keep a best-effort aspect ratio. Avoid recursion by only adjusting when clearly off.
    const int w = width();
    const int targetH = qMax(1, static_cast<int>(w / m_aspectRatio));
    if (qAbs(height() - targetH) > 2) {
        setFixedHeight(targetH);
    }
}
