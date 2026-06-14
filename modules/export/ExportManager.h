#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <QList>
#include <QString>
#include <QTextDocument>
#include <QtPrintSupport/QPrinter>

#include "../common/WordEntry.h"

/**
 * @brief 生词表导出管理器，支持 CSV、PDF、DOC（HTML包装）和 JSON 格式。
 */
class ExportManager
{
public:
    /// 导出为 CSV 文件（UTF-8 with BOM）
    static bool exportToCsv(const QList<WordEntry> &words,
                            const QString &filePath,
                            const QString &title = QString());

    /// 导出为 PDF 文件（基于 HTML 渲染）
    static bool exportToPdf(const QList<WordEntry> &words,
                            const QString &filePath,
                            const QString &title = QString());

    /// 导出为 Microsoft Word 文档（.doc，实为 HTML 包装）
    static bool exportToDoc(const QList<WordEntry> &words,
                            const QString &filePath,
                            const QString &title = QString());

    /// 导出为 JSON 文件（结构化数据）
    static bool exportToJson(const QList<WordEntry> &words,
                             const QString &filePath);

private:
    /// 生成包含单词表格的 HTML 字符串（供 PDF/DOC 复用）
    static QString generateHtmlContent(const QList<WordEntry> &words,
                                       const QString &title);
};

#endif // EXPORTMANAGER_H