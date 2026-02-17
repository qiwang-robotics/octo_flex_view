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

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include <QImage>
#include <QProcess>
#include <memory>
#include <string>

namespace octo_flex {

struct VideoRecorderOptions {
    std::string outputPath;
    int width = 0;
    int height = 0;
    int fps = 30;
    std::string codec = "libx264";
    std::string preset = "veryfast";
    int crf = 23;
    bool overwrite = true;
    bool enableAlpha = false;  // If true, uses rgba format to preserve transparency
};

class VideoRecorder {
   public:
    VideoRecorder();
    ~VideoRecorder();

    bool start(const VideoRecorderOptions& options, std::string* error = nullptr);
    bool writeFrame(const QImage& frame, std::string* error = nullptr);
    bool stop(std::string* error = nullptr);

    bool isRunning() const;

   private:
    bool writeAll(const char* data, qint64 size, std::string* error);
    void setError(const std::string& message, std::string* error);

   private:
    std::unique_ptr<QProcess> process_;
    VideoRecorderOptions options_;
    bool started_ = false;
};

}  // namespace octo_flex

#endif  // VIDEO_RECORDER_H
