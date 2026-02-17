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


#include "octo_flex_view_container.h"
#include <algorithm>
#include <QBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QFocusEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QSplitter>
#include <QStandardPaths>
#include <QTimer>
#include <iostream>
#include "video_recorder.h"
#include "recording_thread.h"

namespace octo_flex {

OctoFlexViewContainer::OctoFlexViewContainer(QWidget* parent)
    : QWidget(parent), currentView_(nullptr), expandedView_(nullptr), objectManager_(nullptr) {
    // Create main layout.
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Create main splitter.
    mainSplitter_ = new QSplitter(this);
    mainSplitter_->setHandleWidth(1);
    layout->addWidget(mainSplitter_);

    // Set layout.
    setLayout(layout);

    // Frame capture timer for recording.
    recordingTimer_ = new QTimer(this);
    connect(recordingTimer_, &QTimer::timeout, this, &OctoFlexViewContainer::captureRecordingFrame);

    // Status timer for updating elapsed recording time.
    recordingStatusTimer_ = new QTimer(this);
    recordingStatusTimer_->setInterval(200);
    connect(recordingStatusTimer_, &QTimer::timeout, this, &OctoFlexViewContainer::updateRecordingStatusLabel);

    // Global overlay label shown while recording.
    recordingStatusLabel_ = new QLabel(this);
    recordingStatusLabel_->setStyleSheet(
        "QLabel { background-color: rgba(200, 32, 32, 180); color: white; border-radius: 4px; padding: 4px 8px; }");
    recordingStatusLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    recordingStatusLabel_->hide();
}

OctoFlexViewContainer::~OctoFlexViewContainer() {
    stopRecording();

    // Clear view list.
    views_.clear();
}

OctoFlexView* OctoFlexViewContainer::getCurrentView() const { return currentView_; }

std::vector<OctoFlexView*> OctoFlexViewContainer::getAllViews() const {
    // Return views_ directly, including views not yet added to splitters.
    return views_;
}

void OctoFlexViewContainer::setObjectManager(ObjectManager::Ptr obj_mgr) {
    objectManager_ = obj_mgr;

    // Apply object manager to all views.
    for (auto* view : views_) {
        if (view) {
            view->setObjectManager(obj_mgr);
        }
    }
}

bool OctoFlexViewContainer::startRecording(const RecordingOptions& options) {
    if (isRecording_) {
        lastRecordingError_ = "Recording is already running";
        return false;
    }

    if (options.fps <= 0) {
        lastRecordingError_ = "Recording FPS must be greater than zero";
        return false;
    }

    if (options.output_path.empty()) {
        lastRecordingError_ = "Recording output path is empty";
        return false;
    }

    if (width() <= 0 || height() <= 0) {
        lastRecordingError_ = "Container size is invalid for recording";
        return false;
    }

    if (!currentView_) {
        lastRecordingError_ = "No active view to capture";
        return false;
    }

    // Capture frame directly from OpenGL framebuffer of the current view
    QImage frame = currentView_->captureFrame();
    if (frame.isNull() || frame.width() <= 0 || frame.height() <= 0) {
        lastRecordingError_ = "Failed to capture initial OpenGL framebuffer";
        return false;
    }

    // Convert to RGB888 format for video recording
    frame = frame.convertToFormat(QImage::Format_RGB888);

    recordingOptions_ = options;
    recordingWidth_ = frame.width();
    recordingHeight_ = frame.height();
    // yuv420p (used by default) requires even dimensions.
    if (recordingWidth_ % 2 != 0) {
        recordingWidth_ -= 1;
    }
    if (recordingHeight_ % 2 != 0) {
        recordingHeight_ -= 1;
    }
    if (recordingWidth_ <= 0 || recordingHeight_ <= 0) {
        lastRecordingError_ = "Invalid recording dimensions after normalization";
        return false;
    }
    if (frame.width() != recordingWidth_ || frame.height() != recordingHeight_) {
        frame = frame.scaled(recordingWidth_, recordingHeight_, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    recordedElapsedMs_ = 0;
    lastRecordingError_.clear();
    recordingQueueWarningShown_ = false;

    // Create and start the recording thread.
    recordingThread_ = std::make_unique<RecordingThread>();
    connect(recordingThread_.get(), &RecordingThread::queueAlmostFull,
            this, &OctoFlexViewContainer::onRecordingQueueAlmostFull);

    VideoRecorderOptions recorderOptions;
    recorderOptions.outputPath = options.output_path;
    recorderOptions.width = recordingWidth_;
    recorderOptions.height = recordingHeight_;
    recorderOptions.fps = options.fps;
    recorderOptions.codec = options.codec;
    recorderOptions.preset = options.preset;
    recorderOptions.crf = options.crf;
    recorderOptions.overwrite = options.overwrite;

    std::string error;
    if (!recordingThread_->startRecording(recorderOptions, &error)) {
        lastRecordingError_ = error;
        recordingThread_.reset();
        return false;
    }

    // Queue the first frame.
    if (!recordingThread_->queueFrame(frame)) {
        lastRecordingError_ = "Failed to queue first frame";
        recordingThread_->stopRecording(&error);
        recordingThread_.reset();
        return false;
    }

    const int intervalMs = std::max(1, 1000 / options.fps);
    recordingTimer_->start(intervalMs);
    recordingStatusTimer_->start();

    isRecording_ = true;
    isRecordingPaused_ = false;
    recordingSegmentTimer_.start();
    updateRecordingStatusLabel();
    recordingStatusLabel_->show();

    return true;
}

bool OctoFlexViewContainer::pauseRecording() {
    if (!isRecording_ || isRecordingPaused_) {
        return false;
    }

    recordedElapsedMs_ += recordingSegmentTimer_.elapsed();
    isRecordingPaused_ = true;
    recordingTimer_->stop();
    updateRecordingStatusLabel();
    return true;
}

bool OctoFlexViewContainer::resumeRecording() {
    if (!isRecording_ || !isRecordingPaused_) {
        return false;
    }

    isRecordingPaused_ = false;
    recordingSegmentTimer_.restart();
    recordingTimer_->start(recordingTimer_->interval());
    updateRecordingStatusLabel();
    return true;
}

bool OctoFlexViewContainer::stopRecording() {
    if (!isRecording_ && !recordingThread_) {
        return false;
    }

    if (isRecording_ && !isRecordingPaused_) {
        recordedElapsedMs_ += recordingSegmentTimer_.elapsed();
    }

    if (recordingTimer_) {
        recordingTimer_->stop();
    }
    if (recordingStatusTimer_) {
        recordingStatusTimer_->stop();
    }

    bool ok = true;
    if (recordingThread_) {
        std::string error;
        ok = recordingThread_->stopRecording(&error);
        if (!ok && lastRecordingError_.empty()) {
            lastRecordingError_ = error;
        }
        recordingThread_.reset();
    }

    isRecording_ = false;
    isRecordingPaused_ = false;
    recordedElapsedMs_ = 0;
    recordingWidth_ = 0;
    recordingHeight_ = 0;
    recordingQueueWarningShown_ = false;

    if (recordingStatusLabel_) {
        recordingStatusLabel_->hide();
    }

    return ok;
}

bool OctoFlexViewContainer::isRecording() const { return isRecording_; }

bool OctoFlexViewContainer::isRecordingPaused() const { return isRecordingPaused_; }

std::string OctoFlexViewContainer::getLastRecordingError() const { return lastRecordingError_; }

OctoFlexView* OctoFlexViewContainer::createInitialView() {
    // If views already exist, skip creation.
    if (!views_.empty()) {
        return currentView_;
    }

    // Create initial view and add to the main splitter.
    OctoFlexView* view = createView();
    mainSplitter_->addWidget(view);

    // Set current view and return.
    currentView_ = view;

    // Ensure the initial view gains focus.
    view->setFocus();

    // Mark as the only view.
    view->setIsOnlyView(true);

    return view;
}

void OctoFlexViewContainer::splitVertical() {
    // Split current view vertically (top/bottom).
    if (currentView_) {
        splitView(currentView_, Qt::Vertical);
    }
}

void OctoFlexViewContainer::splitHorizontal() {
    // Split current view horizontally (left/right).
    if (currentView_) {
        splitView(currentView_, Qt::Horizontal);
    }
}

void OctoFlexViewContainer::removeCurrentView() {
    if (currentView_) {
        removeView(currentView_);
    }
}

void OctoFlexViewContainer::extendViewContextMenu(OctoFlexView* view) {
    // Add event filter to monitor context menu events.
    view->installEventFilter(this);
}

bool OctoFlexViewContainer::eventFilter(QObject* watched, QEvent* event) {
    if (auto* view = qobject_cast<OctoFlexView*>(watched)) {
        // Focus event: update current view.
        if (event->type() == QEvent::FocusIn) {
            currentView_ = view;
        }
        // Context menu event: add view actions.
        else if (event->type() == QEvent::ContextMenu) {
            // Add our actions after OctoFlexView creates its menu.
            // Use a queued connection to ensure the original menu exists.
            QMetaObject::Connection connection;
            connection = connect(view, &QObject::destroyed, [&connection]() { QObject::disconnect(connection); });

            // Delay to ensure the original menu is created.
            QTimer::singleShot(0, this, [this, view, connection]() {
                if (!view) return;

                QObject::disconnect(connection);

                // Find the view's context menu.
                QMenu* menu = view->findChild<QMenu*>();
                if (menu) {
                    // Add recording actions group (container-wide).
                    QMenu* recordingMenu = menu->addMenu("Recording");

                    QAction* startRecordingAction = new QAction("Start Recording...", recordingMenu);
                    startRecordingAction->setEnabled(!isRecording_);
                    connect(startRecordingAction, &QAction::triggered, this, [this]() {
                        if (QStandardPaths::findExecutable("ffmpeg").isEmpty()) {
                            QMessageBox::critical(this, "Recording Error",
                                                  "Recording could not start because ffmpeg was not found in PATH.\n"
                                                  "Please install ffmpeg and try again.");
                            return;
                        }

                        RecordingOptions options;
                        options.fps = 30;
                        options.codec = "libx264";
                        options.preset = "veryfast";
                        options.crf = 23;
                        options.overwrite = true;

                        const QString defaultName =
                            QString("octo_flex_recording_%1.mp4").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
                        const QString selected = QFileDialog::getSaveFileName(
                            this, "Save Recording", defaultName, "MP4 Video (*.mp4)");
                        if (selected.isEmpty()) {
                            return;
                        }
                        options.output_path = selected.toStdString();
                        if (!startRecording(options)) {
                            QMessageBox::critical(this, "Recording Error",
                                                  QString::fromStdString("Start recording failed: " +
                                                                         getLastRecordingError()));
                        }
                    });
                    recordingMenu->addAction(startRecordingAction);

                    QAction* pauseRecordingAction = new QAction("Pause Recording", recordingMenu);
                    pauseRecordingAction->setEnabled(isRecording_ && !isRecordingPaused_);
                    connect(pauseRecordingAction, &QAction::triggered, this, [this]() { pauseRecording(); });
                    recordingMenu->addAction(pauseRecordingAction);

                    QAction* resumeRecordingAction = new QAction("Resume Recording", recordingMenu);
                    resumeRecordingAction->setEnabled(isRecording_ && isRecordingPaused_);
                    connect(resumeRecordingAction, &QAction::triggered, this, [this]() { resumeRecording(); });
                    recordingMenu->addAction(resumeRecordingAction);

                    QAction* stopRecordingAction = new QAction("Stop Recording", recordingMenu);
                    stopRecordingAction->setEnabled(isRecording_);
                    connect(stopRecordingAction, &QAction::triggered, this, [this]() {
                        if (!stopRecording()) {
                            QMessageBox::critical(
                                this, "Recording Error",
                                QString::fromStdString("Stop recording failed: " + getLastRecordingError()));
                        }
                    });
                    recordingMenu->addAction(stopRecordingAction);

                    // Add separator.
                    menu->addSeparator();

                    // Add view actions group.
                    QMenu* viewMenu = menu->addMenu("View Actions");

                    // Add vertical split action.
                    QAction* splitVerticalAction = new QAction("Split Vertical", viewMenu);
                    connect(splitVerticalAction, &QAction::triggered, this, &OctoFlexViewContainer::splitVertical);
                    viewMenu->addAction(splitVerticalAction);

                    // Add horizontal split action.
                    QAction* splitHorizontalAction = new QAction("Split Horizontal", viewMenu);
                    connect(splitHorizontalAction, &QAction::triggered, this, &OctoFlexViewContainer::splitHorizontal);
                    viewMenu->addAction(splitHorizontalAction);

                    // Only add remove action when multiple views exist.
                    if (views_.size() > 1) {
                        viewMenu->addSeparator();
                        QAction* removeAction = new QAction("Remove View", viewMenu);
                        connect(removeAction, &QAction::triggered, this, &OctoFlexViewContainer::removeCurrentView);
                        viewMenu->addAction(removeAction);
                    }
                }
            });
        }
    }
    return QWidget::eventFilter(watched, event);
}

void OctoFlexViewContainer::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (!recordingStatusLabel_) {
        return;
    }

    recordingStatusLabel_->adjustSize();
    const int margin = 10;
    recordingStatusLabel_->move(width() - recordingStatusLabel_->width() - margin, margin);
}

OctoFlexView* OctoFlexViewContainer::createView() {
    // Create new view.
    OctoFlexView* view = new OctoFlexView(this);

    // Assign a unique view ID.
    std::string viewId = "View " + std::to_string(viewIdCounter_++);
    view->setViewId(viewId);

    // Set object manager for the view.
    if (objectManager_) {
        view->setObjectManager(objectManager_);
    }

    // Initialize view and extend its context menu.
    view->initialize();
    extendViewContextMenu(view);

    // Connect view split/remove signals.
    connect(view, &OctoFlexView::requestHorizontalSplit, this, &OctoFlexViewContainer::splitHorizontal);
    connect(view, &OctoFlexView::requestVerticalSplit, this, &OctoFlexViewContainer::splitVertical);
    connect(view, &OctoFlexView::requestViewRemove, this, &OctoFlexViewContainer::removeCurrentView);

    // Connect view expand/collapse signals.
    connect(view, &OctoFlexView::requestExpand, this, &OctoFlexViewContainer::expandCurrentView);
    connect(view, &OctoFlexView::requestCollapse, this, &OctoFlexViewContainer::collapseCurrentView);

    // Add to view list.
    views_.push_back(view);

    // Update "only view" state for all views.
    updateViewsOnlyStatus();

    return view;
}

OctoFlexView* OctoFlexViewContainer::splitView(OctoFlexView* view, Qt::Orientation orientation) {
    // Find the view's parent splitter.
    QSplitter* parentSplitter = findParentSplitter(view);
    if (!parentSplitter) {
        std::cerr << "Failed to find the view's parent splitter!" << std::endl;
        return nullptr;
    }

    // Get the view index in its parent splitter.
    int index = parentSplitter->indexOf(view);
    if (index == -1) {
        std::cerr << "View not found in parent splitter!" << std::endl;
        return nullptr;
    }

    // Save current view size and all splitter sizes.
    QList<int> originalSizes = parentSplitter->sizes();
    int currentSize = originalSizes[index];

    // Check parent splitter orientation.
    bool needNewSplitter = parentSplitter->orientation() != orientation;

    OctoFlexView* newView = nullptr;

    if (needNewSplitter) {
        // Need a new splitter.
        QSplitter* newSplitter = new QSplitter(orientation, parentSplitter);
        newSplitter->setHandleWidth(1);

        // Move view into the new splitter.
        parentSplitter->insertWidget(index, newSplitter);
        view->setParent(newSplitter);
        newSplitter->addWidget(view);

        // Create new view and add to the new splitter.
        newView = createView();
        newSplitter->addWidget(newView);

        // Split space evenly in the new splitter.
        newSplitter->setSizes(QList<int>() << currentSize / 2 << currentSize / 2);

        // Restore original sizes in parent splitter to keep other views unchanged.
        parentSplitter->setSizes(originalSizes);
    } else {
        // Same orientation; insert new view directly.
        newView = createView();
        parentSplitter->insertWidget(index + 1, newView);

        // Preserve original sizes.
        QList<int> newSizes = originalSizes;

        // Only adjust the split view; keep other sizes unchanged.
        newSizes[index] = currentSize / 2;            // Current view halves.
        newSizes.insert(index + 1, currentSize / 2);  // New view takes the other half.

        // Fix: remove last element if size list grew after insertion.
        if (newSizes.size() > parentSplitter->count()) {
            newSizes.removeLast();
        }

        // Apply new size settings.
        parentSplitter->setSizes(newSizes);
    }

    // Copy settings from the original view to the new view.
    if (newView) {
        // Copy hidden layers.
        newView->setUnvisableLayers(view->getUnvisableLayers());

        // Copy unselectable layers.
        newView->setUnselectableLayers(view->getUnselectableLayers());

        // Copy projection mode.
        newView->setPerspectiveMode(view->isPerspectiveMode());

        // Copy info panel visibility.
        if (view->isInfoPanelVisible() != newView->isInfoPanelVisible()) {
            newView->toggleInfoPanel();
        }

        if (view->isGridVisible() != newView->isGridVisible()) {
            newView->toggleGrid();
        }

        // Copy camera settings (use copyCamera).
        newView->copyCamera(view->getCamera());

        // Update current view.
        currentView_ = newView;

        // Ensure the new view gains focus.
        newView->setFocus();
    }

    return newView;
}

void OctoFlexViewContainer::removeView(OctoFlexView* view) {
    // Find in view list.
    auto it = std::find(views_.begin(), views_.end(), view);
    if (it == views_.end()) {
        std::cerr << "View to remove is not in the list!" << std::endl;
        return;
    }

    // Do not remove the last view.
    if (views_.size() <= 1) {
        std::cerr << "This is the last view; removal is not allowed!" << std::endl;
        return;
    }

    // Remove from view list.
    views_.erase(it);

    // Find the view's splitter.
    QSplitter* parentSplitter = findParentSplitter(view);
    if (!parentSplitter) {
        std::cerr << "Failed to find the view's parent splitter!" << std::endl;
        return;
    }

    // Remove view from parent widget.
    view->setParent(nullptr);
    view->deleteLater();

    // If current view is removed, select a new current view.
    if (view == currentView_) {
        if (!views_.empty()) {
            currentView_ = views_.front();
            currentView_->setFocus();
        } else {
            currentView_ = nullptr;
        }
    }

    // Clean up empty splitters.
    cleanupEmptySplitters(mainSplitter_);

    // Update "only view" state.
    updateViewsOnlyStatus();
}

QSplitter* OctoFlexViewContainer::findParentSplitter(OctoFlexView* view) const {
    // Find the view's parent splitter.
    QWidget* parent = view->parentWidget();
    while (parent) {
        if (auto* splitter = qobject_cast<QSplitter*>(parent)) {
            return splitter;
        }
        parent = parent->parentWidget();
    }
    return nullptr;
}

void OctoFlexViewContainer::expandCurrentView() {
    if (!currentView_ || expandedView_) {
        return;  // No current view or already expanded.
    }

    // Save current view sizes.
    saveViewSizes();

    // Walk up splitters from current view and set sizes.
    QWidget* current = currentView_;
    while (current && current != this) {
        QSplitter* parentSplitter = qobject_cast<QSplitter*>(current->parentWidget());
        if (parentSplitter) {
            // Get current widget index in parent splitter.
            int index = parentSplitter->indexOf(current);
            if (index != -1) {
                // Set parent splitter sizes, zero out others.
                QList<int> sizes = parentSplitter->sizes();
                for (int i = 0; i < sizes.size(); ++i) {
                    if (i != index) {
                        sizes[i] = 0;
                    }
                }
                parentSplitter->setSizes(sizes);
            }
        }
        // Move up to parent widget.
        current = current->parentWidget();
    }

    // Mark current view as expanded.
    expandedView_ = currentView_;

    // Notify view it is expanded.
    currentView_->expandView();

    // Update layout.
    layout()->update();

    // Ensure current view has focus.
    currentView_->setFocus();
}

void OctoFlexViewContainer::collapseCurrentView() {
    if (!expandedView_) {
        return;  // No expanded view.
    }

    // Show all views.
    for (auto* view : views_) {
        view->show();
    }

    // Restore original sizes.
    restoreViewSizes();

    // Notify view it is collapsed.
    expandedView_->collapseView();

    // Clear expanded view marker.
    expandedView_ = nullptr;

    // Update layout.
    layout()->update();

    // Ensure current view has focus.
    if (currentView_) {
        currentView_->setFocus();
    }
}

void OctoFlexViewContainer::saveViewSizes() {
    // Clear previously saved sizes.
    savedSplitterSizes_.clear();

    // Walk all splitters and save their sizes.
    std::function<void(QWidget*)> saveSizes = [this, &saveSizes](QWidget* widget) {
        // If widget is a splitter, save its sizes.
        if (auto* splitter = qobject_cast<QSplitter*>(widget)) {
            savedSplitterSizes_[splitter] = splitter->sizes();

            // Walk splitter children.
            for (int i = 0; i < splitter->count(); ++i) {
                saveSizes(splitter->widget(i));
            }
        }
    };

    // Start from main splitter.
    saveSizes(mainSplitter_);
}

void OctoFlexViewContainer::restoreViewSizes() {
    // Restore sizes for all saved splitters.
    for (const auto& pair : savedSplitterSizes_) {
        if (pair.first) {  // Ensure splitter is still valid.
            pair.first->setSizes(pair.second);
        }
    }

    // Clear saved sizes.
    savedSplitterSizes_.clear();
}

void OctoFlexViewContainer::updateViewsOnlyStatus() {
    // Use views_ list to include views not added to splitters.
    bool isOnlyOneView = (views_.size() == 1);

    for (auto* view : views_) {
        view->setIsOnlyView(isOnlyOneView);
    }
}

void OctoFlexViewContainer::cleanupEmptySplitters(QWidget* widget) {
    // If widget is a splitter, check and clean.
    QSplitter* splitter = qobject_cast<QSplitter*>(widget);
    if (!splitter) {
        return;
    }

    // Recursively clean child widgets first.
    for (int i = splitter->count() - 1; i >= 0; --i) {
        QWidget* childWidget = splitter->widget(i);
        cleanupEmptySplitters(childWidget);
    }

    // Remove empty splitters (except main).
    if (splitter->count() == 0 && splitter != mainSplitter_) {
        splitter->setParent(nullptr);
        splitter->deleteLater();
        return;
    }

    // If splitter has one child and isn't main, promote the child to the parent splitter.
    if (splitter->count() == 1 && splitter != mainSplitter_) {
        QWidget* childWidget = splitter->widget(0);
        QSplitter* parentSplitter = qobject_cast<QSplitter*>(splitter->parent());

        if (parentSplitter) {
            int index = parentSplitter->indexOf(splitter);
            childWidget->setParent(nullptr);
            splitter->setParent(nullptr);
            parentSplitter->insertWidget(index, childWidget);
            splitter->deleteLater();
        }
    }
}

void OctoFlexViewContainer::captureRecordingFrame() {
    if (!isRecording_ || !currentView_) {
        return;
    }

    // Capture frame directly from OpenGL framebuffer of the current view
    QImage frame = currentView_->captureFrame();
    if (frame.isNull()) {
        lastRecordingError_ = "Failed to capture OpenGL framebuffer";
        stopRecording();
        return;
    }

    // Convert to RGB888 format for video recording
    frame = frame.convertToFormat(QImage::Format_RGB888);

    // Resize frame if needed
    if (frame.width() != recordingWidth_ || frame.height() != recordingHeight_) {
        frame = frame.scaled(recordingWidth_, recordingHeight_, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Queue frame for async recording (non-blocking)
    if (recordingThread_ && !recordingThread_->queueFrame(frame)) {
        lastRecordingError_ = "Failed to queue frame - recording queue is full";
        stopRecording();
    }
}

void OctoFlexViewContainer::updateRecordingStatusLabel() {
    if (!recordingStatusLabel_ || !isRecording_) {
        return;
    }

    const qint64 totalMs = currentRecordedMs();
    const int totalSec = static_cast<int>(totalMs / 1000);
    const int mm = totalSec / 60;
    const int ss = totalSec % 60;

    const QString prefix = isRecordingPaused_ ? "REC (PAUSED)" : "REC";
    recordingStatusLabel_->setText(QString("%1  %2:%3")
                                       .arg(prefix)
                                       .arg(mm, 2, 10, QChar('0'))
                                       .arg(ss, 2, 10, QChar('0')));
    recordingStatusLabel_->adjustSize();
    const int margin = 10;
    recordingStatusLabel_->move(width() - recordingStatusLabel_->width() - margin, margin);
}

void OctoFlexViewContainer::onRecordingQueueAlmostFull(int currentSize, int maxSize) {
    if (recordingQueueWarningShown_) {
        return;
    }
    recordingQueueWarningShown_ = true;

    // Show a warning message to the user
    QMessageBox::warning(
        this, "Recording Queue Almost Full",
        QString("The recording queue is nearly full (%1/%2 frames). "
                "This may indicate that the system is too slow to encode frames in real-time. "
                "Consider reducing the recording resolution or FPS.")
            .arg(currentSize)
            .arg(maxSize));
}

qint64 OctoFlexViewContainer::currentRecordedMs() const {
    if (!isRecording_) {
        return 0;
    }

    if (isRecordingPaused_) {
        return recordedElapsedMs_;
    }

    return recordedElapsedMs_ + recordingSegmentTimer_.elapsed();
}

}  // namespace octo_flex
