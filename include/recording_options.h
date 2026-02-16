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

#ifndef RECORDING_OPTIONS_H
#define RECORDING_OPTIONS_H

#include <string>

namespace octo_flex {

/**
 * @brief Video recording options for container-level capture.
 */
struct RecordingOptions {
    std::string output_path = "octo_flex_recording.mp4";
    int fps = 30;
    std::string codec = "libx264";
    std::string preset = "veryfast";
    int crf = 23;
    bool overwrite = true;
};

}  // namespace octo_flex

#endif  // RECORDING_OPTIONS_H
