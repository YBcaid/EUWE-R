#include "DebugTool.h"
#include <QFile>
#include <QTextStream>
#include <QSet>

DebugTool::DebugTool()
{
}

void DebugTool::buildTree(const QDir &dir, const QString &prefix, QStringList &lines)
{
    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;
    QDir::SortFlags sort = QDir::Name | QDir::DirsFirst;
    QFileInfoList entries = dir.entryInfoList(filters, sort);

    QFileInfoList filtered;
    for (const QFileInfo &info : entries)
    {
        if (info.isDir())
        {
            QString name = info.fileName();
            if (name == "build" || name == ".qtcreator")
                continue;
        }
        filtered.append(info);
    }

    int count = filtered.size();
    for (int i = 0; i < count; ++i)
    {
        const QFileInfo &info = filtered.at(i);
        bool isLast = (i == count - 1);
        QString connector = isLast ? "└── " : "├── ";
        lines.append(prefix + connector + info.fileName());

        if (info.isDir())
        {
            QString nextPrefix = prefix + (isLast ? "    " : "│   ");
            QDir subDir(info.absoluteFilePath());
            buildTree(subDir, nextPrefix, lines);
        }
    }
}

QString DebugTool::generateDirectoryTree(const QString &path)
{
    QDir rootDir(path);
    if (!rootDir.exists())
    {
        return QString("Directory does not exist: %1").arg(path);
    }

    QStringList lines;
    lines.append(".");
    buildTree(rootDir, "", lines);
    return lines.join("\n");
}

void DebugTool::appendFileContent(const QString &fileAbsolutePath,
                                  const QString &relativePath,
                                  QString &output)
{
    QFile file(fileAbsolutePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        output.append(QString("// [ERROR] Cannot open file: %1\n\n").arg(relativePath));
        return;
    }

    QTextStream stream(&file);
    QString content = stream.readAll();

    output.append(QString("\n// ============================================================\n"));
    output.append(QString("// File: %1\n").arg(relativePath));
    output.append(QString("// ============================================================\n"));
    output.append(content);
    if (!content.endsWith('\n'))
        output.append('\n');
    output.append(QString("// ============================================================\n"));
    output.append(QString("// End of file: %1\n").arg(relativePath));
    output.append(QString("// ============================================================\n\n"));
}

void DebugTool::combineDirectoryRecursive(const QDir &dir,
                                          QString &output,
                                          const QStringList &allowedExtensions,
                                          const QStringList &excludeDirs,
                                          const QString &rootPath,
                                          const QStringList &excludeFileNames)
{
    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks;
    QDir::SortFlags sort = QDir::Name | QDir::DirsFirst;
    QFileInfoList entries = dir.entryInfoList(filters, sort);

    QSet<QString> allowedSet(allowedExtensions.begin(), allowedExtensions.end());
    QSet<QString> excludeSet(excludeFileNames.begin(), excludeFileNames.end());

    for (const QFileInfo &info : entries)
    {
        if (info.isFile())
        {
            QString fileName = info.fileName();
            // 跳过需要排除的文件名（如 combined_for_ai.txt）
            if (excludeSet.contains(fileName))
                continue;

            QString suffix = info.suffix().toLower();
            if (allowedSet.isEmpty() || allowedSet.contains(suffix))
            {
                QString relativePath = QDir(rootPath).relativeFilePath(info.absoluteFilePath());
                appendFileContent(info.absoluteFilePath(), relativePath, output);
            }
        }
    }

    for (const QFileInfo &info : entries)
    {
        if (info.isDir())
        {
            QString dirName = info.fileName();
            if (excludeDirs.contains(dirName))
                continue;

            QDir subDir(info.absoluteFilePath());
            combineDirectoryRecursive(subDir, output, allowedExtensions, excludeDirs, rootPath, excludeFileNames);
        }
    }
}

QString DebugTool::combineAllFilesForAI(const QString &rootPath)
{
    QString targetPath = rootPath.isEmpty() ? QDir::currentPath() : rootPath;
    QDir rootDir(targetPath);
    if (!rootDir.exists())
    {
        return QString("Error: Directory does not exist: %1").arg(targetPath);
    }

    QStringList allowedExtensions = {
        "cpp", "h", "hpp", "c", "cc", "cxx",
        "pro", "pri",
        "qml", "qrc", "ui",
        "txt", "md", "cmake",
        "in", "xml", "json", "yaml", "yml"
    };

    QStringList excludeDirs = {
        "build", ".qtcreator", ".git", ".vs",
        "Debug", "Release", "x64", "Win32",
        "obj", ".idea", "__pycache__", "CMakeFiles",
        "out", "cmake-build-debug", "cmake-build-release"
    };

    QStringList excludeFileNames = {
        "combined_for_ai.txt", "combined_qml.txt"
    };

    QString output;
    output.append("// ============================================================\n");
    output.append("// Combined project files for AI analysis\n");
    output.append(QString("// Root directory: %1\n").arg(QDir::toNativeSeparators(targetPath)));
    output.append("// ============================================================\n\n");

    combineDirectoryRecursive(rootDir, output, allowedExtensions, excludeDirs, targetPath, excludeFileNames);

    output.append("\n\n// ============================================================\n");
    output.append("// Directory Tree Structure\n");
    output.append("// ============================================================\n");
    output.append(generateDirectoryTree(targetPath));
    output.append("\n");

    return output;
}