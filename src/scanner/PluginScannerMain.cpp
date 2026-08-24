// PluginScannerMain.cpp - Out-of-process plugin scanner entry point.
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

#include "PluginScannerIPC.h"

#include <JuceHeader.h>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace PluginScannerIPC;

/// JUCE application class for the out-of-process plugin scanner.
///
/// Connects to the host via a named pipe, receives scan requests, and
/// sends back plugin descriptions or error codes.
class PluginScannerApplication : public juce::JUCEApplicationBase {
  public:
    PluginScannerApplication() = default;

    const juce::String getApplicationName() override { return "Pedalboard3Scanner"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void suspended() override {}
    void resumed() override {}
    /// Logs unhandled exceptions to stderr.
    ///
    /// @param exception The unhandled exception (unused).
    /// @param sourceFile Source file where the exception occurred (unused).
    /// @param lineNumber Line number where the exception occurred (unused).
    void unhandledException(const std::exception*, const juce::String&, int) override {
        std::cerr << "[Scanner] Unhandled exception caught" << std::endl;
    }

    /// Registers plugin formats, connects to the host pipe, and starts the
    /// message processing loop.
    ///
    /// @param commandLine Command line arguments passed to the scanner.
    void initialise(const juce::String& commandLine) override {
        std::cerr << "[Scanner] Starting Pedalboard3 Plugin Scanner" << std::endl;

        // JUCE 8 requires explicit format registration.
        formatManager.addFormat(std::make_unique<juce::VST3PluginFormat>());

        std::cerr << "[Scanner] Registered " << formatManager.getNumFormats() << " plugin formats" << std::endl;

        if (!connectToHost()) {
            std::cerr << "[Scanner] Failed to connect to host pipe" << std::endl;
            setApplicationReturnValue(1);
            quit();
            return;
        }

        std::cerr << "[Scanner] Connected to host, entering message loop" << std::endl;

        sendMessage(MessageType::Ready);

        startTimer(10);
    }

    /// Stops the timer and disconnects from the host.
    void shutdown() override {
        std::cerr << "[Scanner] Shutting down" << std::endl;
        stopTimer();
        disconnectFromHost();
    }

    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted(const juce::String&) override {}

  private:
    juce::AudioPluginFormatManager formatManager;

#ifdef _WIN32
    HANDLE pipeHandle = INVALID_HANDLE_VALUE;
#endif

    /// Connects to the host's named pipe, retrying up to 10 times.
    ///
    /// @return True if the pipe was connected successfully.
    bool connectToHost() {
#ifdef _WIN32
        for (int attempt = 0; attempt < 10; ++attempt) {
            pipeHandle = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

            if (pipeHandle != INVALID_HANDLE_VALUE) {
                DWORD mode = PIPE_READMODE_BYTE;
                SetNamedPipeHandleState(pipeHandle, &mode, nullptr, nullptr);
                return true;
            }

            if (GetLastError() == ERROR_PIPE_BUSY) {
                WaitNamedPipeA(PIPE_NAME, 1000);
            } else {
                Sleep(100);
            }
        }
        return false;
#else
        // Unix implementation would use Unix domain sockets.
        return false;
#endif
    }

    /// Closes the named pipe handle if open.
    void disconnectFromHost() {
#ifdef _WIN32
        if (pipeHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(pipeHandle);
            pipeHandle = INVALID_HANDLE_VALUE;
        }
#endif
    }

    /// Writes a message header and optional payload to the host pipe.
    ///
    /// @param type Message type to send.
    /// @param payload Optional string payload to include after the header.
    /// @return True if the message was written and flushed successfully.
    bool sendMessage(MessageType type, const juce::String& payload = {}) {
#ifdef _WIN32
        if (pipeHandle == INVALID_HANDLE_VALUE)
            return false;

        MessageHeader header;
        header.type = type;
        auto payloadBytes = payload.toUTF8();
        header.payloadSize = static_cast<uint32_t>(payloadBytes.length());

        DWORD bytesWritten;
        if (!WriteFile(pipeHandle, &header, sizeof(header), &bytesWritten, nullptr))
            return false;

        if (header.payloadSize > 0) {
            if (!WriteFile(pipeHandle, payloadBytes.getAddress(), header.payloadSize, &bytesWritten, nullptr))
                return false;
        }

        FlushFileBuffers(pipeHandle);
        return true;
#else
        return false;
#endif
    }

    /// Reads a message from the host pipe if data is available. Returns false
    /// when no complete message is pending.
    ///
    /// @param header Output parameter filled with the read message header.
    /// @param payload Output parameter filled with the read payload string.
    /// @return True if a complete valid message was read.
    bool readMessage(MessageHeader& header, juce::String& payload) {
#ifdef _WIN32
        if (pipeHandle == INVALID_HANDLE_VALUE)
            return false;

        DWORD bytesAvailable = 0;
        if (!PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr))
            return false;

        if (bytesAvailable < sizeof(MessageHeader))
            return false;

        DWORD bytesRead;
        if (!ReadFile(pipeHandle, &header, sizeof(header), &bytesRead, nullptr))
            return false;

        if (header.magic != 0x50444233)
            return false;

        if (header.payloadSize > 0) {
            juce::HeapBlock<char> buffer(header.payloadSize + 1);
            if (!ReadFile(pipeHandle, buffer.get(), header.payloadSize, &bytesRead, nullptr))
                return false;
            buffer[header.payloadSize] = 0;
            payload = juce::String::fromUTF8(buffer.get(), header.payloadSize);
        } else {
            payload.clear();
        }

        return true;
#else
        return false;
#endif
    }

    /// Schedules a timer callback after the given interval in milliseconds.
    ///
    /// @param intervalMs Delay in milliseconds before the timer callback fires.
    void startTimer(int intervalMs) {
        juce::Timer::callAfterDelay(intervalMs, [this]() { timerCallback(); });
    }

    void stopTimer() {}

    /// Processes pending messages and reschedules the next callback.
    void timerCallback() {
        processMessages();

        // Schedule next callback if still running
        if (!juce::JUCEApplicationBase::isStandaloneApp() || juce::JUCEApplicationBase::getInstance()->isInitialising())
            return;

        juce::Timer::callAfterDelay(10, [this]() { timerCallback(); });
    }

    /// Reads and dispatches all pending messages from the host pipe.
    void processMessages() {
        MessageHeader header;
        juce::String payload;

        while (readMessage(header, payload)) {
            switch (header.type) {
            case MessageType::Ping:
                sendMessage(MessageType::Pong);
                break;

            case MessageType::ScanPlugin:
                handleScanRequest(payload);
                break;

            case MessageType::Shutdown:
                std::cerr << "[Scanner] Received shutdown command" << std::endl;
                quit();
                return;

            default:
                std::cerr << "[Scanner] Unknown message type: " << static_cast<int>(header.type) << std::endl;
                break;
            }
        }
    }

    /// Deserializes a scan request, scans the plugin file for the requested
    /// format, and sends the result or error back to the host.
    ///
    /// @param payload JSON-serialized ScanRequest to process.
    void handleScanRequest(const juce::String& payload) {
        auto request = ScanRequest::deserialize(payload);
        std::cerr << "[Scanner] Scanning: " << request.pluginPath.toStdString() << std::endl;

        ScanResponse response;

        juce::AudioPluginFormat* format = nullptr;
        for (int i = 0; i < formatManager.getNumFormats(); ++i) {
            auto* f = formatManager.getFormat(i);
            if (f->getName() == request.formatName) {
                format = f;
                break;
            }
        }

        if (!format) {
            response.resultCode = ScanResultCode::InvalidFormat;
            response.errorMessage = "Unknown format: " + request.formatName;
            sendMessage(MessageType::ScanError, response.serialize());
            return;
        }

        juce::OwnedArray<juce::PluginDescription> results;
        format->findAllTypesForFile(results, request.pluginPath);

        if (results.isEmpty()) {
            response.resultCode = ScanResultCode::LoadFailed;
            response.errorMessage = "No plugins found in file";
            sendMessage(MessageType::ScanError, response.serialize());
            return;
        }

        juce::XmlElement root("PLUGINS");
        for (auto* desc : results) {
            if (auto xml = desc->createXml())
                root.addChildElement(xml.release());
        }

        response.resultCode = ScanResultCode::Success;
        response.pluginXml = root.toString();

        std::cerr << "[Scanner] Found " << results.size() << " plugin(s)" << std::endl;

        sendMessage(MessageType::ScanResult, response.serialize());
    }
};

START_JUCE_APPLICATION(PluginScannerApplication)
