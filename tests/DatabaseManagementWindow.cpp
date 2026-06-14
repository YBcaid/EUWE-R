#include "DatabaseManagementWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QDebug>

DatabaseManagementWindow::DatabaseManagementWindow(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), m_db(db), m_model(nullptr), m_currentMatchIndex(-1)
{
    setWindowTitle("数据库管理");
    resize(1100, 600);
    setStyleSheet(
        "QWidget { background-color: #f5f5f5; }"
        "QTableView { background-color: white; alternate-background-color: #f9f9f9; "
        "selection-background-color: #e3f2fd; selection-color: #1976d2; gridline-color: #e0e0e0; font-size: 13px; }"
        "QTableView::item:selected { background-color: #bbdefb; color: #0d47a1; }"
        "QHeaderView::section { background-color: #e0e0e0; padding: 8px; border: none; font-weight: bold; font-size: 13px; }"
        "QPushButton { border-radius: 4px; padding: 8px 12px; font-size: 13px; font-weight: 500; }"
        "QLineEdit { padding: 6px; border: 2px solid #ddd; border-radius: 4px; background-color: white; font-size: 13px; }"
        "QLineEdit:focus { border-color: #2196F3; }"
        "QLabel { color: #333; }"
        );

    m_tables << "Words" << "stardict";
    setupUI();
    setButtonsEnabled(true);
}

DatabaseManagementWindow::~DatabaseManagementWindow()
{
    if (m_model) {
        delete m_model;
    }
}

void DatabaseManagementWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QToolBar *toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setStyleSheet(
        "QToolBar { background-color: #FFFFFF; padding: 8px 20px; spacing: 15px; border-bottom: 1px solid #EEEEEE; }"
        "QToolButton { color: #333333; background-color: transparent; border: none; border-radius: 4px; padding: 6px 12px; }"
        "QToolButton:hover { background-color: #F5F5F5; }"
        "QToolBar::separator { width: 1px; background-color: #DDDDDD; margin: 5px 0; }"
        );

    QLabel *titleLabel = new QLabel("📊 数据库管理");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 500; padding: 0 5px;");
    toolbar->addWidget(titleLabel);
    toolbar->addSeparator();

    QLabel *tableLabel = new QLabel("表:");
    tableLabel->setStyleSheet("padding: 0 5px;");
    toolbar->addWidget(tableLabel);
    m_tableCombo = new QComboBox();
    m_tableCombo->addItems(m_tables);
    m_tableCombo->setFixedWidth(120);
    connect(m_tableCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DatabaseManagementWindow::onTableChanged);
    toolbar->addWidget(m_tableCombo);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍 快速搜索...");
    m_searchEdit->setFixedWidth(220);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: #F5F5F5; color: #333333; border: 1px solid #E0E0E0; border-radius: 4px; }"
        "QLineEdit:focus { border: 1px solid #2196F3; background-color: #FAFAFA; }"
        );
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DatabaseManagementWindow::onSearchTextChanged);
    toolbar->addWidget(m_searchEdit);

    m_prevBtn = new QPushButton("◀");
    m_prevBtn->setFixedSize(28, 28);
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setVisible(false);
    m_prevBtn->setToolTip("上一个匹配 (Shift+F3)");
    m_prevBtn->setStyleSheet("QPushButton { background-color: #E0E0E0; color: #333; border-radius: 4px; }"
                             "QPushButton:hover { background-color: #BDBDBD; }");
    connect(m_prevBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onPrevMatch);
    toolbar->addWidget(m_prevBtn);

    m_nextBtn = new QPushButton("▶");
    m_nextBtn->setFixedSize(28, 28);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setVisible(false);
    m_nextBtn->setToolTip("下一个匹配 (F3)");
    m_nextBtn->setStyleSheet("QPushButton { background-color: #E0E0E0; color: #333; border-radius: 4px; }"
                             "QPushButton:hover { background-color: #BDBDBD; }");
    connect(m_nextBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onNextMatch);
    toolbar->addWidget(m_nextBtn);

    m_clearSearchBtn = new QPushButton("✕");
    m_clearSearchBtn->setFixedSize(22, 22);
    m_clearSearchBtn->setCursor(Qt::PointingHandCursor);
    m_clearSearchBtn->setVisible(false);
    m_clearSearchBtn->setStyleSheet(
        "QPushButton { background-color: #BDBDBD; color: white; border: none; border-radius: 11px; font-size: 12px; font-weight: bold; margin-left: 5px; }"
        "QPushButton:hover { background-color: #757575; }");
    connect(m_clearSearchBtn, &QPushButton::clicked, [this]() {
        m_searchEdit->clear();
        m_searchEdit->setFocus();
    });
    toolbar->addWidget(m_clearSearchBtn);

    mainLayout->addWidget(toolbar);

    QWidget *mainContent = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(mainContent);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(10);

    m_view = new QTableView(this);
    m_view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->setAlternatingRowColors(true);
    m_view->verticalHeader()->setVisible(false);
    m_view->setSortingEnabled(true);
    m_view->horizontalHeader()->setStretchLastSection(true);
    contentLayout->addWidget(m_view, 3);

    QWidget *rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(280);
    rightPanel->setStyleSheet("background-color: #f8f9fa; border-left: 2px solid #dee2e6;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(15, 20, 15, 20);
    rightLayout->setSpacing(12);

    QLabel *opLabel = new QLabel("操作");
    opLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #495057;");
    opLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(opLabel);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #dee2e6; max-height: 2px; margin: 5px 0;");
    rightLayout->addWidget(line);

    QPushButton *insertBtn = new QPushButton("➕ 插入新行");
    insertBtn->setMinimumHeight(40);
    insertBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }");
    connect(insertBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onInsertRow);
    rightLayout->addWidget(insertBtn);

    QPushButton *deleteBtn = new QPushButton("❌ 删除选中行");
    deleteBtn->setMinimumHeight(40);
    deleteBtn->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #da190b; }"
        "QPushButton:pressed { background-color: #b71c1c; }");
    connect(deleteBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onDeleteRow);
    rightLayout->addWidget(deleteBtn);

    QPushButton *refreshBtn = new QPushButton("🔄 刷新");
    refreshBtn->setMinimumHeight(40);
    refreshBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #455A64; }");
    connect(refreshBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onRefresh);
    rightLayout->addWidget(refreshBtn);

    rightLayout->addSpacing(10);
    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("background-color: #dee2e6; max-height: 2px; margin: 5px 0;");
    rightLayout->addWidget(line2);

    QLabel *importExportLabel = new QLabel("导入/导出");
    importExportLabel->setStyleSheet("font-weight: bold; color: #495057; padding-top: 5px;");
    rightLayout->addWidget(importExportLabel);

    QPushButton *importCsvBtn = new QPushButton("📄 导入CSV");
    importCsvBtn->setMinimumHeight(40);
    importCsvBtn->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #7B1FA2; }"
        "QPushButton:pressed { background-color: #6A1B9A; }");
    connect(importCsvBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onImportCsv);
    rightLayout->addWidget(importCsvBtn);

    QPushButton *exportCsvBtn = new QPushButton("📤 导出CSV");
    exportCsvBtn->setMinimumHeight(40);
    exportCsvBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #e68900; }"
        "QPushButton:pressed { background-color: #cc7c00; }");
    connect(exportCsvBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onExportCsv);
    rightLayout->addWidget(exportCsvBtn);

    QPushButton *exportJsonBtn = new QPushButton("📋 导出JSON");
    exportJsonBtn->setMinimumHeight(40);
    exportJsonBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #546E7A; }"
        "QPushButton:pressed { background-color: #455A64; }");
    connect(exportJsonBtn, &QPushButton::clicked, this, &DatabaseManagementWindow::onExportJson);
    rightLayout->addWidget(exportJsonBtn);

    rightLayout->addStretch();
    contentLayout->addWidget(rightPanel);

    mainLayout->addWidget(mainContent, 1);

    m_statusBar = new QStatusBar(this);
    m_statusBar->setStyleSheet("QStatusBar { background-color: #e0e0e0; padding: 5px; }");
    m_recordCountLabel = new QLabel();
    m_statusBar->addWidget(m_recordCountLabel);
    m_searchResultLabel = new QLabel();
    m_searchResultLabel->setAlignment(Qt::AlignRight);
    m_statusBar->addPermanentWidget(m_searchResultLabel);
    mainLayout->addWidget(m_statusBar);

    onTableChanged(0);

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300);
    connect(m_searchTimer, &QTimer::timeout, this, &DatabaseManagementWindow::performSearch);
}

void DatabaseManagementWindow::setupTableModel(const QString &tableName)
{
    if (m_model) {
        delete m_model;
    }
    m_model = new QSqlTableModel(this, m_db);
    m_model->setTable(tableName);
    m_model->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!m_model->select()) {
        QMessageBox::critical(this, "错误", "无法加载表 " + tableName + ":\n" + m_model->lastError().text());
        setButtonsEnabled(false);
        return;
    }
    setButtonsEnabled(true);
    m_view->setModel(m_model);
    m_view->resizeColumnsToContents();
    int idCol = -1;
    for (int i = 0; i < m_model->columnCount(); ++i) {
        if (m_model->headerData(i, Qt::Horizontal).toString().toLower() == "id") {
            idCol = i;
            break;
        }
    }
    if (idCol != -1) {
        m_view->hideColumn(idCol);
    }
    if (tableName == "Words") {
        m_view->setColumnWidth(1, 180);
        m_view->setColumnWidth(2, 300);
        m_view->setColumnWidth(3, 150);
    } else if (tableName == "stardict") {
        m_view->setColumnWidth(1, 180);
        m_view->setColumnWidth(2, 300);
        m_view->setColumnWidth(3, 150);
    }
    updateStatusBar();
    updateSearchNavigation(false);
}

void DatabaseManagementWindow::onTableChanged(int index)
{
    if (index < 0 || index >= m_tables.size()) return;
    QString tableName = m_tables[index];
    setupTableModel(tableName);
    m_searchEdit->clear();
    m_lastSearchText.clear();
    m_matchedRows.clear();
    m_currentMatchIndex = -1;
    updateSearchNavigation(false);
}

void DatabaseManagementWindow::onSearchTextChanged(const QString &text)
{
    m_searchTimer->start();
    m_clearSearchBtn->setVisible(!text.isEmpty());
    if (text.isEmpty()) {
        performSearch();
    }
}

void DatabaseManagementWindow::performSearch()
{
    QString searchText = m_searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        for (int row = 0; row < m_model->rowCount(); ++row) {
            m_view->setRowHidden(row, false);
        }
        m_matchedRows.clear();
        m_currentMatchIndex = -1;
        m_lastSearchText.clear();
        updateSearchNavigation(false);
        m_searchResultLabel->setText("");
        return;
    }

    if (searchText == m_lastSearchText && !m_matchedRows.isEmpty()) {
        onNextMatch();
        return;
    }

    m_lastSearchText = searchText;
    m_matchedRows.clear();
    m_currentMatchIndex = -1;

    for (int row = 0; row < m_model->rowCount(); ++row) {
        bool matched = false;
        for (int col = 0; col < m_model->columnCount(); ++col) {
            QVariant data = m_model->data(m_model->index(row, col));
            if (data.toString().contains(searchText, Qt::CaseInsensitive)) {
                matched = true;
                break;
            }
        }
        m_view->setRowHidden(row, !matched);
        if (matched) {
            m_matchedRows.append(row);
        }
    }

    if (!m_matchedRows.isEmpty()) {
        m_currentMatchIndex = 0;
        int firstRow = m_matchedRows.first();
        m_view->selectRow(firstRow);
        m_view->scrollTo(m_model->index(firstRow, 0), QAbstractItemView::PositionAtCenter);
        m_searchResultLabel->setText(QString("找到 %1 个匹配").arg(m_matchedRows.size()));
        updateSearchNavigation(true);
    } else {
        m_searchResultLabel->setText("未找到匹配项");
        updateSearchNavigation(false);
    }
}

void DatabaseManagementWindow::onPrevMatch()
{
    if (m_matchedRows.isEmpty()) return;
    m_currentMatchIndex = (m_currentMatchIndex - 1 + m_matchedRows.size()) % m_matchedRows.size();
    int row = m_matchedRows[m_currentMatchIndex];
    m_view->selectRow(row);
    m_view->scrollTo(m_model->index(row, 0), QAbstractItemView::PositionAtCenter);
    m_searchResultLabel->setText(QString("匹配 %1/%2").arg(m_currentMatchIndex + 1).arg(m_matchedRows.size()));
}

void DatabaseManagementWindow::onNextMatch()
{
    if (m_matchedRows.isEmpty()) return;
    m_currentMatchIndex = (m_currentMatchIndex + 1) % m_matchedRows.size();
    int row = m_matchedRows[m_currentMatchIndex];
    m_view->selectRow(row);
    m_view->scrollTo(m_model->index(row, 0), QAbstractItemView::PositionAtCenter);
    m_searchResultLabel->setText(QString("匹配 %1/%2").arg(m_currentMatchIndex + 1).arg(m_matchedRows.size()));
}

void DatabaseManagementWindow::onInsertRow()
{
    if (!m_model) return;
    int row = m_model->rowCount();
    m_model->insertRow(row);
    m_view->scrollToBottom();
    m_view->selectRow(row);
    m_view->setFocus();
}

void DatabaseManagementWindow::onDeleteRow()
{
    if (!m_model) return;
    QItemSelectionModel *selection = m_view->selectionModel();
    QModelIndexList indexes = selection->selectedRows();
    if (indexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选中要删除的行");
        return;
    }
    if (QMessageBox::question(this, "确认删除", "确定要删除选中的行吗？",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    // 从后往前删除避免索引变化
    QList<int> rows;
    for (const QModelIndex &idx : indexes) {
        rows.append(idx.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows) {
        m_model->removeRow(row);
    }
    if (!m_model->submitAll()) {
        QMessageBox::warning(this, "错误", "删除失败: " + m_model->lastError().text());
        m_model->revertAll();
    } else {
        updateStatusBar();
    }
}

void DatabaseManagementWindow::onImportCsv()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择CSV文件", QDir::homePath(), "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件");
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString firstLine = stream.readLine();
    if (firstLine.isEmpty()) {
        QMessageBox::warning(this, "错误", "空文件");
        return;
    }
    QStringList headers = firstLine.split(',');
    QSqlRecord record = m_model->record();
    int success = 0, fail = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList fields = line.split(',');
        if (fields.size() < headers.size()) continue;
        QSqlRecord newRecord = record;
        for (int i = 0; i < headers.size(); ++i) {
            QString fieldName = headers[i];
            if (fieldName == "id") continue;
            int col = record.indexOf(fieldName);
            if (col != -1) {
                newRecord.setValue(col, fields[i]);
            }
        }
        if (m_model->insertRecord(-1, newRecord)) {
            success++;
        } else {
            fail++;
        }
    }
    file.close();
    if (success > 0) {
        m_model->submitAll();
        updateStatusBar();
        QMessageBox::information(this, "导入完成", QString("成功导入 %1 行，失败 %2 行").arg(success).arg(fail));
    } else {
        QMessageBox::warning(this, "导入失败", "没有导入任何数据");
    }
}

void DatabaseManagementWindow::onExportCsv()
{
    if (!m_model) return;
    QString fileName = QFileDialog::getSaveFileName(this, "导出CSV文件", QDir::homePath() + "/export.csv", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QStringList headers;
    for (int col = 0; col < m_model->columnCount(); ++col) {
        QString header = m_model->headerData(col, Qt::Horizontal).toString();
        if (header == "id") continue;
        headers << header;
    }
    stream << headers.join(',') << "\n";
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_model->columnCount(); ++col) {
            if (m_model->headerData(col, Qt::Horizontal).toString() == "id") continue;
            QString value = m_model->data(m_model->index(row, col)).toString();
            if (value.contains(',') || value.contains('"')) {
                value = '"' + value.replace('"', "\"\"") + '"';
            }
            rowData << value;
        }
        stream << rowData.join(',') << "\n";
    }
    file.close();
    QMessageBox::information(this, "导出成功", QString("已导出 %1 条记录").arg(m_model->rowCount()));
}

void DatabaseManagementWindow::onExportJson()
{
    if (!m_model) return;
    QString fileName = QFileDialog::getSaveFileName(this, "导出JSON文件", QDir::homePath() + "/export.json", "JSON文件 (*.json)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "{\n  \"rows\": [\n";
    for (int row = 0; row < m_model->rowCount(); ++row) {
        stream << "    {\n";
        for (int col = 0; col < m_model->columnCount(); ++col) {
            QString header = m_model->headerData(col, Qt::Horizontal).toString();
            if (header == "id") continue;
            QString value = m_model->data(m_model->index(row, col)).toString();
            value.replace('\\', "\\\\").replace('"', "\\\"");
            stream << "      \"" << header << "\": \"" << value << "\"";
            if (col < m_model->columnCount() - 1) stream << ",";
            stream << "\n";
        }
        stream << "    }";
        if (row < m_model->rowCount() - 1) stream << ",";
        stream << "\n";
    }
    stream << "  ]\n}\n";
    file.close();
    QMessageBox::information(this, "导出成功", QString("已导出 %1 条记录").arg(m_model->rowCount()));
}

void DatabaseManagementWindow::onRefresh()
{
    if (m_model) {
        m_model->select();
        updateStatusBar();
    }
}

void DatabaseManagementWindow::updateStatusBar()
{
    if (m_model) {
        m_recordCountLabel->setText(QString("总记录数: %1 条").arg(m_model->rowCount()));
    } else {
        m_recordCountLabel->setText("");
    }
}

void DatabaseManagementWindow::updateSearchNavigation(bool visible)
{
    m_prevBtn->setVisible(visible);
    m_nextBtn->setVisible(visible);
}

void DatabaseManagementWindow::setButtonsEnabled(bool enabled)
{
    Q_UNUSED(enabled);
}