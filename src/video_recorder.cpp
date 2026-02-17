// SPDX-License-Identifier: Apache-2.0
//
// Copyright (c) 2026 Qi Wang
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "video_recorder.h"

#include <QByteArray>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>
#include <cstring>

namespace octo_flex {

VideoRecorder::VideoRecorder() : process_(std::make_unique<QProcess>()) {}

VideoRecorder::~VideoRecorder() {
    std::string ignored;
    stop(&ignored);
}

bool VideoRecorder::start(const VideoRecorderOptions& options, std::string* error) {
    if (started_) {
        setError("Recorder already started", error);
        return false;
    }

    if (options.width <= 0 || options.height <= 0 || options.fps <= 0 || options.outputPath.empty()) {
        setError("Invalid recorder options", error);
        return false;
    }

    options_ = options;

    const QString ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
    if (ffmpegPath.isEmpty()) {
        setError("ffmpeg executable was not found in PATH", error);
        return false;
    }

    QStringList args;
    args << "-hide_banner" << "-loglevel" << "error";
    args << (options.overwrite ? "-y" : "-n");
    args << "-f" << "rawvideo";
    args << "-pix_fmt" << (options.enableAlpha ? "rgba" : "rgb24");
    args << "-s" << QString("%1x%2").arg(options.width).arg(options.height);
    args << "-r" << QString::number(options.fps);
    args << "-i" << "-";
    args << "-an";
    args << "-c:v" << QString::fromStdString(options.codec);
    args << "-preset" << QString::fromStdString(options.preset);
    args << "-crf" << QString::number(options.crf);
    args << "-pix_fmt" << (options.enableAlpha ? "yuva420p" : "yuv420p");
    args << QString::fromStdString(options.outputPath);

    process_->setProcessChannelMode(QProcess::MergedChannels);
    process_->start(ffmpegPath, args, QIODevice::WriteOnly);

    if (!process_->waitForStarted(3000)) {
        setError("Failed to start ffmpeg process", error);
        return false;
    }

    started_ = true;
    return true;
}

bool VideoRecorder::writeFrame(const QImage& frame, std::string* error) {
    if (!started_ || !process_ || process_->state() != QProcess::Running) {
        setError("Recorder is not running", error);
        return false;
    }

    QImage processed = frame;
    if (options_.enableAlpha) {
        // Preserve alpha channel by using RGBA format
        if (processed.format() != QImage::Format_RGBA8888) {
            processed = processed.convertToFormat(QImage::Format_RGBA8888);
        }
    } else {
        // Discard alpha channel for RGB format
        if (processed.format() != QImage::Format_RGB888) {
            processed = processed.convertToFormat(QImage::Format_RGB888);
        }
    }

    if (processed.width() != options_.width || processed.height() != options_.height) {
        processed = processed.scaled(options_.width, options_.height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const int rowBytes = options_.width * (options_.enableAlpha ? 4 : 3);
    QByteArray packed;
    packed.resize(rowBytes * options_.height);

    for (int y = 0; y < options_.height; ++y) {
        const char* src = reinterpret_cast<const char*>(processed.constScanLine(y));
        char* dst = packed.data() + y * rowBytes;
        memcpy(dst, src, rowBytes);
    }

    bool result = writeAll(packed.constData(), packed.size(), error);
    if (result) {
        framesWritten_++;
    }
    return result;
}

bool VideoRecorder::stop(std::string* error) {
    if (!process_) {
        started_ = false;
        return true;
    }

    if (process_->state() == QProcess::NotRunning) {
        started_ = false;
        return true;
    }

    process_->closeWriteChannel();
    if (!process_->waitForFinished(30000)) {
        process_->terminate();
        if (!process_->waitForFinished(3000)) {
            process_->kill();
            process_->waitForFinished(1000);
        }
        started_ = false;
        setError("ffmpeg did not stop cleanly (timeout)", error);
        return false;
    }

    const bool ok = (process_->exitStatus() == QProcess::NormalExit && process_->exitCode() == 0);
    if (!ok) {
        const QString ffmpegOutput = QString::fromUtf8(process_->readAll());
        const std::string message =
            ffmpegOutput.isEmpty()
                ? "ffmpeg exited with an error"
                : ("ffmpeg exited with an error: " + ffmpegOutput.left(512).toStdString());
        started_ = false;
        setError(message, error);
        return false;
    }

    started_ = false;
    return true;
}

bool VideoRecorder::isRunning() const { return started_ && process_ && process_->state() == QProcess::Running; }

bool VideoRecorder::writeAll(const char* data, qint64 size, std::string* error) {
    qint64 written = 0;
    while (written < size) {
        const qint64 n = process_->write(data + written, size - written);
        if (n <= 0) {
            if (!process_->waitForBytesWritten(2000)) {
                setError("Failed writing frame to ffmpeg process", error);
                return false;
            }
            continue;
        }
        written += n;
    }
    return true;
}

void VideoRecorder::setError(const std::string& message, std::string* error) {
    lastError_ = message;
    if (error) {
        *error = message;
    }
}

}  // namespace octo_flex
