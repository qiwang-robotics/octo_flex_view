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

#include "recording_thread.h"

#include <QMutexLocker>

namespace octo_flex {

RecordingThread::RecordingThread(QObject* parent) : QThread(parent) {}

RecordingThread::~RecordingThread() {
    std::string ignored;
    stopRecording(&ignored);
    wait(3000);  // Wait up to 3 seconds for thread to finish
}

bool RecordingThread::startRecording(const VideoRecorderOptions& options, std::string* error) {
    if (recordingActive_) {
        if (error) *error = "Recording already active";
        return false;
    }

    options_ = options;
    
    // Reset state
    shouldStop_ = false;
    recordingActive_ = true;
    lastError_.clear();
    clearQueue();

    // Start the worker thread - ffmpeg will be started in run()
    start(QThread::HighPriority);
    
    // Wait for recorder to be initialized in the worker thread
    int timeout = 3000;  // 3 seconds
    while (!recorder_ && timeout > 0) {
        msleep(10);
        timeout -= 10;
    }
    
    if (!recorder_) {
        if (error) *error = "Failed to initialize recorder in worker thread";
        recordingActive_ = false;
        return false;
    }

    return true;
}

bool RecordingThread::queueFrame(const QImage& frame) {
    if (!recordingActive_ || shouldStop_) {
        return false;
    }

    QMutexLocker locker(&queueMutex_);

    // Check queue capacity
    if (static_cast<int>(frameQueue_.size()) >= maxQueueSize_) {
        // Queue is full, emit warning signal
        locker.unlock();
        emit queueAlmostFull(maxQueueSize_, maxQueueSize_);
        return false;
    }

    // Copy frame and add to queue
    frameQueue_.push(frame.copy());

    // Check if queue is getting full (>= 80%)
    int currentSize = static_cast<int>(frameQueue_.size());
    if (currentSize >= maxQueueSize_ * 0.8) {
        locker.unlock();
        emit queueAlmostFull(currentSize, maxQueueSize_);
    } else {
        locker.unlock();
    }

    // Notify worker thread
    queueCondition_.wakeOne();

    return true;
}

bool RecordingThread::stopRecording(std::string* error) {
    if (!recordingActive_) {
        if (error) *error = lastError_;
        return lastError_.empty();
    }

    // Signal thread to stop
    shouldStop_ = true;
    queueCondition_.wakeOne();

    // Wait for thread to finish processing
    if (!wait(30000)) {  // 30 second timeout
        terminate();
        wait(1000);
        if (error) *error = "Recording thread did not stop cleanly (timeout)";
        recordingActive_ = false;
        recorder_.reset();
        return false;
    }

    // Recorder is stopped in processFrames() after the loop exits
    // Just clean up here
    recordingActive_ = false;
    recorder_.reset();

    if (!lastError_.empty() && error) {
        *error = lastError_;
    }

    return lastError_.empty();
}

bool RecordingThread::isRecording() const {
    return recordingActive_;
}

void RecordingThread::run() {
    // Create and start ffmpeg in this worker thread
    recorder_ = std::make_unique<VideoRecorder>();
    
    std::string startError;
    if (!recorder_->start(options_, &startError)) {
        lastError_ = "Failed to start ffmpeg: " + startError;
        recordingActive_ = false;
        emit recordingStopped(false, lastError_);
        return;
    }
    
    // Process frames
    processFrames();
}

void RecordingThread::processFrames() {
    int frameCount = 0;
    
    while (!shouldStop_ || !frameQueue_.empty()) {
        QImage frame;

        {
            QMutexLocker locker(&queueMutex_);

            // Wait for a frame or stop signal
            while (frameQueue_.empty() && !shouldStop_ && recordingActive_) {
                queueCondition_.wait(&queueMutex_, 100);  // 100ms timeout
            }

            if (frameQueue_.empty()) {
                if (shouldStop_ || !recordingActive_) {
                    break;
                }
                continue;
            }

            frame = frameQueue_.front();
            frameQueue_.pop();
        }

        // Write frame to recorder
        if (!frame.isNull() && recorder_) {
            std::string error;
            if (!recorder_->writeFrame(frame, &error)) {
                lastError_ = "Failed to write frame: " + error;
                break;
            }
            frameCount++;
        }
    }
    
    // Stop ffmpeg and wait for it to finish
    if (recorder_) {
        std::string stopError;
        if (!recorder_->stop(&stopError)) {
            if (lastError_.empty()) {
                lastError_ = stopError;
            }
        }
    }

    // Clear any remaining frames
    clearQueue();

    emit recordingStopped(lastError_.empty(), lastError_);
}

void RecordingThread::clearQueue() {
    QMutexLocker locker(&queueMutex_);
    while (!frameQueue_.empty()) {
        frameQueue_.pop();
    }
}

}  // namespace octo_flex
