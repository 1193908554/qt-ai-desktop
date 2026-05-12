#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <memory>

namespace QtAIDesktop {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    Debug = 0,
    Info,
    Warning,
    Error,
    Fatal
};

/**
 * @brief 日志输出目标
 */
enum class LogTarget {
    Console = 1,
    File = 2,
    Both = 3
};

/**
 * @brief 结构化日志消息
 */
struct LogMessage {
    LogLevel level;
    QString category;
    QString message;
    QString file;
    int line;
    QString function;
    QDateTime timestamp;
    
    QString formatted() const;
    QString levelString() const;
};

/**
 * @brief 日志系统
 * 
 * 支持多级别、多输出目标、分类过滤、格式化输出。
 * 线程安全，适合多线程环境。
 * 
 * @example
 * @code
 * // 基本使用
 * QTAI_LOG_INFO("Engine", "Model loaded successfully");
 * QTAI_LOG_ERROR("Engine", "Failed to load model: %1", path);
 * 
 * // 配置日志系统
 * Logger::instance().setLevel(LogLevel::Debug);
 * Logger::instance().setTarget(LogTarget::Both);
 * Logger::instance().setLogFile("app.log");
 * @endcode
 */
class Logger {
public:
    static Logger& instance();
    
    /**
     * @brief 设置日志级别
     */
    void setLevel(LogLevel level);
    LogLevel level() const;
    
    /**
     * @brief 设置输出目标
     */
    void setTarget(LogTarget target);
    
    /**
     * @brief 设置日志文件路径
     * @return 是否成功打开文件
     */
    bool setLogFile(const QString &filePath);
    
    /**
     * @brief 设置分类过滤器
     * @param categories 允许输出的分类列表，空列表表示允许所有
     */
    void setCategoryFilter(const QStringList &categories);
    
    /**
     * @brief 输出日志
     */
    void log(LogLevel level, const QString &category, const QString &message,
             const char *file = nullptr, int line = 0, const char *function = nullptr);
    
    /**
     * @brief 输出日志（带格式化）
     */
    void logf(LogLevel level, const QString &category, const QString &fmt,
              const char *file = nullptr, int line = 0, const char *function = nullptr, ...);
    
    /**
     * @brief 刷新缓冲区
     */
    void flush();
    
    /**
     * @brief 获取最近的日志消息
     * @param count 最大返回数量
     */
    QList<LogMessage> recentMessages(int count = 100) const;
    
    /**
     * @brief 清除最近日志
     */
    void clearRecent();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void writeToFile(const LogMessage &msg);
    void writeToConsole(const LogMessage &msg);
    bool shouldLog(const QString &category) const;
    
    LogLevel m_level = LogLevel::Info;
    LogTarget m_target = LogTarget::Console;
    QFile *m_file = nullptr;
    QTextStream *m_stream = nullptr;
    QStringList m_categoryFilter;
    QList<LogMessage> m_recentMessages;
    mutable QMutex m_mutex;
};

// ============================================================================
// 便捷宏
// ============================================================================
#define QTAI_LOG_DEBUG(cat, msg) \
    QtAIDesktop::Logger::instance().log(QtAIDesktop::LogLevel::Debug, cat, msg, __FILE__, __LINE__, __FUNCTION__)

#define QTAI_LOG_INFO(cat, msg) \
    QtAIDesktop::Logger::instance().log(QtAIDesktop::LogLevel::Info, cat, msg, __FILE__, __LINE__, __FUNCTION__)

#define QTAI_LOG_WARN(cat, msg) \
    QtAIDesktop::Logger::instance().log(QtAIDesktop::LogLevel::Warning, cat, msg, __FILE__, __LINE__, __FUNCTION__)

#define QTAI_LOG_ERROR(cat, msg) \
    QtAIDesktop::Logger::instance().log(QtAIDesktop::LogLevel::Error, cat, msg, __FILE__, __LINE__, __FUNCTION__)

#define QTAI_LOG_FATAL(cat, msg) \
    QtAIDesktop::Logger::instance().log(QtAIDesktop::LogLevel::Fatal, cat, msg, __FILE__, __LINE__, __FUNCTION__)

} // namespace QtAIDesktop
