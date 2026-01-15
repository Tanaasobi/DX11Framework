//==============================================================================
// Logger.cpp - ログ出力クラス実装
//==============================================================================

#include "Logger.h"
#include <cstdarg>
#include <cstdio>

//==============================================================================
// ログ出力
//==============================================================================
void Logger::Log(LogLevel level, const char* message)
{
    Log(level, std::string(message));
}

void Logger::Log(LogLevel level, const std::string& message)
{
    std::stringstream ss;
    ss << "[" << GetLevelString(level) << "] " << message << "\n";
    Output(ss.str());
}

//==============================================================================
// 便利関数
//==============================================================================
void Logger::Info(const char* message)
{
    Log(LogLevel::Info, message);
}

void Logger::Info(const std::string& message)
{
    Log(LogLevel::Info, message);
}

void Logger::Warning(const char* message)
{
    Log(LogLevel::Warning, message);
}

void Logger::Warning(const std::string& message)
{
    Log(LogLevel::Warning, message);
}

void Logger::Error(const char* message)
{
    Log(LogLevel::Error, message);
}

void Logger::Error(const std::string& message)
{
    Log(LogLevel::Error, message);
}

//==============================================================================
// フォーマット付きログ
//==============================================================================
void Logger::LogFormat(LogLevel level, const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Log(level, buffer);
}

void Logger::InfoFormat(const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Info(buffer);
}

void Logger::WarningFormat(const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Warning(buffer);
}

void Logger::ErrorFormat(const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Error(buffer);
}

//==============================================================================
// レベル文字列取得
//==============================================================================
const char* Logger::GetLevelString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARNING";
    case LogLevel::Error:   return "ERROR";
    default:                return "UNKNOWN";
    }
}

//==============================================================================
// 出力処理
//==============================================================================
void Logger::Output(const std::string& text)
{
    // Visual Studio 出力ウィンドウに出力
    OutputDebugStringA(text.c_str());

    // コンソールにも出力（デバッグビルド時）
#ifdef _DEBUG
    printf("%s", text.c_str());
#endif
}