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

#include <cstring>
#include <sstream>

namespace octo_flex {

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

    // Build ffmpeg command line
    std::ostringstream cmd;
    cmd << "ffmpeg"
        << " -hide_banner -loglevel error"
        << (options.overwrite ? " -y" : " -n")
        << " -f rawvideo"
        << " -pix_fmt " << (options.enableAlpha ? "rgba" : "rgb24")
        << " -s " << options.width << "x" << options.height
        << " -r " << options.fps
        << " -i -"
        << " -an"
        << " -c:v " << options.codec
        << " -preset " << options.preset
        << " -crf " << options.crf
        << " -pix_fmt " << (options.enableAlpha ? "yuva420p" : "yuv420p")
	<< " -movflags frag_keyframe+empty_moov"
        << " " << options.outputPath;

    pipe_ = popen(cmd.str().c_str(), "w");
    if (!pipe_) {
        setError("Failed to start ffmpeg process via popen", error);
        return false;
    }

    started_ = true;
    return true;
}

bool VideoRecorder::writeFrame(const QImage& frame, std::string* error) {
    if (!started_ || !pipe_) {
        setError("Recorder is not running", error);
        return false;
    }

    QImage processed = frame;
    if (options_.enableAlpha) {
        if (processed.format() != QImage::Format_RGBA8888) {
            processed = processed.convertToFormat(QImage::Format_RGBA8888);
        }
    } else {
        if (processed.format() != QImage::Format_RGB888) {
            processed = processed.convertToFormat(QImage::Format_RGB888);
        }
    }

    if (processed.width() != options_.width || processed.height() != options_.height) {
        processed = processed.scaled(options_.width, options_.height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const int rowBytes = options_.width * (options_.enableAlpha ? 4 : 3);

    for (int y = 0; y < options_.height; ++y) {
        const unsigned char* src = processed.constScanLine(y);
        size_t written = fwrite(src, 1, rowBytes, pipe_);
        if (static_cast<int>(written) != rowBytes) {
            setError("Failed writing frame data to ffmpeg pipe", error);
            return false;
        }
    }

    return true;
}

bool VideoRecorder::stop(std::string* error) {
    if (!pipe_) {
        started_ = false;
        return true;
    }

    int status = pclose(pipe_);
    pipe_ = nullptr;
    started_ = false;

    if (status != 0) {
        setError("ffmpeg exited with status " + std::to_string(status), error);
        return false;
    }

    return true;
}

bool VideoRecorder::isRunning() const { return started_ && pipe_ != nullptr; }

void VideoRecorder::setError(const std::string& message, std::string* error) {
    if (error) {
        *error = message;
    }
}

}  // namespace octo_flex
