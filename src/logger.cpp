#include "qt-ai-desktop/logger.h"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <iostream>
#include <cstdarg>

namespace QtAIDesktop {

// ============================================================================
// LogMessage
// ============================================================================

QString LogMessage::levelString() const {
    switch (level) {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO ";
    case LogLevel::Warning: return "WARN ";
    case LogLevel::Error:   return "ERROR";
    case LogLevel::Fatal:   return "FATAL";
    }
    return "?????";
}

QString LogMessage::formatted() const {
    QString time = timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString loc;
    if (file) {
        QString fileName = QFileInfo(file).fileName();
        loc = QString(" [%1:%2]").arg(fileName).arg(line);
    }
    
    return QString("[%1] [%2] [%3]%4: %5")
        .arg(time)
        .arg(levelString())
        .arg(category)
        .arg(loc)
        .arg(message);
}

// ============================================================================
// Logger
// ============================================================================

Logger::Logger() {
    m_recentMessages.reserve(1000);
}

Logger::~Logger() {
    flush();
    delete m_stream;
    delete m_file;
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    QMutexLocker locker(&m_mutex);
    m_level = level;
}

LogLevel Logger::level() const {
    return m_level;
}

void Logger::setTarget(LogTarget target) {
    QMutexLocker locker(&m_mutex);
    m_target = target;
}

bool Logger::setLogFile(const QString &filePath) {
    QMutexLocker locker(&m_mutex);
    
    // 关闭旧文件
    if (m_stream) {
        m_stream->flush();
        delete m_stream;
        m_stream = nullptr;
    }
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
    
    if (filePath.isEmpty()) {
        return false;
    }
    
    // 确保目录存在
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 打开新文件
    m_file = new QFile(filePath);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        delete m_file;
        m_file = nullptr;
        return false;
    }
    
    m_stream = new QTextStream(m_file);
    m_stream->setEncoding(QStringConverter::Utf8);
    
    return true;
}

void Logger::setCategoryFilter(const QStringList &categories) {
    QMutexLocker locker(&m_mutex);
    m_categoryFilter = categories;
}

bool Logger::shouldLog(const QString &category) const {
    if (m_categoryFilter.isEmpty()) {
        return true;
    }
    return m_categoryFilter.contains(category);
}

void Logger::log(LogLevel level, const QString &category, const QString &message,
                 const char *file, int line, const char *function) {
    if (level < m_level) {
        return;
    }
    
    if (!shouldLog(category)) {
        return;
    }
    
    LogMessage msg;
    msg.level = level;
    msg.category = category;
    msg.message = message;
    msg.file = file;
    msg.line = line;
    msg.function = function;
    msg.timestamp = QDateTime::currentDateTime();
    
    QMutexLocker locker(&m_mutex);
    
    // 保存到最近消息
    m_recentMessages.append(msg);
    if (m_recentMessages.size() > 1000) {
        m_recentMessages.removeFirst();
    }
    
    // 输出到控制台
    if (static_cast<int>(m_target) & static_cast<int>(LogTarget::Console)) {
        writeToConsole(msg);
    }
    
    // 输出到文件
    if (static_cast<int>(m_target) & static_cast<int>(LogTarget::File)) {
        writeToFile(msg);
    }
}

void Logger::writeToConsole(const LogMessage &msg) {
    QTextStream &out = (msg.level >= LogLevel::Warning) ? std::cerr : std::cout;
    out << msg.formatted() << "\n";
    out.flush();
}

void Logger::writeToFile(const LogMessage &msg) {
    if (!m_stream) {
        return;
    }
    
    *m_stream << msg.formatted() << "\n";
    m_stream->flush();
}

void Logger::flush() {
    QMutexLocker locker(&m_mutex);
    if (m_stream) {
        m_stream->flush();
    }
}

QList<LogMessage> Logger::recentMessages(int count) const {
    QMutexLocker locker(&m_mutex);
    int start = qMax(0, m_recentMessages.size() - count);
    return m_recentMessages.mid(start);
}

void Logger::clearRecent() {
    QMutexLocker locker(&m_mutex);
    m_recentMessages.clear();
}

} // namespace QtAIDesktop
