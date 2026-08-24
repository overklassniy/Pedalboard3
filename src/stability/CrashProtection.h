// CrashProtection.h - Defensive crash protection for plugin operations.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported from the Pedalboard3-VST3 fork by Project12x.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <spdlog/spdlog.h>
#include <thread>


/// Result of a timed operation
enum class TimedOperationResult
{
    Success,      ///< Operation completed successfully
    Exception,    ///< Operation threw an exception (SEH or C++)
    Timeout,      ///< Operation exceeded timeout
    Cancelled     ///< Operation was cancelled
};

/// Provides defensive crash protection for risky plugin operations.
///
/// Features:
/// - SEH wrappers for catching hardware exceptions on Windows
/// - Auto-save triggers before risky operations
/// - Watchdog thread to detect UI hangs
/// - Timeout protection for hung operations
/// - Crash state logging for diagnostics
class CrashProtection
{
  public:
    /// Get the singleton instance
    static CrashProtection& getInstance();

    /// Execute a function with SEH protection (Windows only).
    /// @param operation The function to execute safely
    /// @param operationName Name for logging
    /// @param pluginName Optional plugin name for crash context
    /// @return true if operation completed without exception
    bool executeWithProtection(std::function<void()> operation, const juce::String& operationName,
                               const juce::String& pluginName = {});

    /// Execute a function with timeout protection.
    /// Runs the operation in a separate thread and waits for completion or timeout.
    /// @param operation The function to execute
    /// @param operationName Name for logging
    /// @param timeoutMs Timeout in milliseconds
    /// @param pluginPath Optional plugin path for auto-blacklisting on timeout
    /// @return TimedOperationResult indicating success, timeout, or exception
    TimedOperationResult executeWithTimeout(std::function<void()> operation, const juce::String& operationName,
                                            int timeoutMs, const juce::String& pluginPath = {});

    /// Execute a function with both SEH protection and timeout.
    /// Combines SEH wrapping with timeout protection for maximum safety.
    /// @param operation The function to execute
    /// @param operationName Name for logging
    /// @param timeoutMs Timeout in milliseconds
    /// @param pluginPath Optional plugin path for auto-blacklisting
    /// @return TimedOperationResult indicating the outcome
    TimedOperationResult executeWithProtectionAndTimeout(std::function<void()> operation,
                                                         const juce::String& operationName, int timeoutMs,
                                                         const juce::String& pluginPath = {});

    /// Set the current operation context for crash logs.
    /// Call before risky operations so crash logs know what was happening.
    void setCurrentOperation(const juce::String& operation, const juce::String& pluginName = {});

    /// Clear the current operation context.
    /// Call after operation completes successfully.
    void clearCurrentOperation();

    /// Get the current operation for crash logging.
    juce::String getCurrentOperation() const;

    /// Get the current plugin name for crash logging.
    juce::String getCurrentPluginName() const;

    /// Set auto-save callback to be called before risky operations.
    void setAutoSaveCallback(std::function<void()> callback);

    /// Trigger the auto-save callback.
    void triggerAutoSave();

    /// Start the watchdog thread.
    /// @param timeoutMs Timeout in milliseconds before considering UI hung
    void startWatchdog(int timeoutMs = 10000);

    /// Stop the watchdog thread.
    void stopWatchdog();

    /// Ping the watchdog to indicate the UI is responsive.
    /// Call this from the message thread periodically.
    void pingWatchdog();

    /// Check if the watchdog detected a hang.
    bool isHangDetected() const { return hangDetected.load(); }

    /// Write crash context to log file.
    /// Call this from a crash handler.
    void writeCrashContext();

  private:
    CrashProtection();
    ~CrashProtection();

    CrashProtection(const CrashProtection&) = delete;
    CrashProtection& operator=(const CrashProtection&) = delete;

    void watchdogLoop();

    juce::String currentOperation;
    juce::String currentPluginName;
    mutable juce::CriticalSection operationLock;

    std::function<void()> autoSaveCallback;

    // Watchdog
    std::atomic<bool> watchdogRunning{false};
    std::atomic<bool> hangDetected{false};
    std::atomic<std::chrono::steady_clock::time_point> lastPing;
    std::thread watchdogThread;
    int watchdogTimeoutMs = 10000;

    // Timeout operation state
    std::atomic<bool> timeoutOperationRunning{false};
    std::atomic<bool> timeoutOperationComplete{false};
    std::atomic<bool> timeoutOperationSuccess{false};
};

/// RAII helper to set/clear operation context.
class ScopedOperationContext
{
  public:
    ScopedOperationContext(const juce::String& operation, const juce::String& pluginName = {})
    {
        CrashProtection::getInstance().setCurrentOperation(operation, pluginName);
    }

    ~ScopedOperationContext() { CrashProtection::getInstance().clearCurrentOperation(); }
};

// Macro for wrapping risky operations
#define PROTECTED_OPERATION(name, plugin, code)                                                                        \
    CrashProtection::getInstance().executeWithProtection([&]() { code; }, name, plugin)

