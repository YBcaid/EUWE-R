#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "DebugTool.h"
#include "DebugWindow.h"

namespace Ui {
class Widget;
}

/**
 * @brief 应用程序主窗口（调试入口）。
 *        实际功能委托给 DebugWindow，自身在构造后立即关闭。
 */
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H