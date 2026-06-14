// TextUtils.h
#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <QString>

/**
 * @brief 文本处理工具函数集合
 *
 * 提供拉丁字母判定、数字字符串检查、单词清洗等辅助函数。
 * 所有函数均为线程安全且无副作用。
 */
namespace TextUtils
{

/**
 * @brief 判断字符是否为拉丁字母（含基本区及常见扩展区）
 *
 * 覆盖范围：
 * - 基本拉丁字母 A-Z, a-z
 * - Latin-1 Supplement 字母部分 (À-Ö, Ø-ö, ø-ÿ)，排除乘号 × (U+00D7)
 * - Latin Extended-A (U+0100-U+017F)
 * - Latin Extended-B (U+0180-U+024F)
 *
 * @param ch 待判断的字符
 * @return true 若为拉丁字母（含变音符号）
 */
bool isLatinLetter(QChar ch);

/**
 * @brief 判断字符是否为基本 ASCII 英文字母 (A-Z, a-z)
 * @param ch 待判断的字符
 * @return true 若为 ASCII 字母
 */
bool isLetter(QChar ch);

/**
 * @brief 判断字符串是否完全由十进制数字组成
 *
 * 十进制数字包括所有 Unicode 中表示 0-9 的字符（如 ASCII 数字、全角数字等）。
 *
 * @param s 待判断的字符串
 * @return true 若非空且所有字符均为十进制数字字符
 */
bool isAllDigits(const QString &s);

/**
 * @brief 移除单词首尾多余的连字符 '-'
 *
 * 将单词首尾的所有连字符连续片段移除，若清理后无有效字符则返回空串。
 *
 * @param word 输入单词
 * @return 去除首尾连字符后的单词
 */
QString cleanWord(const QString &word);

/**
 * @brief 保留拉丁字母与连字符，并转为小写
 *
 * 遍历输入字符串，仅保留拉丁字母（含扩展区）和连字符 '-'，
 * 所有保留的字母统一转换为小写。
 *
 * @param text 原始文本
 * @return 过滤并小写后的字符串
 */
QString keepOnlyEnglishLetters(const QString &text);

} // namespace TextUtils

#endif // TEXTUTILS_H