#ifndef DATABASEMANAGEMENTWINDOW_H
#define DATABASEMANAGEMENTWINDOW_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QItemSelectionModel>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QTimer>
#include "DebugTool.h"

/**
 * @brief 数据库管理窗口，提供表数据查看、编辑、搜索、导入/导出功能。
 */
class DatabaseManagementWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseManagementWindow(const QSqlDatabase &db, QWidget *parent = nullptr);
    ~DatabaseManagementWindow();

private slots:
    void onTableChanged(int index);          ///< 切换数据表
    void onSearchTextChanged(const QString &text);  ///< 搜索文本变化
    void onPrevMatch();                      ///< 上一个匹配项
    void onNextMatch();                      ///< 下一个匹配项
    void onInsertRow();                      ///< 插入新行
    void onDeleteRow();                      ///< 删除选中的行
    void onImportCsv();                      ///< 从 CSV 文件导入数据
    void onExportCsv();                      ///< 导出数据为 CSV
    void onExportJson();                     ///< 导出数据为 JSON
    void onRefresh();                        ///< 刷新数据

private:
    void setupUI();
    void setupTableModel(const QString &tableName);
    void updateStatusBar();
    void updateSearchNavigation(bool visible);
    void performSearch();
    void setButtonsEnabled(bool enabled);

    QSqlDatabase m_db;
    QSqlTableModel *m_model;
    QTableView *m_view;
    QComboBox *m_tableCombo;
    QLineEdit *m_searchEdit;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_clearSearchBtn;
    QLabel *m_searchResultLabel;
    QStatusBar *m_statusBar;
    QLabel *m_recordCountLabel;

    QTimer *m_searchTimer;
    QList<int> m_matchedRows;
    int m_currentMatchIndex;
    QString m_lastSearchText;

    QStringList m_tables;
};

#endif // DATABASEMANAGEMENTWINDOW_H