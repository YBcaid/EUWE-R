#ifndef FILEIMPORTER_H
#define FILEIMPORTER_H

#include <QString>
#include <QStringList>

struct ImportResult
{
    QString text;               ///< 提取的纯文本内容
    QStringList imagePaths;     ///< 提取的图片临时文件路径列表
};

/**
 * @brief 文件导入工具，支持 TXT、MD、HTML、PDF、DOCX、PPTX 格式，
 *        自动提取文本和嵌入的图片（图片保存至临时目录）。
 */
class FileImporter
{
public:
    /// 导入文件，返回文本内容和提取的图片临时路径列表
    static ImportResult importFile(const QString &filePath);
};

#endif // FILEIMPORTER_H