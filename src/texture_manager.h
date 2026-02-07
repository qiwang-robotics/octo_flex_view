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


#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H


#include <QMutex>
#include <QQueue>
#include <QSet>
#include <QThread>
#include <QWaitCondition>
#include <atomic>
#include <memory>

#include "textured_quad.h"

#include <QSurfaceFormat>

class QOpenGLContext;
class QOffscreenSurface;

namespace octo_flex {

class TextureManager : public QThread {
    Q_OBJECT

   public:
    static TextureManager* instance();

    // Initialize with the main OpenGL context to create a shared context
    void initialize(QOpenGLContext* mainContext);
    void cleanup();

    // Add texture for upload (thread-safe, can be called from any thread)
    void queueTextureForUpload(const std::shared_ptr<Shape>& shape);

    // Check if texture is ready
    bool isTextureReady(GLuint textureId) const;

    // Cancel upload for a texture
    void cancelTextureUpload(const std::shared_ptr<Shape>& shape);

    // Get upload queue size (for monitoring)
    int getQueueSize() const;

   protected:
    void run() override;

   private:
    TextureManager(QObject* parent = nullptr);
    ~TextureManager();

    void createSharedContextInThread();

    static TextureManager* instance_;

    QMutex mutex_;
    QWaitCondition condition_;
    QQueue<std::weak_ptr<Shape>> uploadQueue_;
    QSet<GLuint> readyTextures_;
    std::atomic<bool> shouldQuit_;
    std::atomic<bool> initialized_;

    // Main context info (used to create shared context in background thread)
    QOpenGLContext* mainContext_;
    QSurfaceFormat contextFormat_;

    // OpenGL context for background thread (created in the thread itself)
    QOpenGLContext* sharedContext_;
    QOffscreenSurface* offscreenSurface_;
};

}  // namespace octo_flex

#endif  // TEXTURE_MANAGER_H
