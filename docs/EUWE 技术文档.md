

> 本文档适用于 Obsidian，请放在 `docs/` 目录下。所有快速索引链接均为文档内部锚点，可直接跳转。  
> 本文档为每个类提供详尽的 API 说明、使用示例和业务流程，方便开发者查阅和维护。

---

## 📑 快速索引

### C++ 类/结构体/枚举

| 类/组件 | 跳转 |
|--------|------|
| [[#AppConfig\|AppConfig]] | 配置管理 |
| [[#WordEntry (struct)\|WordEntry]] | 单词数据结构 |
| [[#TextUtils (namespace)\|TextUtils]] | 文本工具函数 |
| [[#Logger\|Logger]] | 日志管理器 |
| [[#LogMessage\|LogMessage]] | 日志消息构建器 |
| [[#LoggerTarget\|LoggerTarget]] | 日志输出目标基类 |
| [[#ConsoleTarget\|ConsoleTarget]] | 控制台输出目标 |
| [[#FileTarget\|FileTarget]] | 文件输出目标（支持滚动） |
| [[#LogLevel / LogCategory\|LogLevel / LogCategory]] | 日志级别与分类枚举 |
| [[#DatabaseTableManager\|DatabaseTableManager]] | 通用数据库表管理器 |
| [[#WordDatabaseManager\|WordDatabaseManager]] | 熟词库管理器 |
| [[#LookupService\|LookupService]] | 词典查询服务 |
| [[#WordExtractor\|WordExtractor]] | 英文单词提取器 |
| [[#ExportManager\|ExportManager]] | 导出生词表（CSV/PDF/DOC/JSON） |
| [[#FileImporter\|FileImporter]] | 文件导入工具 |
| [[#ImportResult (struct)\|ImportResult]] | 文件导入结果结构体 |
| [[#OcrManager\|OcrManager]] | 多线程 OCR 管理器 |
| [[#OcrTask\|OcrTask]] | OCR 任务（QRunnable） |
| [[#MainController\|MainController]] | 主控制器（QML 与 C++ 桥接） |
| [[#DatabaseManagementWindow\|DatabaseManagementWindow]] | 数据库管理窗口 |
| [[#DebugTool\|DebugTool]] | 调试工具（目录树/代码拼接） |
| [[#DebugWindow\|DebugWindow]] | 全功能调试窗口 |
| [[#Widget (app)\|Widget (app)]] | 应用入口占位窗口 |
| **占位符类** | |
| [[#SpacedRepetition\|SpacedRepetition]] | 间隔重复（未完成） |
| [[#WordStatistician\|WordStatistician]] | 单词统计（未完成） |
| [[#WordCorrection\|WordCorrection]] | 单词纠正（未完成） |
| [[#LocalServer\|LocalServer]] | 本地服务器（未完成） |
| [[#UpdateChecker\|UpdateChecker]] | 更新检查（未完成） |
| [[#ResultDisplayWidget\|ResultDisplayWidget]] | 结果展示组件（未完成） |

### QML 组件

| 组件 | 跳转 |
|-----|------|
| [[#CapsuleButton\|CapsuleButton]] | 胶囊按钮 |
| [[#ExtractPage\|ExtractPage]] | 提取页面 |
| [[#MessageDialog\|MessageDialog]] | 模态对话框 |
| [[#PanelGroup\|PanelGroup]] | 面板分组标题 |
| [[#ResultPage\|ResultPage]] | 结果页面（单词分类列表） |
| [[#SelectableWordList\|SelectableWordList]] | 可选单词列表（支持多选/拖拽） |
| [[#SideBarItem\|SideBarItem]] | 侧边栏图标项 |
| [[#Theme (singleton)\|Theme]] | 主题单例（亮色/暗色） |
| [[#ThemeToggleBtn\|ThemeToggleBtn]] | 主题切换按钮 |
| [[#WindowControlBtn\|WindowControlBtn]] | 窗口控制按钮（最小化/最大化/关闭） |
| [[#WordManagePage\|WordManagePage]] | 熟词库管理页面 |
| [[#WordTable\|WordTable]] | 单词表格（简单列表） |
| [[#ResultDisplayWidget (QML)\|ResultDisplayWidget (QML)]] | 结果展示窗口（占位） |

---

## 第一部分：核心业务流程（文本化详细调用链）

### 1.1 单词提取与分析

**触发**：用户在“提取页”点击“提取单词” → QML 调用 `MainController::extractWords(text)`

**详细调用链**：

```
1. MainController::extractWords(text) 被调用
   ├─ 检查 text.trimmed().isEmpty() → 若空，设置状态消息并返回
   ├─ 检查 m_isProcessing → 若真，提示“正在处理”并返回
   ├─ 设置 m_isProcessing = true，发射 processingChanged()
   ├─ 设置状态消息 = "正在提取单词并分析..."，发射 statusMessageChanged()
   ├─ 启动 QtConcurrent::run 后台线程
   │   ├─ 创建 WordExtractor 对象：WordExtractor extractor(text, m_dbManager)
   │   │   └─ 构造函数内部调用 extract(text)
   │   ├─ WordExtractor::extract(text) 执行：
   │   │   ├─ 清空内部 m_words 集合
   │   │   ├─ 将文本末尾追加一个空格，简化边界处理
   │   │   ├─ 遍历每个字符：
   │   │   │   ├─ 若为拉丁字母 (TextUtils::isLatinLetter)：
   │   │   │   │   ├─ 若处于 afterNewline 状态且 current 非空 → 处理跨行合并
   │   │   │   │   │   ├─ 前瞻收集后续连字符和字母序列到 following
   │   │   │   │   │   ├─ 调用 tryMergeAcrossNewline(current, following, hasTrailingHyphen)
   │   │   │   │   │   ├─ 若合并成功则 current = merged；否则保存 cleanedCurrent，current = following
   │   │   │   │   ├─ 否则常规追加字符（小写），清除 hasTrailingHyphen 标志
   │   │   │   ├─ 若为连字符 '-':
   │   │   │   │   ├─ 处理各种边界情况，更新 hasTrailingHyphen 和 current
   │   │   │   ├─ 若为换行符 '\n':
   │   │   │   │   ├─ 若 current 非空，设置 afterNewline = true
   │   │   │   ├─ 其他字符（分隔符）:
   │   │   │   │   ├─ 若 current 非空，清洗后插入 m_words，清空 current
   │   │   ├─ 循环结束后，若 current 非空，清洗后插入 m_words
   │   ├─ 调用 extractor.getWords() → 返回 std::unordered_set<QString> wordsSet（移动语义，内部集合被清空）
   │   ├─ 通过 QMetaObject::invokeMethod 将 wordsSet 传回主线程，调用 classifyWords(wordsSet)
   ├─ 主线程执行 classifyWords(wordsSet)
   │   ├─ 若 wordsSet 为空 → 清空三个列表，发射信号，返回
   │   ├─ 获取已知熟词集合：
   │   │   ├─ QList<WordEntry> allKnown = m_dbManager->getAllWords()
   │   │   ├─ QSet<QString> knownSet; 遍历 allKnown 插入 knownSet
   │   ├─ 清空本地临时列表 localKnown, localNew, localUnknown
   │   ├─ 遍历 wordsSet 中的每个单词 w：
   │   │   ├─ 若 knownSet.contains(w) → localKnown.append(w)
   │   │   ├─ 否则调用 m_lookupService->lookupWord(w)
   │   │   │   ├─ 查询 stardict 表：SELECT word, phonetic, translation FROM stardict WHERE word = ? COLLATE NOCASE
   │   │   │   ├─ 若找到，构造 WordEntry 加入 localNew
   │   │   │   └─ 否则 localUnknown.append(w)
   │   ├─ 将 localKnown, localNew, localUnknown 赋值给成员变量 m_knownWords, m_newWords, m_unknownWords
   │   ├─ 发射 wordStatsChanged() 通知 QML 刷新计数
   │   ├─ 发射 extractionFinished()
   │   └─ 设置状态消息为“分析完成 - 熟词: X, 生词: Y, 未识别: Z”
   └─ 设置 m_isProcessing = false，发射 processingChanged()
```

**关键算法：跨行连字符合并** (`WordExtractor::tryMergeAcrossNewline`)：
1. 将 `part2` 开头的连字符去除到 `part2Clean`，记录是否有前导连字符 `part2StartsWithHyphen`。
2. 若 `part2Clean` 为空，返回空字符串。
3. 构造候选 `candidateWithHyphen = part1 + "-" + part2Clean` 和 `candidateWithoutHyphen = part1 + part2Clean`。
4. 若 `candidateWithoutHyphen` 在熟词库中存在（`isWordInDictionary`），返回它。
5. 若 `candidateWithHyphen` 在熟词库中存在，返回它。
6. 如果 `hasHyphenBeforeNewline` 或 `part2StartsWithHyphen` 为真，返回 `candidateWithHyphen`。
7. 如果 `part1` 全是数字（`TextUtils::isAllDigits`），返回 `candidateWithHyphen`。
8. 否则返回空字符串（不合并）。

### 1.2 文件导入与 OCR

**触发**：点击“文件/图片导入” → `MainController::importFile(filePath)`

**详细调用链**：

```
1. MainController::importFile(filePath) 被调用
   ├─ 检查 filePath 是否为空 → 返回
   ├─ 检查 m_isProcessing → 若真，返回
   ├─ 设置 m_isProcessing = true，发射 processingChanged()
   ├─ 设置状态消息 = "正在导入文件..."
   ├─ 启动 QtConcurrent::run 后台线程
   │   ├─ ImportResult result = FileImporter::importFile(filePath)
   │   │   ├─ 根据文件后缀选择解析器：
   │   │   │   ├─ .txt, .md → readTxt(): QFile 打开，QTextStream 自动检测 Unicode，读取全部内容
   │   │   │   ├─ .html, .htm → readHtml(): 读取 HTML 内容，用 QTextDocument 解析，返回纯文本
   │   │   │   ├─ .pdf → readPdf(): 使用 QPdfDocument 逐页加载，调用 getAllText() 获取每页文本，拼接
   │   │   │   ├─ .docx → readDocx():
   │   │   │   │   ├─ 使用 QZipReader 解压
   │   │   │   │   ├─ 读取 "word/document.xml"，使用 QXmlStreamReader 提取 <w:t> 元素文本
   │   │   │   │   ├─ 遍历文件列表，将 "word/media/" 下的图片（png/jpg/jpeg/bmp/gif）提取到临时目录，路径存入 imagePaths
   │   │   │   ├─ .pptx → readPptx():
   │   │   │   │   ├─ 解压 ZIP，收集 "ppt/slides/slide*.xml"，按数字排序
   │   │   │   │   ├─ 对每个幻灯片 XML，提取 <a:t> 元素文本
   │   │   │   │   ├─ 提取 "ppt/media/" 下的图片到临时目录
   │   │   │   └─ 其他后缀 → 尝试 readTxt()
   │   │   └─ 返回 ImportResult{ text, imagePaths }
   │   ├─ 发射 fileImportFinished(result.text, result.imagePaths) → QML 将文本追加到主输入框
   │   ├─ 如果 result.imagePaths 非空：
   │   │   ├─ 调用 m_ocrManager->startRecognition(result.imagePaths, "\n---\n")
   │   │   │   ├─ 在 QtConcurrent 线程中执行 recognizeImages()
   │   │   │   │   ├─ 设置 m_pendingCount = imagePaths.size()
   │   │   │   │   ├─ 为每张图片创建 OcrTask，提交到 m_ocrThreadPool
   │   │   │   │   ├─ 主线程等待 m_waitCondition，直到 pendingCount == 0
   │   │   │   │   ├─ 收集成功识别的文本，用 separator 连接
   │   │   │   │   └─ 返回合并后的文本
   │   │   │   └─ 完成时发射 finished(mergedText)
   │   │   └─ 在回调中：发射 ocrFinished(mergedText) → QML 将 OCR 结果追加到输入框
   │   ├─ 如果 result.text 为空且 result.imagePaths 为空 → 发射 operationFailed("无法读取文件内容")
   │   └─ 设置状态消息为“文件导入成功...”，并设置 m_isProcessing = false（在合适的时机）
   └─ QML 更新文本编辑区
```

### 1.3 熟词库管理

**录入生词**（结果页选中生词 → “录入熟词库”）：

```
MainController::addToKnown(selectedIndexes)
   ├─ 检查 m_dbManager 是否有效，无效则提示并返回
   ├─ 遍历 selectedIndexes（QVariantList，每个元素是 int 索引）：
   │   └─ 若索引在 m_newWords 范围内，将对应 WordEntry 添加到 toAdd 列表
   ├─ 若 toAdd 为空，提示并返回
   ├─ 调用 m_dbManager->addWords(toAdd)
   │   ├─ WordDatabaseManager::addWords(words):
   │   │   ├─ 调用 beginTransaction()
   │   │   ├─ 准备 SQL：INSERT OR IGNORE INTO Words (word, translation, phonetic) VALUES (?, ?, ?)
   │   │   ├─ 对每个单词：
   │   │   │   ├─ norm = normalizeWord(w.word)
   │   │   │   ├─ 若 norm 为空，跳过
   │   │   │   ├─ 执行 query，若成功且 numRowsAffected() > 0 则 inserted++
   │   │   ├─ commitTransaction()
   │   │   └─ 返回 inserted
   │   └─ 返回实际插入数量
   ├─ 若 inserted > 0：
   │   ├─ 将 toAdd 中的单词从 m_newWords 中移除（根据 word 匹配）
   │   ├─ 将单词添加到 m_knownWords（字符串形式）
   │   ├─ 发射 wordStatsChanged()
   │   ├─ 发射 extractionFinished() 通知 QML 刷新列表
   │   └─ 设置状态消息 = "成功录入 X 个单词到熟词库"
   └─ 否则设置状态消息 = "录入失败，可能单词已存在或数据库错误"
```

**移出熟词库**：类似，调用 `removeWords`，将单词从 `m_knownWords` 移除，添加到 `m_unknownWords`。

**手动添加/删除单词**（管理页）：
- `WordManagePage.qml` 调用 `MainController::addToKnown` 或 `removeFromKnown`，或直接调用 `WordDatabaseManager::addWord`/`removeWord`（通过信号 `addWord`/`deleteWord` 连接到槽函数）。

### 1.4 导出功能

**导出 CSV**：
```
MainController::exportCsv()
   ├─ 若 m_newWords 为空，设置状态消息并返回
   ├─ 调用 QFileDialog::getSaveFileName 选择文件路径（默认 "生词表.csv"）
   ├─ 调用 ExportManager::exportToCsv(m_newWords, filePath)
   │   ├─ 打开文件，写入 UTF-8 BOM
   │   ├─ 写入表头 "单词,音标,释义"
   │   ├─ 遍历 words，对每个字段进行 CSV 转义（逗号、引号、换行符）
   │   ├─ 写入文件
   │   └─ 返回 true/false
   └─ 若成功，设置状态消息 "已导出 CSV 到 ..."，否则提示失败
```

其他导出格式（PDF、DOC、JSON）类似，内部调用相应方法：
- `exportToPdf`：调用 `generateHtmlContent` 生成 HTML → `QTextDocument` → `QPrinter` 打印为 PDF。
- `exportToDoc`：生成带 Word 兼容标记的 HTML，保存为 `.doc`。
- `exportToJson`：序列化为 `{"words":[{"word":"...","translation":"...","phonetic":"..."}]}`。

### 1.5 配置管理（AppConfig）

**详细使用流程**（已在之前文档中给出，此处再强调关键点）：

1. **初始化**：`AppConfig::initialize("path/config.json")` 可选，默认路径为应用数据目录。
2. **获取单例**：`AppConfig &cfg = AppConfig::instance();` 首次调用时自动加载文件（若不存在则创建空对象）。
3. **读取**：`cfg.value<int>("section/key", 123)`；若 `autoWriteDefault` 为 true 且键不存在，自动写入默认值并发出 `configChanged`。
4. **写入**：`cfg.setValue("section/key", val, autoSave)`；若新旧值相同则跳过；若 `autoSave` 或全局自动保存启用，立即写盘。
5. **保存**：`cfg.save()` 手动保存；析构时若已初始化也会自动保存。
6. **监听**：连接 `configChanged` 信号。

### 1.6 日志系统（Logger）

**初始化步骤**（参考 `tests/main.cpp` 中的 `initAllLogging()`）：
1. 为每个日志分类创建 `FileTarget`，指向 `data/logs/` 下的独立日志文件（如 `database.log`）。
2. 创建 `ConsoleTarget`，并调用 `addTargetForAllCategories` 使其输出到所有分类。
3. 设置全局日志级别：`Logger::instance().setGlobalLevel(LogLevel::DEBUG);`

**使用宏**：
```cpp
LOG_INFO_CAT(LogCategory::Database) << "User logged in";
LOG_ERROR() << "Failed to open file";
```

**输出格式**：`[yyyy-MM-dd HH:mm:ss.zzz] [LEVEL] [CAT] [threadID] file:line - function: message`

---

## 第二部分：类详细使用参考

### AppConfig

**头文件**：`common/AppConfig.h`  
**基类**：`QObject`  
**线程安全**：是（所有公有方法内部使用 `QMutex` 保护）

#### 静态方法

| 方法 | 说明 |
|------|------|
| `static void initialize(const QString &filePath = QString())` | 设置配置文件路径。默认路径为 `QStandardPaths::writableLocation(AppDataLocation) + "/config.json"`。若实例已加载过配置，后续调用传入非空路径将被忽略并输出警告。 |
| `static AppConfig &instance()` | 获取单例实例。首次调用时触发惰性加载（读取文件，失败则创建空对象）。 |

#### 实例方法

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `bool load()` | 无 | `bool` | 重新加载配置文件。若未初始化则直接加载；若已初始化则读取文件并将顶层键覆盖到内存对象（嵌套对象整体替换）。成功后发射 `configReloaded()`。 |
| `bool save()` | 无 | `bool` | 将当前配置保存到磁盘（使用 `QSaveFile` 原子写入）。 |
| `template<typename T> T value(const QString &key, const T &defaultValue = T())` | `key`: 层级键，如 `"server/port"`; `defaultValue`: 默认值 | `T` | 读取配置值，自动进行类型转换。若键不存在且 `autoWriteDefault` 为 true，则将默认值写入配置并发射 `configChanged` 信号。若转换失败则返回 `defaultValue`。 |
| `QVariant value(const QString &key, const QVariant &defaultValue = QVariant())` | 同上 | `QVariant` | QVariant 版本。 |
| `void setValue(const QString &key, const QVariant &val, bool autoSave = false)` | `key`: 层级键; `val`: 新值; `autoSave`: 是否立即保存（覆盖全局设置） | 无 | 设置配置值。若新旧值相同则不做任何操作。若 `autoSave` 为 true 或全局自动保存启用，则调用 `writeToDisk()`。成功后发射 `configChanged`。 |
| `bool remove(const QString &key)` | `key`: 要删除的键 | `bool` | 删除键及其子键。成功则发射 `configChanged(key, QVariant())`。 |
| `bool contains(const QString &key) const` | `key`: 键 | `bool` | 检查键是否存在（路径指向有效值）。 |
| `QStringList childGroups() const` | 无 | `QStringList` | 返回所有顶级组名（值为 JSON 对象的键）。 |
| `QStringList childKeys(const QString &group = QString()) const` | `group`: 组路径（如 `"network"`），空字符串表示根 | `QStringList` | 返回指定组下所有叶子键的完整路径（如 `"network/host"`, `"network/port"`）。 |
| `void setAutoSaveEnabled(bool enabled)` | `enabled`: 是否启用 | 无 | 设置全局自动保存标志。启用后每次 `setValue` 都会自动写盘。 |
| `bool isAutoSaveEnabled() const` | 无 | `bool` | 返回全局自动保存是否启用。 |
| `void setAutoWriteDefault(bool enabled)` | `enabled`: 是否启用 | 无 | 设置自动写入默认值标志。启用后，读取不存在的键时会将默认值写入配置。 |
| `bool isAutoWriteDefault() const` | 无 | `bool` | 返回自动写入默认值是否启用。 |
| `QString filePath() const` | 无 | `QString` | 返回当前配置文件完整路径。 |

#### 信号

| 信号 | 参数 | 说明 |
|------|------|------|
| `void configChanged(const QString &key, const QVariant &newValue)` | `key`: 变更的键; `newValue`: 新值（删除时为无效 QVariant） | 当配置值被修改或删除时发出。 |
| `void configReloaded()` | 无 | 成功重新加载配置后发出。 |

#### 使用示例

```cpp
// 在 main() 中初始化
AppConfig::initialize("myapp.json");

// 获取单例
AppConfig &cfg = AppConfig::instance();

// 启用自动写入默认值
cfg.setAutoWriteDefault(true);

// 读取，若不存在则自动写入 8080
int port = cfg.value<int>("network/port", 8080);

// 修改并立即保存
cfg.setValue("network/port", 9090, true);

// 监听变化
QObject::connect(&cfg, &AppConfig::configChanged,
    [](const QString &key, const QVariant &val) {
        qDebug() << "Changed:" << key << "=" << val;
    });

// 手动保存
cfg.save();
```

---

### WordEntry (struct)

**头文件**：`common/WordEntry.h`

#### 成员变量

| 名称 | 类型 | 说明 |
|------|------|------|
| `word` | `QString` | 单词原文（未规范化） |
| `translation` | `QString` | 中文翻译 |
| `phonetic` | `QString` | 音标（如 `/ˈæp.əl/`） |

#### 构造函数

```cpp
explicit WordEntry(const QString &word = QString(),
                   const QString &translation = QString(),
                   const QString &phonetic = QString())
```

创建单词条目，所有字段可选。

#### 使用示例

```cpp
WordEntry entry("apple", "苹果", "/ˈæp.əl/");
qDebug() << entry.word << entry.translation;
```

---

### TextUtils (namespace)

**头文件**：`common/TextUtils.h`

#### 函数

| 函数 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `bool isLatinLetter(QChar ch)` | `ch`: 字符 | `bool` | 判断是否为拉丁字母（包含基本拉丁、Latin-1 Supplement、Latin Extended-A/B）。 |
| `bool isLetter(QChar ch)` | `ch`: 字符 | `bool` | 判断是否为基本 ASCII 字母（A-Z 或 a-z）。 |
| `bool isAllDigits(const QString &s)` | `s`: 字符串 | `bool` | 判断字符串是否全部由十进制数字字符组成（包括 Unicode 数字）。 |
| `QString cleanWord(const QString &word)` | `word`: 单词 | `QString` | 移除单词首尾的连字符 `'-'`，若结果为空返回空字符串。 |
| `QString keepOnlyEnglishLetters(const QString &text)` | `text`: 输入文本 | `QString` | 保留所有拉丁字母和连字符，并将字母转换为小写。 |

#### 使用示例

```cpp
QString raw = "Hello-World! 123";
QString cleaned = TextUtils::keepOnlyEnglishLetters(raw);
// cleaned == "hello-world"

bool ok = TextUtils::isAllDigits("12345"); // true
QString word = TextUtils::cleanWord("--hello-"); // "hello"
```

---

### Logger 类族

#### LogLevel

**头文件**：`common/Logger.h`  
**枚举定义**：
```cpp
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};
```

#### LogCategory

**枚举定义**：
```cpp
enum class LogCategory {
    General,   // 通用/默认
    Database,  // 数据库操作
    Extract,   // 单词提取
    Ocr,       // OCR 识别
    Import,    // 文件导入
    Export,    // 导出生词表
    UI,        // 界面交互
    Config,    // 配置读写
    Network,   // 网络/本地服务器
    Update     // 更新检查
};
```

#### Logger

**头文件**：`common/Logger.h`  
**设计**：单例模式，线程安全，管理多个 `LoggerTarget`，支持按分类路由。

**静态方法**：
- `static Logger &instance()` — 获取单例。

**配置方法**：
- `void addTarget(const QSharedPointer<LoggerTarget> &target, LogCategory category = LogCategory::General)`  
  为指定分类添加一个输出目标。同一个分类可以有多个目标。
- `void addTargetForAllCategories(const QSharedPointer<LoggerTarget> &target)`  
  将目标添加到所有已存在的分类中（注意：未来新增的分类不会自动添加）。
- `void clearTargets()` — 移除所有目标。
- `void setGlobalLevel(LogLevel level)` — 设置全局最低日志级别，低于该级别的日志不会被记录（在宏中判断）。
- `LogLevel globalLevel() const` — 获取当前全局级别。

**内部写入方法**（通常通过宏调用）：
- `void write(LogLevel level, LogCategory category, const QString &message, const char *file, int line, const char *func)`  
  格式化消息并写入所有匹配的目标（同时写入 General 分类的目标）。

**日志宏**（推荐使用）：
```cpp
#define LOG_DEBUG_CAT(cat)  if (Logger::instance().globalLevel() > LogLevel::DEBUG) {} else LogMessage(LogLevel::DEBUG, cat, __FILE__, __LINE__, Q_FUNC_INFO).stream()
#define LOG_INFO_CAT(cat)   ...
#define LOG_WARNING_CAT(cat)...
#define LOG_ERROR_CAT(cat) ...
#define LOG_FATAL_CAT(cat) ...
// 默认 General 分类：
#define LOG_DEBUG()   LOG_DEBUG_CAT(LogCategory::General)
#define LOG_INFO()    LOG_INFO_CAT(LogCategory::General)
// ...
```

#### LogMessage

**职责**：RAII 辅助类，构造时接收日志级别、分类、文件、行号、函数名，在析构时调用 `Logger::write()` 提交缓冲区内容。不应直接实例化，应通过宏使用。

**公开方法**：
- `QDebug &stream()` — 返回 QDebug 流，用于流式输出。

#### LoggerTarget (抽象基类)

**纯虚方法**：
- `virtual void write(const QString &formattedMessage) = 0`

#### ConsoleTarget

**继承**：`LoggerTarget`  
**实现**：`void write(const QString &formattedMessage) override` — 输出到 `qDebug().noquote()`。

#### FileTarget

**继承**：`LoggerTarget`  
**职责**：将日志写入文件，支持大小滚动。

**构造函数**：
```cpp
explicit FileTarget(const QString &filePath,
                    qint64 maxSizeBytes = 10 * 1024 * 1024,
                    int backupCount = 5)
```
- `filePath`: 日志文件路径。
- `maxSizeBytes`: 单个文件最大字节数，超过后触发滚动。
- `backupCount`: 保留的备份文件数量（例如 `app.log.1`, `app.log.2`, ...）。

**方法**：
- `void write(const QString &formattedMessage) override` — 写入文件，若超过大小则调用 `rollFile()`。

**内部机制**：
- 滚动时：`app.log` → `app.log.1` → `app.log.2` ... 删除最老的备份，然后重新打开文件。

#### 使用示例

```cpp
// 初始化
auto fileTarget = QSharedPointer<FileTarget>::create("logs/database.log", 5*1024*1024, 3);
Logger::instance().addTarget(fileTarget, LogCategory::Database);
auto console = QSharedPointer<ConsoleTarget>::create();
Logger::instance().addTargetForAllCategories(console);
Logger::instance().setGlobalLevel(LogLevel::INFO);

// 日志记录
LOG_INFO_CAT(LogCategory::Database) << "Query executed in " << elapsed << " ms";
LOG_WARNING() << "Low memory";
```

---

### DatabaseTableManager

**头文件**：`core/database/DatabaseTableManager.h`  
**基类**：`QObject`  
**线程安全**：每个实例应在创建它的线程中使用（Qt 对象线程亲和性）。

#### 构造函数

```cpp
explicit DatabaseTableManager(const QString &dbPath,
                              const QString &tableName,
                              const QString &connectionName = QString(),
                              QObject *parent = nullptr)
```
- `dbPath`: SQLite 数据库文件路径。
- `tableName`: 目标表名（会进行合法性校验：正则 `^[A-Za-z_][A-Za-z0-9_]*$`）。
- `connectionName`: Qt 数据库连接名，若为空则自动生成唯一名称（基于表名、对象地址、线程 ID），以避免多线程冲突。
- `parent`: QObject 父对象。

**行为**：
- 自动打开数据库（如果连接名已存在则复用，否则新建）。
- 设置 `PRAGMA foreign_keys = ON` 和 `PRAGMA journal_mode = WAL`。
- 若表名无效，记录错误并返回。

#### 公开方法

**状态查询**：
- `bool isOpen() const` — 返回数据库是否已打开。
- `QString lastError() const` — 返回最后一次错误信息。

**表结构管理**：
- `bool createTableIfNotExists(const QString &ddl)` — 执行 DDL 语句（通常是 `CREATE TABLE IF NOT EXISTS ...`）。

**事务**：
- `bool beginTransaction()`
- `bool commitTransaction()`
- `bool rollbackTransaction()`

**插入**：
- `qint64 insert(const QVariantMap &data)` — 插入一行，返回新行的 rowid（失败返回 -1）。  
  `data` 的键为列名，必须通过 `isValidIdentifier` 校验。

**更新**：
- `int update(const QVariantMap &data, const QString &whereClause, const QVariantList &whereArgs = QVariantList())`  
  更新满足条件的行，返回受影响行数（失败返回 -1）。  
  `whereClause` 中应使用 `?` 占位符，参数通过 `whereArgs` 传递。

**删除**：
- `int remove(const QString &whereClause, const QVariantList &whereArgs = QVariantList())` — 删除满足条件的行，返回删除行数（失败返回 -1）。

**查询**：
- `QVariantMap selectOne(const QStringList &columns = QStringList(), const QString &whereClause = QString(), const QVariantList &whereArgs = QVariantList(), const QString &orderBy = QString())`  
  查询单行，若没有结果返回空 `QVariantMap`。
- `QList<QVariantMap> select(const QStringList &columns = QStringList(), const QString &whereClause = QString(), const QVariantList &whereArgs = QVariantList(), const QString &orderBy = QString(), int limit = 0)`  
  查询多行，`limit <= 0` 表示不限制。
- `int count(const QString &whereClause = QString(), const QVariantList &whereArgs = QVariantList())` — 统计行数，失败返回 -1。

**批量操作**：
- `int insertBatch(const QList<QVariantMap> &dataList)` — 在事务中批量插入，若任一行失败则回滚并返回 -1，否则返回成功插入的行数。

**工具**：
- `static bool isValidIdentifier(const QString &name)` — 检查表名/列名是否合法（字母/数字/下划线，不以数字开头）。

**受保护方法**（供子类使用）：
- `bool executeQuery(QSqlQuery &query, const QString &errorContext)` — 执行查询，出错时设置 `m_lastError`。
- `bool checkDatabaseOpen()` — 检查数据库是否打开，未打开则设置错误。
- `QString validateColumns(const QStringList &columns)` — 验证列名列表并返回逗号分隔的字符串，若全部无效则返回空字符串。

#### 使用示例

```cpp
DatabaseTableManager mgr("test.db", "users");
if (!mgr.isOpen()) {
    qDebug() << mgr.lastError();
    return;
}
mgr.createTableIfNotExists("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");

// 插入
QVariantMap data{{"name", "Alice"}, {"age", 30}};
qint64 id = mgr.insert(data);
qDebug() << "Inserted id:" << id;

// 更新
mgr.update({{"age", 31}}, "name = ?", {"Alice"});

// 查询
auto rows = mgr.select({"name", "age"}, "age > ?", {20}, "age DESC", 10);
for (auto &row : rows) {
    qDebug() << row["name"] << row["age"];
}

// 删除
mgr.remove("age < ?", {18});
```

---

### WordDatabaseManager

**头文件**：`core/database/WordDatabaseManager.h`  
**继承**：`DatabaseTableManager`

#### 构造函数

```cpp
explicit WordDatabaseManager(const QString &dbPath = "euwe_words.db",
                             const QString &tableName = "Words",
                             const QString &connectionName = QString(),
                             QObject *parent = nullptr)
```
- 默认数据库文件名为 `euwe_words.db`，表名为 `Words`。
- 构造时自动创建表（若不存在）：
  ```sql
  CREATE TABLE IF NOT EXISTS Words (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      word TEXT NOT NULL UNIQUE,
      translation TEXT,
      phonetic TEXT
  )
  ```

#### 公开方法

**单词查询**：
- `bool containsWord(const QString &word)`  
  规范化 `word` 后，查询是否存在。内部调用 `count("word = ?", {norm})`。
- `WordEntry getWordInfo(const QString &word)`  
  规范化后查询，返回 `WordEntry`（若不存在，`word` 字段为空）。
- `QList<WordEntry> getAllWords()`  
  返回所有熟词，按 `word` 升序排序。

**插入**：
- `bool addWord(const QString &word, const QString &translation, const QString &phonetic)`  
  使用 `INSERT OR IGNORE`，规范化单词后插入，若已存在则不报错。
- `int addWords(const QList<WordEntry> &words)`  
  批量插入（在事务中执行），返回实际新增的单词数量（忽略已存在的）。若事务中发生错误则回滚并返回 -1。

**删除**：
- `bool removeWord(const QString &word)`  
  删除单个单词（规范化后匹配）。
- `int removeWords(const QStringList &words)`  
  批量删除（在事务中执行），返回实际删除数量。

**工具**：
- `static QString normalizeWord(const QString &text)`  
  调用 `TextUtils::keepOnlyEnglishLetters`，保留拉丁字母和连字符，转小写。

**临时公开**（调试用）：
- `QSqlDatabase database() const` — 返回底层 `QSqlDatabase` 对象。

#### 私有辅助方法

- `static QVariantMap wordToMap(const WordEntry &word)` — 将 `WordEntry` 转为 `QVariantMap`（供基类通用接口使用）。
- `static WordEntry mapToWord(const QVariantMap &map)` — 反向转换。

#### 使用示例

```cpp
WordDatabaseManager wm;

// 添加单词
wm.addWord("hello", "你好", "/həˈloʊ/");
wm.addWord("world", "世界", "/wɝːld/");

// 检查存在
if (wm.containsWord("Hello")) {
    qDebug() << "Hello exists";
}

// 获取全部
QList<WordEntry> all = wm.getAllWords();
for (auto &e : all) {
    qDebug() << e.word << e.translation;
}

// 批量添加
QList<WordEntry> batch = {WordEntry("apple","苹果","/ˈæp.əl/"), WordEntry("pear","梨","/peər/")};
int inserted = wm.addWords(batch);
qDebug() << "Inserted:" << inserted;

// 删除
wm.removeWord("pear");
```

---

### LookupService

**头文件**：`core/dictionary/LookupService.h`  
**基类**：`QObject`

#### 构造函数

```cpp
explicit LookupService(const QSqlDatabase &db, QObject *parent = nullptr)
```
- `db`: 已打开的数据库连接，必须包含表 `stardict`（字段 `word`, `phonetic`, `translation`）。

#### 方法

- `WordEntry lookupWord(const QString &word) const`  
  查询单词，使用 `COLLATE NOCASE` 实现大小写不敏感匹配。  
  返回 `WordEntry`，若未找到则 `word` 字段为空。  
  内部 SQL：
  ```sql
  SELECT word, phonetic, translation FROM stardict WHERE word = ? COLLATE NOCASE
  ```

#### 使用示例

```cpp
WordDatabaseManager wm;  // 它会打开同一个数据库文件
LookupService lookup(wm.database());
WordEntry entry = lookup.lookupWord("apple");
if (!entry.word.isEmpty()) {
    qDebug() << entry.translation;
} else {
    qDebug() << "Word not found";
}
```

---

### WordExtractor

**头文件**：`core/extractor/WordExtractor.h`  
**基类**：`QObject`

#### 构造函数

```cpp
explicit WordExtractor(const QString &text = QString(),
                       WordDatabaseManager *dbManager = nullptr,
                       QObject *parent = nullptr)
```
- `text`: 待提取的文本，若非空则在构造时自动调用 `extract(text)`。
- `dbManager`: 可选的熟词库管理器指针，用于跨行合并时验证单词存在性（若为 `nullptr`，则 `tryMergeAcrossNewline` 中的词典验证始终返回 `false`）。

#### 方法

- `void extract(const QString &text)`  
  清空内部集合，执行提取。算法细节参见业务流程部分。
- `std::unordered_set<QString> getWords()`  
  返回提取出的单词集合（移动语义），调用后内部集合被清空。

#### 私有辅助方法

- `bool isWordInDictionary(const QString &word) const` — 若 `m_dbManager` 有效则调用其 `containsWord`。
- `QString tryMergeAcrossNewline(const QString &part1, const QString &part2, bool hasHyphenBeforeNewline) const`  
  决策是否将换行前后的两部分合并为一个单词，策略见上文。

#### 使用示例

```cpp
WordDatabaseManager wm;
WordExtractor extractor;
extractor.extract("Hello world, this is a well-\nknown example.");
auto words = extractor.getWords();
for (const auto &w : words) {
    qDebug() << w;  // 可能包含 "hello", "world", "well-known" 或 "wellknown"
}
```

---

### ExportManager

**头文件**：`modules/export/ExportManager.h`  
**所有方法均为静态**。

#### 公开静态方法

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `exportToCsv` | `words`: 单词列表; `filePath`: 输出路径; `title`: 标题（未使用） | `bool` | 导出为 CSV（UTF-8 with BOM），列：单词,音标,释义。字段中包含逗号、引号或换行符时自动转义。 |
| `exportToPdf` | `words`, `filePath`, `title` | `bool` | 生成 HTML 表格，通过 `QTextDocument` 和 `QPrinter` 渲染为 PDF。页面大小 A4，页边距 5mm。 |
| `exportToDoc` | `words`, `filePath`, `title` | `bool` | 生成带 Word 兼容标记的 HTML 文件，保存为 `.doc`。 |
| `exportToJson` | `words`, `filePath` | `bool` | 导出为 JSON 格式：`{"words":[{"word":"...","translation":"...","phonetic":"..."}]}`。 |

#### 私有辅助方法

- `static QString generateHtmlContent(const QList<WordEntry> &words, const QString &title)`  
  生成包含单词表格的 HTML 字符串，字体为“Microsoft YaHei”，表格样式简洁。

#### 使用示例

```cpp
QList<WordEntry> words = {WordEntry("apple","苹果","/ˈæp.əl/")};
ExportManager::exportToCsv(words, "output.csv");
ExportManager::exportToPdf(words, "output.pdf", "My Word List");
ExportManager::exportToJson(words, "output.json");
```

---

### FileImporter

**头文件**：`modules/import/FileImporter.h`

#### ImportResult 结构体

```cpp
struct ImportResult {
    QString text;               // 提取的纯文本内容
    QStringList imagePaths;     // 提取的图片临时文件路径列表
};
```

- 图片会被提取到静态 `QTemporaryDir` 中，程序退出时自动删除。

#### FileImporter 静态方法

- `static ImportResult importFile(const QString &filePath)`  
  根据文件后缀调用相应的解析器。

**支持的格式及解析细节**：

| 后缀 | 解析函数 | 说明 |
|------|----------|------|
| `.txt`, `.md` | `readTxt()` | 使用 `QTextStream` 自动检测 Unicode。 |
| `.html`, `.htm` | `readHtml()` | 读取文件，`QTextDocument::setHtml` + `toPlainText`。 |
| `.pdf` | `readPdf()` | 使用 `QPdfDocument` 逐页调用 `getAllText()`。 |
| `.docx` | `readDocx()` | 解压 ZIP，解析 `word/document.xml` 提取 `<w:t>` 文本，同时提取 `word/media/` 下的图片（png/jpg/jpeg/bmp/gif）到临时目录。 |
| `.pptx` | `readPptx()` | 解压 ZIP，解析 `ppt/slides/slide*.xml` 提取 `<a:t>` 文本，提取 `ppt/media/` 图片。 |
| 其他 | `readTxt()` | 尝试作为纯文本打开。 |

#### 使用示例

```cpp
ImportResult result = FileImporter::importFile("document.docx");
if (!result.text.isEmpty()) {
    ui->textEdit->setPlainText(result.text);
}
for (const QString &img : result.imagePaths) {
    // 显示或处理图片
}
```

---

### OcrManager 与 OcrTask

**头文件**：`modules/ocr/OcrManager.h`

#### OcrTask

`QRunnable` 子类，用于在后台线程中执行单张图片的 OCR。

**构造函数**（由 `OcrManager` 内部调用）：
```cpp
OcrTask(const QString &imagePath,
        const QString &ocrExecutablePath,
        const QString &tessdataDirectory,
        const QString &language,
        OcrManager *manager,
        int index)
```
- 任务执行时启动外部进程 `ocrExecutablePath`，参数：`<imagePath> -l <language> -t <tessdataDirectory>`。
- 等待进程完成（最长 30 秒），读取 stdout 作为识别结果。
- 完成后调用 `manager->onTaskFinished(index, success, text)`。

#### OcrManager

**职责**：管理 OCR 任务池，支持同步/异步识别。

**构造函数**：
```cpp
explicit OcrManager(QObject *parent = nullptr)
```
默认值：
- `m_ocrExecutablePath = QCoreApplication::applicationDirPath() + "/EUWEOCRT.exe"`
- `m_tessdataDirectory = QCoreApplication::applicationDirPath() + "/tessdata"`
- `m_language = "eng"`
- `m_maxThreads = QThread::idealThreadCount()`

**配置方法**：
- `void setOcrProgramPath(const QString &path)`
- `void setTessdataPath(const QString &path)`
- `void setLanguage(const QString &lang)`
- `void setMaxThreads(int maxThreads)` — 设置线程池大小。

**识别方法**：
- `QString recognizeImages(const QStringList &imagePaths, const QString &separator = "\n---\n")`  
  同步阻塞，返回合并后的文本。内部实现：
  1. 初始化 `m_pendingCount = imagePaths.size()`，`m_results` 和 `m_successFlags` 调整大小。
  2. 为每张图片创建 `OcrTask` 并提交到线程池。
  3. 等待条件变量 `m_waitCondition`，直到 `m_pendingCount == 0`。
  4. 收集成功的识别结果，用 `separator` 连接后返回。
- `void startRecognition(const QStringList &imagePaths, const QString &separator = "\n---\n")`  
  异步识别，内部在 `QtConcurrent` 线程中调用 `recognizeImages`，完成后发射 `finished` 信号。

**信号**：
- `void finished(const QString &mergedText)`

**私有方法**：
- `void onTaskFinished(int index, bool success, const QString &text)` — 由 `OcrTask` 调用，更新结果并唤醒等待线程。

#### 使用示例

```cpp
OcrManager ocr;
ocr.setLanguage("eng+chi_sim");

// 异步识别
QObject::connect(&ocr, &OcrManager::finished, [](const QString &text) {
    qDebug() << "OCR result:" << text;
});
ocr.startRecognition({"img1.png", "img2.jpg"});

// 同步识别
QString result = ocr.recognizeImages({"test.png"}, "\n");
```

---

### MainController

**头文件**：`ui/main/MainController.h`  
**基类**：`QObject`  
**作用**：作为 QML 与 C++ 后端的唯一桥梁，暴露属性和可调用方法。

#### Q_PROPERTY

| 属性 | 类型 | 读/写 | 通知信号 |
|------|------|-------|----------|
| `statusMessage` | `QString` | 只读 | `statusMessageChanged` |
| `knownWordCount` | `int` | 只读 | `wordStatsChanged` |
| `newWordCount` | `int` | 只读 | `wordStatsChanged` |
| `unknownWordCount` | `int` | 只读 | `wordStatsChanged` |
| `isProcessing` | `bool` | 只读 | `processingChanged` |

#### 构造函数与析构

```cpp
explicit MainController(QObject *parent = nullptr);
~MainController();
```
- 构造函数中初始化后端：创建 `WordDatabaseManager`、`LookupService`、`OcrManager`，连接信号。

#### Q_INVOKABLE 方法

**单词提取**：
- `void extractWords(const QString &text)` — 异步提取单词，结果通过 `extractionFinished` 信号通知，QML 可通过 `knownWordsList` 等获取结果。

**文件导入与 OCR**：
- `void importFile(const QString &filePath)` — 异步导入文件，完成后发射 `fileImportFinished` 和可能 `ocrFinished`。
- `void selectAndOcrImages()` — 弹出文件对话框选择图片，执行 OCR。

**数据库管理**：
- `void openDatabaseManager()` — 打开数据库管理窗口（当前为占位，显示消息框）。

**文本操作**：
- `void clearText()` — 仅改变状态消息（实际清空由 QML 执行）。
- `void copyToClipboard(const QString &text)` — 将文本复制到系统剪贴板。

**帮助/关于**：
- `void showHelp()`, `void showAbout()` — 显示消息框。

**熟词库操作**：
- `void addToKnown(const QVariantList &selectedIndexes)` — 将生词列表中指定索引的单词录入熟词库。
- `void removeFromKnown(const QVariantList &selectedIndexes)` — 将熟词列表中指定索引的单词移出熟词库。

**导出**：
- `void exportCsv()`, `void exportPdf()`, `void exportDoc()` — 导出生词列表为相应格式。

**获取列表（供 QML 模型使用）**：
- `QVariantList knownWordsList() const` — 返回熟词字符串列表。
- `QVariantList newWordsList() const` — 返回生词列表，每个元素为 `QVariantMap`（包含 `word`, `translation`, `phonetic`）。
- `QVariantList unknownWordsList() const` — 返回未识别单词字符串列表。

#### 信号

| 信号 | 参数 | 说明 |
|------|------|------|
| `statusMessageChanged` | 无 | 状态消息变更。 |
| `wordStatsChanged` | 无 | 单词计数变更（`knownWordCount` 等）。 |
| `processingChanged` | 无 | `isProcessing` 变更。 |
| `extractionFinished` | 无 | 单词提取完成，QML 应刷新列表。 |
| `ocrFinished` | `const QString &mergedText` | OCR 识别完成。 |
| `fileImportFinished` | `const QString &text, const QStringList &imagePaths` | 文件导入完成，包含提取的文本和图片路径。 |
| `operationFailed` | `const QString &errorMessage` | 操作失败（如无法读取文件）。 |

#### 私有辅助方法

- `void initBackend()` — 初始化后端对象。
- `void setStatusMessage(const QString &msg)` — 修改 `m_statusMessage` 并发射信号。
- `void setProcessing(bool processing)` — 修改 `m_isProcessing` 并发射信号。
- `void classifyWords(const std::unordered_set<QString> &extractedWords)` — 对提取的单词进行分类（熟词/生词/未识别）。
- `void updateWordStats()` — 发射 `wordStatsChanged`。

#### 使用示例（QML）

```qml
import QtQuick 2.15

ApplicationWindow {
    visible: true
    width: 800; height: 600
    property var appController: MainController {}

    Column {
        TextArea { id: input; width: parent.width; height: 200 }
        Button { text: "提取单词"; onClicked: appController.extractWords(input.text) }
        Button { text: "导入文件"; onClicked: appController.importFile(fileDialog.fileUrl) }
        Text { text: appController.statusMessage }
        ListView {
            width: 200; height: 200
            model: appController.newWordsList
            delegate: Text { text: modelData.word + ": " + modelData.translation }
        }
    }
}
```

---

### DatabaseManagementWindow

**头文件**：`tests/DatabaseManagementWindow.h`  
**基类**：`QWidget`

#### 构造函数

```cpp
explicit DatabaseManagementWindow(const QSqlDatabase &db, QWidget *parent = nullptr)
```
- `db`: 已打开的数据库连接，包含 `Words` 和 `stardict` 表。

#### 界面功能

- **工具栏**：表选择下拉框（Words / stardict）、搜索框（实时过滤、高亮匹配）、上下匹配导航按钮。
- **主区域**：`QTableView` 可编辑表格（双击单元格编辑、多选删除）。
- **右侧面板**：插入新行、删除选中行、刷新、导入 CSV、导出 CSV、导出 JSON。

#### 私有槽函数

- `onTableChanged(int index)` — 切换表时重新加载模型。
- `onSearchTextChanged(const QString &text)` — 启动定时搜索。
- `performSearch()` — 执行搜索并高亮匹配行。
- `onPrevMatch()`, `onNextMatch()` — 导航匹配项。
- `onInsertRow()`, `onDeleteRow()` — 插入/删除行。
- `onImportCsv()`, `onExportCsv()`, `onExportJson()` — 导入/导出数据。
- `onRefresh()` — 刷新模型。

#### 使用示例

```cpp
WordDatabaseManager wm;
DatabaseManagementWindow *win = new DatabaseManagementWindow(wm.database());
win->show();
```

---

### DebugTool

**头文件**：`tests/DebugTool.h`

#### 公开方法

- `QString generateDirectoryTree(const QString &path)`  
  返回指定路径的目录树文本（使用 `└──`, `├──` 等字符），自动排除 `build`, `.qtcreator`, `.git`, `Debug`, `Release` 等目录。

- `QString combineAllFilesForAI(const QString &rootPath = QString())`  
  递归遍历 `rootPath`（默认当前目录），收集所有源代码文件（扩展名如 `.cpp`, `.h`, `.hpp`, `.pro`, `.qml`, `.ui`, `.txt`, `.md`, `.xml`, `.json` 等），将每个文件的内容用注释包裹后拼接成一个字符串，末尾附加目录树。自动排除 `combined_for_ai.txt` 自身和 `combined_qml.txt`。

#### 私有辅助方法

- `void buildTree(const QDir &dir, const QString &prefix, QStringList &lines)` — 递归构建目录树。
- `void combineDirectoryRecursive(...)` — 递归遍历并调用 `appendFileContent`。
- `void appendFileContent(const QString &fileAbsolutePath, const QString &relativePath, QString &output)` — 将单个文件内容以 `// File: ...` 注释头尾包裹后追加到输出。

#### 使用示例

```cpp
DebugTool tool;
QString tree = tool.generateDirectoryTree("/path/to/project");
qDebug() << tree;

QString combined = tool.combineAllFilesForAI("/path/to/project");
QFile file("combined.txt");
file.open(QIODevice::WriteOnly);
file.write(combined.toUtf8());
file.close();
```

---

### DebugWindow

**头文件**：`tests/DebugWindow.h`  
**基类**：`QWidget`

#### 构造函数

```cpp
explicit DebugWindow(QWidget *parent = nullptr)
```
- 初始化 `WordDatabaseManager`（`euwe_words.db`）、`LookupService`、`OcrManager`。
- 创建 UI（左侧文本编辑区、操作按钮、调试输出区、命令行；右侧三个表格及操作按钮）。

#### 公开方法

- `void log(const QString &msg)` — 向调试输出区追加带时间戳的日志。

#### 私有槽函数

- `onExtractWords()` — 提取当前文本区域的单词，刷新右侧表格。
- `onImportFile()` — 导入文件（支持 txt/pdf/docx/pptx 等），自动 OCR 提取的图片。
- `onOcrImages()` — 手动选择图片进行 OCR。
- `onClearText()`, `onCopyText()`, `onHelp()`, `onAbout()` — 对应按钮操作。
- `onAddToKnown()`, `onRemoveFromKnown()` — 将选中的生词录入熟词库或移出熟词库。
- `onExportCsv()`, `onExportPdf()`, `onExportDoc()` — 导出生词表。
- `onOpenDatabaseManager()` — 打开数据库管理窗口。
- `onOcrFinished(const QString &mergedText)` — OCR 完成时将结果追加到文本区。
- `onCommandEntered()` — 处理命令行输入。

#### 命令行支持的命令

在输入框中按回车执行：

| 命令 | 说明 |
|------|------|
| `help` | 显示帮助信息。 |
| `checkDir` | 输出项目目录树到调试输出区。 |
| `exportCode [路径]` | 导出源代码到 `data/combined_for_ai.txt`。 |
| `exportQml` | 导出所有 QML 文件到 `data/combined_qml.txt`。 |
| `open qml <文件路径>` 或 `oq <路径>` | 在新窗口中运行 QML 文件（`oq main` 打开主界面）。 |
| `log clear` | 清空 `data/logs/` 下的所有 `.log` 文件并重新初始化日志目标。 |

#### 使用示例

```cpp
DebugWindow *dw = new DebugWindow();
dw->show();
dw->log("Application started");
```

---

### Widget (app)

**头文件**：`app/widget.h`  
**基类**：`QWidget`

#### 构造函数

```cpp
explicit Widget(QWidget *parent = nullptr)
```
- 加载 UI（`widget.ui`，实际未使用）。
- 创建 `DebugWindow` 并显示。
- 关闭自身。

#### 使用注意

此 `Widget` 仅为占位，实际应用程序入口为 `tests/main.cpp`，它会创建 `Widget`（但内部打开了 `DebugWindow`）。正常运行时，`tests/main.cpp` 会初始化日志、QML 引擎并加载主界面，不依赖 `app` 子项目。

---

## 占位符类（未完成）

以下类目前仅有空声明，未实现具体功能，可视为预留接口：

### SpacedRepetition

**头文件**：`core/learning/SpacedRepetition.h`

```cpp
class SpacedRepetition {
public:
    SpacedRepetition();
};
```

### WordStatistician

**头文件**：`core/statistics/WordStatistician.h`

```cpp
class WordStatistician {
public:
    WordStatistician();
};
```

### WordCorrection

**头文件**：`modules/ai/WordCorrection.h`

```cpp
class WordCorrection {
public:
    WordCorrection();
};
```

### LocalServer

**头文件**：`modules/server/LocalServer.h`

```cpp
class LocalServer {
public:
    LocalServer();
};
```

### UpdateChecker

**头文件**：`modules/updater/UpdateChecker.h`

```cpp
class UpdateChecker {
public:
    UpdateChecker();
};
```

### ResultDisplayWidget

**头文件**：`ui/result/ResultDisplayWidget.h`

```cpp
class ResultDisplayWidget {
public:
    ResultDisplayWidget();
};
```

---

## QML 组件详细说明

所有 QML 组件位于 `ui/main/` 和 `ui/result/`，通过上下文属性 `appController` 访问 `MainController`。

### CapsuleButton

**文件**：`ui/main/CapsuleButton.qml`  
**基类**：`Rectangle`

**属性**：
- `text: string` — 按钮文字。
- `iconType: string` — 图标类型，可选 `"play"`, `"upload"`, `"trash"`, `"copy"`, `"help"`, `"info"`。
- `isPrimary: bool` — 是否为主要按钮（背景色特殊）。
- `isDanger: bool` — 是否为危险按钮（红色系）。

**信号**：
- `clicked()` — 点击时发出。

**内部细节**：
- 按钮为胶囊形状（半径 17）。
- 颜色根据主题和悬停状态动态变化。
- 图标通过内嵌 SVG 数据动态生成，颜色随按钮状态变化。

**示例**：
```qml
CapsuleButton {
    text: "提取单词"
    iconType: "play"
    isPrimary: true
    onClicked: console.log("clicked")
}
```

### ExtractPage

**文件**：`ui/main/ExtractPage.qml`  
**基类**：`Rectangle`

**属性**：
- `rootWindow: var` — 主窗口引用（可选，用于显示对话框）。

**信号**：
- `textChanged(string fullText, int charCount, int wordCount, int lineCount)` — 文本内容变更时发出。
- `extractRequested()` — 提取按钮点击。
- `clearRequested()` — 清空按钮点击。
- `copyRequested()` — 复制按钮点击。
- `helpRequested()` — 帮助按钮点击。
- `aboutRequested()` — 关于按钮点击。
- `importRequested()` — 导入按钮点击。

**内部组件**：
- 主文本输入区（`TextArea`）。
- 右侧快捷操作面板（`CapsuleButton` 组）。

**使用示例**：
```qml
ExtractPage {
    onExtractRequested: appController.extractWords(mainInput.text)
    onTextChanged: { statusText = "已更新"; updateStats(charCount, wordCount, lineCount) }
}
```

### MessageDialog

**文件**：`ui/main/MessageDialog.qml`  
**基类**：`Popup`

**属性**：
- `dialogTitle: string` — 对话框标题。
- `dialogText: string` — 对话框内容。
- `standardButtons: string` — 按钮组合，可选 `"ok"`, `"okcancel"`, `"yesno"`。
- `type: string` — 类型，影响边框颜色（`"info"`, `"success"`, `"warning"`, `"error"`）。
- `onAccepted: function` — 确定/是按钮的回调。
- `onRejected: function` — 取消/否按钮的回调。

**方法**：
- `openCentered(win)` — 在窗口 `win` 中居中打开对话框。

**示例**：
```qml
var dialog = Qt.createComponent("MessageDialog.qml").createObject(rootWindow);
dialog.dialogTitle = "提示";
dialog.dialogText = "操作成功";
dialog.standardButtons = "ok";
dialog.openCentered(rootWindow);
```

### PanelGroup

**文件**：`ui/main/PanelGroup.qml`  
**基类**：`Item`

**属性**：
- `title: string` — 分组标题。

**用途**：在快捷操作面板中创建带标题的分隔线。

### ResultPage

**文件**：`ui/main/ResultPage.qml`  
**基类**：`Rectangle`

**属性**：
- `unknownModel: ListModel` — 未识别单词模型（单词字符串）。
- `newWordModel: ListModel` — 生词模型（包含 `word`, `translation`, `phonetic`）。
- `knownWordModel: ListModel` — 熟词模型（同上）。

**内部组件**：
- 三个 `SelectableWordList` 组件，分别展示未识别、生词、熟词。
- 底部按钮栏：导出（弹出菜单）、熟词库操作（录入/移出）、数据库管理、继续识别。

**方法**：
- `clearAllSelections()` — 清除所有列表的选择。
- `clearOtherList(listName)` — 清除其他列表的选择。

**示例**：
```qml
ResultPage {
    unknownModel: unknownModel
    newWordModel: newWordModel
    knownWordModel: knownWordModel
    mainWindow: rootWindow
}
```

### SelectableWordList

**文件**：`ui/main/SelectableWordList.qml`  
**基类**：`Item`

**属性**：
- `title: string` — 列表标题。
- `iconColor: color` — 标题图标颜色。
- `iconSvg: string` — 标题图标 SVG 字符串。
- `wordModel: ListModel` — 数据模型。
- `selectedIndices: var` — 当前选中的索引数组。
- `anchorIndex: int` — 用于范围选择的锚点索引。
- `activeListType: string` — 当前列表类型标识。
- `showTranslation: bool` — 是否显示释义列。
- `showPhonetic: bool` — 是否显示音标列。

**信号**：
- `selectionChanged(var indices)` — 选择变更时发出。
- `clearOtherLists(string activeType)` — 通知其他列表清除选择。

**方法**（供外部调用）：
- `selectAll()` — 全选。
- `clearSelection()` — 清空选择。
- `isSelected(index)` — 检查指定行是否选中。

**交互特性**：
- 支持 Ctrl+Click 切换选择、Shift+Click 范围选择。
- 支持拖拽选择（鼠标按下后拖动，自动滚动）。
- 支持 Ctrl+A 全选。
- 自动显示滚动条，悬停时变粗，1 秒无交互自动淡出。

**示例**：
```qml
SelectableWordList {
    title: "生词"
    iconColor: Theme.accent
    wordModel: newWordModel
    showTranslation: true
    showPhonetic: true
    onSelectionChanged: { console.log(selectedIndices) }
}
```

### SideBarItem

**文件**：`ui/main/SideBarItem.qml`  
**基类**：`Item`

**属性**：
- `iconType: string` — 图标类型，`"home"`, `"folder"`, `"file"`。
- `active: bool` — 是否激活状态。

**信号**：
- `clicked()`

### Theme

**文件**：`ui/main/Theme.qml`  
**类型**：`pragma Singleton`，QtObject

**属性**：
- `dark: bool` — 当前是否为暗色主题（可读写，改变时触发颜色动画）。

**颜色属性**（动态，随 `dark` 变化）：
- `bg`, `titleBg`, `sidebarBg`, `surface`, `border`, `borderInner`
- `textDark`, `textMid`, `textLight`
- `accent`
- `btnNormalBg`, `btnNormalBorder`, `btnPrimaryBg`, `btnPrimaryBorder`, `btnDangerBg`, `btnDangerBorder`, `btnDangerText`
- `primaryBtnBg`, `primaryBtnHover`, `primaryBtnPressed`, `primaryBtnText`
- `secondaryBtnBg`, `secondaryBtnBorder`, `secondaryBtnHoverBg`, `secondaryBtnHoverBorder`, `secondaryBtnText`
- `ghostBtnText`, `ghostBtnHoverBg`

**使用示例**：
```qml
Rectangle {
    color: Theme.bg
    Text { color: Theme.textDark }
}
```

### ThemeToggleBtn

**文件**：`ui/main/ThemeToggleBtn.qml`  
**基类**：`Rectangle`

**信号**：
- `toggleTheme()` — 点击时发出，通常连接到 `rootWindow.toggleTheme()`。

### WindowControlBtn

**文件**：`ui/main/WindowControlBtn.qml`  
**基类**：`Rectangle`

**属性**：
- `text: string` — 按钮文字（`"—"`, `"☐"`, `"✕"`）。
- `isClose: bool` — 是否为关闭按钮（特殊颜色）。

**信号**：
- `clicked()`

### WordManagePage

**文件**：`ui/main/WordManagePage.qml`  
**基类**：`Rectangle`

**属性**：
- `knownWordsModel: ListModel` — 熟词库原始模型。
- `filteredModel: ListModel` — 筛选后的显示模型（自动维护）。
- `rootWindow: var` — 主窗口引用。
- `selectedIndices: var` — 当前选中的索引列表。

**信号**：
- `addWord(string word, string translation, string phonetic)` — 添加单词。
- `deleteWord(string word)` — 删除单词。

**内部功能**：
- 左侧：`SelectableWordList` 展示筛选后的熟词。
- 右侧：Tab 面板（基础操作、高级查找、导入导出、统计）。
  - 基础操作：输入框添加单词、删除选中。
  - 高级查找：关键词搜索、区分大小写、精确匹配、字段选择。
  - 导入导出：导出 CSV/JSON（导入为演示占位）。
  - 统计：显示总词数和当前显示数量。

**使用示例**：
```qml
WordManagePage {
    knownWordsModel: knownWordModel
    rootWindow: rootWindow
    onAddWord: { knownWordModel.append({word, translation, phonetic}) }
    onDeleteWord: { for (var i=0; i<knownWordModel.count; ++i) if (knownWordModel.get(i).word === word) knownWordModel.remove(i) }
}
```

### WordTable

**文件**：`ui/main/WordTable.qml`  
**基类**：`Rectangle`

**属性**：
- `rootModel: ListModel` — 数据模型（包含 `word`, `translation`, `phonetic`）。
- `filterText: string` — 过滤文本。
- `selectedRow: int` — 当前选中的行索引。

**信号**：
- `rowSelected(int index, var itemData)`

**内部**：基于 `ListView` 的表格，支持过滤。

### ResultDisplayWidget (QML)

**文件**：`ui/result/ResultDisplayWidget.qml`  
**基类**：`ApplicationWindow`

这是一个独立的窗口样式预览组件，未被主界面使用，功能与 `ResultPage` 类似但设计不同，属于遗留或备用组件。

---

## 第三部分：构建与部署

### 依赖模块

- Qt 6.8+（Core, Gui, Widgets, Sql, Quick, QuickControls2, QuickLayouts, PrintSupport, Pdf, Concurrent）
- 外部 OCR 程序 `EUWEOCRT.exe` 和 `tessdata` 目录（需放置在可执行文件同目录）。
- SQLite 词典数据库：需包含表 `stardict`（字段 `word`, `phonetic`, `translation`），数据库文件与 `euwe_words.db` 可共用或单独提供。

### 构建步骤（Qt Creator）

1. 打开 `EUWE.pro`。
2. 选择构建套件（MinGW 11+）。
3. 依次构建子项目（qmake 自动处理依赖）。
4. 运行目标：`tests` 子项目生成的 `EUWE_tests.exe`（推荐入口，已包含完整日志和 QML 主界面）。
   注意: App为正式版本的软件子项目,目前尚未实现

### 部署注意事项

- 确保 `EUWEOCRT.exe` 和 `tessdata` 目录位于可执行文件同级目录。
- 若词典数据库文件不在默认位置，需修改 `WordDatabaseManager` 或 `LookupService` 的数据库路径参数。
- 日志文件默认写入 `data/logs/`，请确保该目录可写。

---

*文档版本：2.0 | 生成日期：2026-06-14 | 适用于 Obsidian，请将本文件置于 `docs/` 目录。*