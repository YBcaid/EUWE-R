## AppConfig 操作文档

`AppConfig` 是一个线程安全的应用程序配置管理类（单例），基于 JSON 文件持久化，支持层级键（如 `"window/geometry/width"`）。提供类型安全的存取接口，具备默认值、自动保存、脏标记和惰性加载机制。

---

### 1. 初始化

在首次使用前（例如 `main` 函数中）调用 `initialize()` 指定配置文件路径。如果不调用，将使用默认路径：
`QStandardPaths::writableLocation(AppDataLocation) + "/config.json"`

```cpp
// 使用默认路径
AppConfig::initialize();

// 或自定义完整路径
AppConfig::initialize("C:/MyApp/settings/config.json");
```

> **注意**：`initialize()` 必须在任何配置读写操作之前调用。若已加载过配置（如已调用 `instance()`），再次调用 `initialize()` 并传入不同路径会被忽略，并输出警告。

---

### 2. 获取单例

```cpp
AppConfig &cfg = AppConfig::instance();
```

首次调用 `instance()` 时会触发惰性加载（从磁盘读取配置文件）。加载失败（文件不存在、解析错误等）会初始化空对象，不会抛出异常。

---

### 3. 读写配置

#### 3.1 读取值（类型安全模板）

```cpp
// 读取字符串，不存在时返回默认值
QString host = cfg.value("network/host", QString("127.0.0.1"));

// 读取整数
int port = cfg.value("network/port", 8080);

// 读取布尔值
bool logging = cfg.value("debug/logging", false);
```

#### 3.2 读取值（QVariant 版本）

```cpp
QVariant val = cfg.value("network/timeout", 5000);
int timeout = val.toInt();
```

#### 3.3 设置值

```cpp
cfg.setValue("network/host", "192.168.1.100");   // 不立即保存
cfg.setValue("network/port", 80, true);          // 立即保存到磁盘
```

#### 3.4 删除键

```cpp
if (cfg.remove("debug/logging")) {
    // 删除成功，会发出 configChanged 信号
}
```

#### 3.5 检查键是否存在

```cpp
if (cfg.contains("network/host")) { ... }
```

---

### 4. 查询子组与键

```cpp
// 获取顶级所有组（值为对象的键）
QStringList groups = cfg.childGroups();   // e.g. ["network", "window"]

// 获取 network 组下的所有叶子键（完整层级路径）
QStringList keys = cfg.childKeys("network");
// 可能返回 ["network/host", "network/port"]
```

---

### 5. 自动保存与自动写入默认值

```cpp
// 启用自动保存（每次 setValue 后自动保存到磁盘）
cfg.setAutoSaveEnabled(true);

// 启用自动写入默认值（读取不存在的键时，将默认值写入配置）
cfg.setAutoWriteDefault(true);
```

---

### 6. 监听信号

```cpp
// 配置值变更（setValue、remove、自动写入默认值时触发）
QObject::connect(&cfg, &AppConfig::configChanged,
    [](const QString &key, const QVariant &value) {
        if (value.isValid()) {
            qDebug() << "Config changed:" << key << "=" << value;
        } else {
            qDebug() << "Config key removed:" << key;
        }
    });

// 配置文件被重新加载（调用 load() 成功时触发）
QObject::connect(&cfg, &AppConfig::configReloaded, []() {
    qDebug() << "Configuration reloaded from disk.";
});
```

---

### 7. 主动保存与重载

```cpp
cfg.save();   // 手动保存当前配置到磁盘
cfg.load();   // 从磁盘重新加载并合并到内存（覆盖已有键）
```

---

### 8. 完整示例

```cpp
#include <QCoreApplication>
#include <QDebug>
#include "appconfig.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. 初始化（自定义路径）
    AppConfig::initialize("myapp_config.json");

    // 2. 获取单例
    AppConfig &cfg = AppConfig::instance();

    // 3. 连接信号
    QObject::connect(&cfg, &AppConfig::configChanged,
        [](const QString &key, const QVariant &val) {
            qDebug() << "Config change:" << key << val;
        });

    // 4. 读取配置（带默认值）
    int width  = cfg.value("window/width",  800);
    int height = cfg.value("window/height", 600);
    qDebug() << "Window size:" << width << "x" << height;

    // 5. 修改配置并立即保存
    cfg.setValue("window/width", 1024, true);
    cfg.setValue("window/height", 768, true);

    // 6. 读取自动写入的默认值
    cfg.setAutoWriteDefault(true);
    QString theme = cfg.value("ui/theme", QString("dark"));
    // 此时 "ui/theme" 不存在，会将 "dark" 写入配置并发出 configChanged

    // 7. 保存到磁盘（如果未使用自动保存）
    cfg.save();

    return 0;
}
```

---

### 线程安全说明

- 所有公有接口内部均使用 `QMutex` 保护，可安全地在多线程中调用。
- 信号在解锁后发出，避免死锁，但信号槽中应避免长时间阻塞或再次调用配置接口（可能导致递归锁等待）。
- 惰性加载通过 `mutable` 成员实现，`const` 方法（如 `contains`, `childKeys`）也能安全触发初始化。

---

### 注意事项

1. **配置文件格式**：必须是合法的 JSON 对象（如 `{}` 包裹），否则加载失败时会被重置为空对象。
2. **合并策略**：`load()` 仅合并顶层键，嵌套对象会被整体替换，不是深度合并。
3. **空键警告**：`setValue` 传入空键会输出警告并忽略操作。
4. **类型转换**：模板 `value<T>()` 若实际存储的类型无法转换为 `T`，会返回传入的 `defaultValue`。
5. **析构自动保存**：程序正常退出时，析构函数会尝试保存当前配置到磁盘（若已初始化）。

---

*基于 Qt 6.8 / C++17，兼容 Qt 5.12+（需注意 `QLatin1Char` 用法）*