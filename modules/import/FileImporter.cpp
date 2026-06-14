#include "FileImporter.h"

#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <QRegularExpression>
#include <QDebug>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QPdfDocument>
#include <QPdfSelection>
#include <QXmlStreamReader>
#include <QtCore/private/qzipreader_p.h>
#include <algorithm>

static QTemporaryDir s_tempDir;

static QString readTxt(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    QString content = stream.readAll();
    file.close();
    return content;
}

static QString readMarkdown(const QString &filePath)
{
    return readTxt(filePath);
}

static QString readHtml(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    QString html = stream.readAll();
    file.close();
    QTextDocument doc;
    doc.setHtml(html);
    return doc.toPlainText();
}

static QString readPdf(const QString &filePath)
{
    QString allText;
    QPdfDocument doc;
    if (doc.load(filePath) != QPdfDocument::Error::None) {
        qWarning() << "无法加载 PDF：" << filePath;
        return allText;
    }

    int pageCount = doc.pageCount();
    for (int i = 0; i < pageCount; ++i) {
        QPdfSelection selection = doc.getAllText(i);
        QString pageText = selection.isValid() ? selection.text() : QString();
        allText += pageText;
        if (i < pageCount - 1)
            allText += '\n';
    }
    doc.close();
    return allText;
}

static QString readDocx(const QString &filePath, QStringList &outImagePaths)
{
    QString allText;
    QZipReader zip(filePath);
    if (!zip.exists()) {
        qWarning() << "无法打开 docx 文件：" << filePath;
        return allText;
    }

    QByteArray docXml = zip.fileData("word/document.xml");
    if (!docXml.isEmpty()) {
        QXmlStreamReader xml(docXml);
        xml.setNamespaceProcessing(false);

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == QLatin1String("w:t") ||
                    xml.name() == QLatin1String("m:t") ||
                    xml.name() == QLatin1String("t")) {
                    allText += xml.readElementText();
                }
            }
            if (xml.isEndElement()) {
                if (xml.name() == QLatin1String("w:p") ||
                    xml.name() == QLatin1String("m:p") ||
                    xml.name() == QLatin1String("p")) {
                    allText += '\n';
                }
            }
        }
    }

    for (const QZipReader::FileInfo &info : zip.fileInfoList()) {
        if (info.filePath.startsWith("word/media/") && info.filePath.contains('.')) {
            QString ext = QFileInfo(info.filePath).suffix().toLower();
            if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif") {
                QByteArray imgData = zip.fileData(info.filePath);
                if (imgData.isEmpty())
                    continue;
                QString tempPath = s_tempDir.filePath(QFileInfo(info.filePath).fileName());
                QFile f(tempPath);
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(imgData);
                    f.close();
                    outImagePaths.append(tempPath);
                }
            }
        }
    }
    zip.close();
    return allText;
}

static QString readPptx(const QString &filePath, QStringList &outImagePaths)
{
    QString allText;
    QZipReader zip(filePath);
    if (!zip.exists()) {
        qWarning() << "无法打开 pptx 文件：" << filePath;
        return allText;
    }

    QStringList slideFiles;
    for (const QZipReader::FileInfo &info : zip.fileInfoList()) {
        if (info.filePath.startsWith("ppt/slides/slide") && info.filePath.endsWith(".xml"))
            slideFiles.append(info.filePath);
    }
    std::sort(slideFiles.begin(), slideFiles.end(),
              [](const QString &a, const QString &b) {
                  int na = QRegularExpression("slide(\\d+)\\.xml").match(a).captured(1).toInt();
                  int nb = QRegularExpression("slide(\\d+)\\.xml").match(b).captured(1).toInt();
                  return na < nb;
              });

    for (const QString &slidePath : slideFiles) {
        QByteArray data = zip.fileData(slidePath);
        if (data.isEmpty())
            continue;

        QXmlStreamReader xml(data);
        xml.setNamespaceProcessing(false);

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == QLatin1String("a:t") || xml.name() == QLatin1String("t")) {
                    allText += xml.readElementText() + ' ';
                }
            }
        }
        allText += '\n';
    }

    for (const QZipReader::FileInfo &info : zip.fileInfoList()) {
        if (info.filePath.startsWith("ppt/media/") && info.filePath.contains('.')) {
            QString ext = QFileInfo(info.filePath).suffix().toLower();
            if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif") {
                QByteArray imgData = zip.fileData(info.filePath);
                if (imgData.isEmpty())
                    continue;
                QString tempPath = s_tempDir.filePath(QFileInfo(info.filePath).fileName());
                QFile f(tempPath);
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(imgData);
                    f.close();
                    outImagePaths.append(tempPath);
                }
            }
        }
    }
    zip.close();
    return allText;
}

ImportResult FileImporter::importFile(const QString &filePath)
{
    ImportResult result;
    QFileInfo fi(filePath);
    QString suffix = fi.suffix().toLower();

    if (suffix == "txt") {
        result.text = readTxt(filePath);
    } else if (suffix == "md") {
        result.text = readMarkdown(filePath);
    } else if (suffix == "html" || suffix == "htm") {
        result.text = readHtml(filePath);
    } else if (suffix == "pdf") {
        result.text = readPdf(filePath);
    } else if (suffix == "docx") {
        result.text = readDocx(filePath, result.imagePaths);
    } else if (suffix == "pptx") {
        result.text = readPptx(filePath, result.imagePaths);
    } else {
        result.text = readTxt(filePath);
    }

    return result;
}