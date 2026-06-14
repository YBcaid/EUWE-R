#include "ExportManager.h"

#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <QtPrintSupport/QPrinter>
#include <QDateTime>
#include <QDebug>
#include <QStringConverter>
#include <QString>

static QString escapeCsvField(const QString &field)
{
    if (field.contains(',') || field.contains('"') || field.contains('\n'))
    {
        QString escaped = field;
        escaped.replace(QLatin1Char('"'), QLatin1String("\"\""));
        return QLatin1Char('"') + escaped + QLatin1Char('"');
    }
    return field;
}

bool ExportManager::exportToCsv(const QList<WordEntry> &words,
                                const QString &filePath,
                                const QString &title)
{
    Q_UNUSED(title);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "无法打开文件用于 CSV 导出：" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << QChar(0xFEFF);
    stream << "单词,音标,释义\n";

    for (const WordEntry &w : words)
    {
        QString word = escapeCsvField(w.word);
        QString phonetic = escapeCsvField(w.phonetic.isEmpty() ? "" : w.phonetic);
        QString translation = escapeCsvField(w.translation);
        stream << word << "," << phonetic << "," << translation << "\n";
    }
    file.close();
    return true;
}

QString ExportManager::generateHtmlContent(const QList<WordEntry> &words,
                                           const QString &title)
{
    QString html;
    html += "<html><head><meta charset='UTF-8'><style>";
    html += "body { font-family: 'Microsoft YaHei', 'SimHei', 'Arial', sans-serif; margin: 0.3cm; }";
    html += "h1 { color: #2c3e50; border-bottom: 1px solid #3498db; padding-bottom: 2px; margin: "
            "2px 0; font-size: 9pt; font-weight: bold; }";
    html += "table { width:100%; border-collapse: collapse; font-size: 7.5pt; }";
    html += "th { background-color: #e9ecef; padding: 2px; text-align: left; font-weight: bold; "
            "border-bottom: 1px solid #aaa; }";
    html += "td { padding: 1px 2px; border-bottom: 1px dotted #ccc; }";
    html += ".word { color: #e74c3c; font-weight: bold; }";
    html += ".phonetic { color: #9b59b6; font-style: italic; }";
    html += ".translation { color: #2c3e50; }";
    html += ".count { color: #7f8c8d; font-size: 7pt; margin-left: 5px; font-weight: normal; }";
    html += ".footer { text-align: right; color: #95a5a6; margin-top: 5px; font-size: 6pt; }";
    html += "</style></head><body>";

    QString displayTitle = title.isEmpty() ? "生词本" : title;
    html += QString("<h1>%1 <span class='count'>(共 %2 个单词)</span></h1>")
                .arg(displayTitle)
                .arg(words.size());

    if (!words.isEmpty())
    {
        html += "<table>";
        html += "<tr><th>单词</th><th>音标</th><th>释义</th></tr>";
        for (const WordEntry &w : words)
        {
            QString phonetic = w.phonetic.isEmpty() ? "—" : w.phonetic;
            html += QString("<tr>"
                            "<td class='word'>%1</td>"
                            "<td class='phonetic'>%2</td>"
                            "<td class='translation'>%3</td>"
                            "</tr>")
                        .arg(w.word.toHtmlEscaped())
                        .arg(phonetic.toHtmlEscaped())
                        .arg(w.translation.toHtmlEscaped());
        }
        html += "</table>";
    }
    else
    {
        html += "<p style='font-size:7.5pt; color:#999;'>没有单词记录</p>";
    }

    html += "<div class='footer'>生成时间: " +
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</div>";
    html += "</body></html>";
    return html;
}

bool ExportManager::exportToPdf(const QList<WordEntry> &words,
                                const QString &filePath,
                                const QString &title)
{
    QTextDocument doc;
    doc.setHtml(generateHtmlContent(words, title));
    QFont defaultFont("Microsoft YaHei");
    defaultFont.setPointSizeF(7.5);
    doc.setDefaultFont(defaultFont);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);
    printer.setPageOrientation(QPageLayout::Portrait);
    doc.print(&printer);
    return true;
}

bool ExportManager::exportToDoc(const QList<WordEntry> &words,
                                const QString &filePath,
                                const QString &title)
{
    QString html = generateHtmlContent(words, title);
    html = "<html xmlns:v='urn:schemas-microsoft-com:vml' "
           "xmlns:o='urn:schemas-microsoft-com:office:office' "
           "xmlns:w='urn:schemas-microsoft-com:office:word' "
           "xmlns:m='http://schemas.microsoft.com/office/2004/12/omml' "
           "xmlns='http://www.w3.org/TR/REC-html40'>"
           "<head><meta charset='UTF-8'>"
           "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"
           "<!--[if gte mso 9]><xml>"
           "<w:WordDocument><w:View>Print</w:View><w:Zoom>100</w:Zoom>"
           "<w:HyphenationZone>21</w:HyphenationZone><w:DoNotOptimizeForBrowser/>"
           "</w:WordDocument></xml><![endif]-->"
           "<style>"
           "body { margin: 0.3cm; font-family: 'Microsoft YaHei', 'SimHei', 'Arial', sans-serif; }"
           "h1 { font-size: 9pt; margin: 2px 0; border-bottom: 1px solid #3498db; }"
           "table { border-collapse: collapse; width: 100%; font-size: 7.5pt; }"
           "th { background-color: #e9ecef; padding: 2px; border-bottom: 1px solid #aaa; }"
           "td { padding: 1px 2px; border-bottom: 1px dotted #ccc; }"
           ".word { color: #e74c3c; font-weight: bold; }"
           ".phonetic { color: #9b59b6; font-style: italic; }"
           ".translation { color: #2c3e50; }"
           ".count { color: #7f8c8d; font-size: 7pt; }"
           ".footer { text-align: right; color: #95a5a6; font-size: 6pt; }"
           "</style></head>" +
           html.mid(html.indexOf("<body>"));

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "无法创建 DOC 文件：" << filePath;
        return false;
    }
    file.write(html.toUtf8());
    file.close();
    return true;
}

bool ExportManager::exportToJson(const QList<WordEntry> &words,
                                 const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "无法创建 JSON 文件：" << filePath;
        return false;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "{\n  \"words\": [\n";

    for (int i = 0; i < words.size(); ++i)
    {
        const WordEntry &w = words[i];
        QString wordEscaped = w.word;
        wordEscaped.replace(QLatin1String("\\"), QLatin1String("\\\\"));
        wordEscaped.replace(QLatin1String("\""), QLatin1String("\\\""));
        QString transEscaped = w.translation;
        transEscaped.replace(QLatin1String("\\"), QLatin1String("\\\\"));
        transEscaped.replace(QLatin1String("\""), QLatin1String("\\\""));
        QString phoneticEscaped = w.phonetic;
        phoneticEscaped.replace(QLatin1String("\\"), QLatin1String("\\\\"));
        phoneticEscaped.replace(QLatin1String("\""), QLatin1String("\\\""));

        QString wordJson = QString("    {\n"
                                   "      \"word\": \"%1\",\n"
                                   "      \"translation\": \"%2\",\n"
                                   "      \"phonetic\": \"%3\"\n"
                                   "    }")
                               .arg(wordEscaped)
                               .arg(transEscaped)
                               .arg(phoneticEscaped);
        stream << wordJson;
        if (i < words.size() - 1)
            stream << ",";
        stream << "\n";
    }
    stream << "  ]\n}\n";
    file.close();
    return true;
}