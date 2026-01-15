#pragma once

//==============================================================================
// Logger.h - ログ出力クラス
//==============================================================================

#include "main.h"
#include <sstream>

//==============================================================================
// ログレベル
//==============================================================================
enum class LogLevel
{
    Info,
    Warning,
    Error,
};

//==============================================================================
// Logger クラス
// - 統一的なログ出力
// - Visual Studio出力ウィンドウ / コンソールに出力
//==============================================================================
class Logger
{
public:
    // ログ出力
    static void Log(LogLevel level, const char* message);
    static void Log(LogLevel level, const std::string& message);

    // 便利関数
    static void Info(const char* message);
    static void Info(const std::string& message);

    static void Warning(const char* message);
    static void Warning(const std::string& message);

    static void Error(const char* message);
    static void Error(const std::string& message);

    // フォーマット付きログ（printf形式）
    static void LogFormat(LogLevel level, const char* format, ...);
    static void InfoFormat(const char* format, ...);
    static void WarningFormat(const char* format, ...);
    static void ErrorFormat(const char* format, ...);

private:
    static const char* GetLevelString(LogLevel level);
    static void Output(const std::string& text);
};