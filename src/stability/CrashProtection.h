// CrashProtection.h - Defensive crash protection for plugin operations.
//
// This file is part of Pedalboard3, an audio plugin host.
// Ported and modified from the Pedalboard3 fork by Project12x.
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

/// Outcome of a timed, protected operation.
enum class TimedOperationResult {
    /// Operation completed successfully.
    Success,
    /// Operation threw an exception (SEH or C++).
    Exception,
    /// Operation exceeded the configured timeout.
    Timeout,
    /// Operation was cancelled before completion.
    Cancelled
};

/// Provides defensive crash protection for risky plugin operations.
///
/// Features:
/// - SEH wrappers for catching hardware exceptions on Windows
/// - Auto-save triggers before risky operations
/// - Watchdog thread to detect UI hangs
/// - Timeout protection for hung operations
/// - Crash state logging for diagnostics
class CrashProtection {
  public:
    /// Returns the singleton instance.
    ///
    /// @return The global CrashProtection instance.
    static CrashProtection& getInstance();

    /// Executes a function with SEH protection (Windows only).
    ///
    /// @param operation The function to run safely.
    /// @param operationName Name used for logging.
    /// @param pluginName When provided, recorded in the crash context so a failing plugin can be identified.
    /// @return True if the operation completed without throwing.
    bool executeWithProtection(std::function<void()> operation, const juce::String& operationName,
                               const juce::String& pluginName = {});

    /// Executes a function with timeout protection.
    ///
    /// Runs operation in a separate thread and waits for completion or the
    /// timeout.
    ///
    /// @param operation The function to run.
    /// @param operationName Name used for logging.
    /// @param timeoutMs Maximum time to wait in milliseconds.
    /// @param pluginPath When provided, auto-blacklisted if the operation times out.
    /// @return The outcome as a TimedOperationResult.
    TimedOperationResult executeWithTimeout(std::function<void()> operation, const juce::String& operationName,
                                            int timeoutMs, const juce::String& pluginPath = {});

    /// Executes a function with both SEH protection and a timeout.
    ///
    /// Combines SEH wrapping with timeout protection for maximum safety.
    ///
    /// @param operation The function to run.
    /// @param operationName Name used for logging.
    /// @param timeoutMs Maximum time to wait in milliseconds.
    /// @param pluginPath When provided, auto-blacklisted on timeout.
    /// @return The outcome as a TimedOperationResult.
    TimedOperationResult executeWithProtectionAndTimeout(std::function<void()> operation,
                                                         const juce::String& operationName, int timeoutMs,
                                                         const juce::String& pluginPath = {});

    /// Sets the current operation context for crash logs.
    ///
    /// Call before risky operations so crash logs record what was happening.
    ///
    /// @param operation Description of the current operation.
    /// @param pluginName When provided, identifies the plugin involved.
    void setCurrentOperation(const juce::String& operation, const juce::String& pluginName = {});

    /// Clears the current operation context.
    ///
    /// Call after an operation completes so stale context is not logged on a
    /// later crash.
    void clearCurrentOperation();

    /// Returns the current operation string for crash logging.
    juce::String getCurrentOperation() const;

    /// Returns the current plugin name for crash logging.
    juce::String getCurrentPluginName() const;

    /// Sets the callback invoked before risky operations to trigger auto-save.
    ///
    /// @param callback Function to call before risky operations.
    void setAutoSaveCallback(std::function<void()> callback);

    /// Invokes the registered auto-save callback, if any.
    void triggerAutoSave();

    /// Starts the watchdog thread.
    ///
    /// @param timeoutMs How long without a ping before the UI is considered hung.
    void startWatchdog(int timeoutMs = 10000);

    /// Stops the watchdog thread.
    void stopWatchdog();

    /// Pings the watchdog to indicate the UI is responsive.
    ///
    /// Call this periodically from the message thread.
    void pingWatchdog();

    /// Returns true if the watchdog has detected a UI hang.
    bool isHangDetected() const { return hangDetected.load(); }

    /// Writes the current crash context to the log file.
    ///
    /// Call this from a crash handler.
    void writeCrashContext();

  private:
    CrashProtection();
    ~CrashProtection();

    CrashProtection(const CrashProtection&) = delete;
    CrashProtection& operator=(const CrashProtection&) = delete;

    /// Watchdog thread loop that flags a hang when pings stop arriving.
    void watchdogLoop();

    /// Current operation description, written to crash logs.
    juce::String currentOperation;
    /// Plugin involved in the current operation, written to crash logs.
    juce::String currentPluginName;
    /// Guards access to the operation context strings.
    mutable juce::CriticalSection operationLock;

    /// Callback invoked before risky operations to trigger auto-save.
    std::function<void()> autoSaveCallback;

    // Watchdog state
    std::atomic<bool> watchdogRunning{false};
    std::atomic<bool> hangDetected{false};
    std::atomic<std::chrono::steady_clock::time_point> lastPing;
    std::thread watchdogThread;
    int watchdogTimeoutMs = 10000;

    // Timeout operation state shared with the worker thread
    std::atomic<bool> timeoutOperationRunning{false};
    std::atomic<bool> timeoutOperationComplete{false};
    std::atomic<bool> timeoutOperationSuccess{false};
};

/// RAII helper that sets the operation context on construction and clears it
/// on destruction, so the context is always cleaned up even on early returns.
class ScopedOperationContext {
  public:
    ScopedOperationContext(const juce::String& operation, const juce::String& pluginName = {}) {
        CrashProtection::getInstance().setCurrentOperation(operation, pluginName);
    }

    ~ScopedOperationContext() { CrashProtection::getInstance().clearCurrentOperation(); }
};

/// Wraps a risky code block in executeWithProtection.
#define PROTECTED_OPERATION(name, plugin, code)                                                                        \
    CrashProtection::getInstance().executeWithProtection([&]() { code; }, name, plugin)
