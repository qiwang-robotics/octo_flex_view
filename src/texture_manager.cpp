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


#include "texture_manager.h"
#include <QDebug>
#include <QMutexLocker>
#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace octo_flex {

TextureManager* TextureManager::instance_ = nullptr;

TextureManager::TextureManager(QObject* parent)
    : QThread(parent),
      shouldQuit_(false),
      initialized_(false),
      mainContext_(nullptr),
      sharedContext_(nullptr),
      offscreenSurface_(nullptr) {}

TextureManager::~TextureManager() {
    if (isRunning()) {
        cleanup();
    }

    // Clean up OpenGL resources
    if (sharedContext_) {
        delete sharedContext_;
        sharedContext_ = nullptr;
    }
    if (offscreenSurface_) {
        offscreenSurface_->destroy();
        delete offscreenSurface_;
        offscreenSurface_ = nullptr;
    }
}

TextureManager* TextureManager::instance() {
    if (!instance_) {
        instance_ = new TextureManager();
    }
    return instance_;
}

void TextureManager::initialize(QOpenGLContext* mainContext) {
    if (!mainContext) {
        qWarning() << "TextureManager::initialize: mainContext is null!";
        return;
    }

    if (initialized_) {
        qWarning() << "TextureManager::initialize: Already initialized!";
        return;
    }

    qDebug() << "TextureManager::initialize: Saving main context information...";

    // Save the main context and its format (don't create shared context here!)
    mainContext_ = mainContext;
    contextFormat_ = mainContext->format();

    // Start the background thread (shared context will be created there)
    if (!isRunning()) {
        start();
    }

    initialized_ = true;
    qDebug() << "TextureManager::initialize: Background thread started";
}

void TextureManager::createSharedContextInThread() {
    // This function MUST be called in the background thread

    qDebug() << "TextureManager: Creating shared OpenGL context in background thread...";

    // Create offscreen surface for the background thread
    offscreenSurface_ = new QOffscreenSurface();
    offscreenSurface_->setFormat(contextFormat_);
    offscreenSurface_->create();

    if (!offscreenSurface_->isValid()) {
        qWarning() << "TextureManager: Failed to create offscreen surface!";
        delete offscreenSurface_;
        offscreenSurface_ = nullptr;
        return;
    }

    // Create shared OpenGL context IN THIS THREAD
    sharedContext_ = new QOpenGLContext();
    sharedContext_->setFormat(contextFormat_);
    sharedContext_->setShareContext(mainContext_);  // Share resources with main context

    if (!sharedContext_->create()) {
        qWarning() << "TextureManager: Failed to create shared OpenGL context!";
        delete sharedContext_;
        sharedContext_ = nullptr;
        offscreenSurface_->destroy();
        delete offscreenSurface_;
        offscreenSurface_ = nullptr;
        return;
    }

    qDebug() << "TextureManager: Shared OpenGL context created successfully in background thread";
}

void TextureManager::cleanup() {
    shouldQuit_ = true;

    {
        QMutexLocker locker(&mutex_);
        condition_.wakeAll();
    }

    if (isRunning()) {
        wait();
    }

    // Clear the queue
    QMutexLocker locker(&mutex_);
    uploadQueue_.clear();
}

void TextureManager::queueTextureForUpload(const std::shared_ptr<Shape>& shape) {
    auto texturedQuad = std::dynamic_pointer_cast<TexturedQuad>(shape);
    if (!texturedQuad || !texturedQuad->hasImageData()) {
        return;  // Nothing to upload
    }

    QMutexLocker locker(&mutex_);
    uploadQueue_.enqueue(texturedQuad);
    condition_.wakeAll();
}

bool TextureManager::isTextureReady(GLuint textureId) const {
    QMutexLocker locker(const_cast<QMutex*>(&mutex_));
    return readyTextures_.contains(textureId);
}

void TextureManager::cancelTextureUpload(const std::shared_ptr<Shape>& shape) {
    auto texturedQuad = std::dynamic_pointer_cast<TexturedQuad>(shape);
    if (!texturedQuad) return;

    QMutexLocker locker(&mutex_);
    // Remove from queue if it's there
    auto it = uploadQueue_.begin();
    while (it != uploadQueue_.end()) {
        auto quad = it->lock();
        if (quad && quad.get() == texturedQuad.get()) {
            it = uploadQueue_.erase(it);
        } else {
            ++it;
        }
    }
}

int TextureManager::getQueueSize() const {
    QMutexLocker locker(const_cast<QMutex*>(&mutex_));
    return uploadQueue_.size();
}

void TextureManager::run() {
    qDebug() << "TextureManager::run: Background thread started";

    // Create the shared OpenGL context IN THIS THREAD
    createSharedContextInThread();

    // Check if context creation succeeded
    if (!sharedContext_ || !offscreenSurface_) {
        qWarning() << "TextureManager::run: Failed to create shared context, thread exiting!";
        return;
    }

    // Make the shared OpenGL context current in this thread
    if (!sharedContext_->makeCurrent(offscreenSurface_)) {
        qWarning() << "TextureManager::run: Failed to make shared context current!";
        return;
    }

    qDebug() << "TextureManager::run: OpenGL context is current, ready to upload textures";

    while (!shouldQuit_) {
        std::shared_ptr<TexturedQuad> texturedQuad;

        {
            QMutexLocker locker(&mutex_);

            // Wait for work or timeout
            if (uploadQueue_.isEmpty() && !shouldQuit_) {
                condition_.wait(&mutex_, 100);  // Wait for 100ms
                continue;
            }

            if (shouldQuit_) {
                break;
            }

            if (!uploadQueue_.isEmpty()) {
                auto shape = uploadQueue_.dequeue().lock();
                if (shape) {
                    texturedQuad = std::dynamic_pointer_cast<TexturedQuad>(shape);
                }
            }
        }

        // Process the texture upload outside the mutex lock
        if (texturedQuad && texturedQuad->hasImageData()) {
            // Upload the texture using the shared context
            GLuint textureId = texturedQuad->ensureTextureUploaded();

            if (textureId != 0) {
                // Mark as ready
                QMutexLocker locker(&mutex_);
                readyTextures_.insert(textureId);
                qDebug() << "TextureManager: Uploaded texture ID" << textureId
                         << "- Queue size:" << uploadQueue_.size();
            }
        }
    }

    // Release the context before thread exits
    sharedContext_->doneCurrent();
    qDebug() << "TextureManager::run: Background upload thread stopped";
}

}  // namespace octo_flex
