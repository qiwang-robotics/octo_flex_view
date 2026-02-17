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

#ifndef RECORDING_THREAD_H
#define RECORDING_THREAD_H

#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <queue>
#include <atomic>
#include <string>
#include "video_recorder.h"

namespace octo_flex {

class RecordingThread : public QThread {
    Q_OBJECT

   public:
    explicit RecordingThread(QObject* parent = nullptr);
    ~RecordingThread() override;

    // Start recording with the given options.
    bool startRecording(const VideoRecorderOptions& options, std::string* error = nullptr);

    // Queue a frame for recording (non-blocking).
    // Returns false if the queue is full or recording is not active.
    bool queueFrame(const QImage& frame);

    // Stop recording and wait for the thread to finish.
    // Returns true if all queued frames were written successfully.
    bool stopRecording(std::string* error = nullptr);

    // Check if recording is active.
    bool isRecording() const;

    // Get the number of queued frames.
    int getQueueSize() const;

    // Set maximum queue size (default: 60 frames).
    void setMaxQueueSize(int max_size);

   signals:
    // Emitted when recording starts.
    void recordingStarted();

    // Emitted when recording stops.
    void recordingStopped(bool success, const std::string& error);

    // Emitted when the queue is getting full (>= 80% capacity).
    void queueAlmostFull(int currentSize, int maxSize);

   protected:
    void run() override;

   private:
    void processFrames();
    void clearQueue();

    std::unique_ptr<VideoRecorder> recorder_;
    VideoRecorderOptions options_;

    std::queue<QImage> frameQueue_;
    mutable QMutex queueMutex_;
    QWaitCondition queueCondition_;

    std::atomic<bool> recordingActive_{false};
    std::atomic<bool> shouldStop_{false};
    std::atomic<int> maxQueueSize_{60};

    std::string lastError_;
};

}  // namespace octo_flex

#endif  // RECORDING_THREAD_H
