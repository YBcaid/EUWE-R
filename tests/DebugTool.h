#ifndef DEBUGTOOL_H
#define DEBUGTOOL_H

#include <QCoreApplication>
#include <QDir>
#include <QFileInfoList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <iostream>

#define PROJECTROOT QDir::cleanPath(QString(PROJECT_ROOT))

/**
 * @brief 调试工具类，提供项目目录树生成和源代码文件拼接功能。
 */
class DebugTool
{
public:
    DebugTool();

    /// 生成指定路径的目录树（文本格式）
    QString generateDirectoryTree(const QString &path);
    void buildTree(const QDir &dir, const QString &prefix, QStringList &lines);

    /**
     * @brief 遍历目录，将所有源代码文件内容拼接为一个字符串，末尾附加目录树。
     * @param rootPath 要遍历的根目录，为空则使用当前工作目录
     * @return 拼接后的完整文本
     */
    QString combineAllFilesForAI(const QString &rootPath = QString());

private:
    void combineDirectoryRecursive( const QDir &dir,
                                    QString &output,
                                    const QStringList &allowedExtensions,
                                    const QStringList &excludeDirs,
                                    const QString &rootPath,
                                    const QStringList &excludeFileNames);

    void appendFileContent(const QString &fileAbsolutePath,
                           const QString &relativePath,
                           QString &output);
};

#endif // DEBUGTOOL_H