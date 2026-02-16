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


#ifndef OCTO_FLEX_VIEW_CONTAINER_H
#define OCTO_FLEX_VIEW_CONTAINER_H


#include <QAction>
#include <QElapsedTimer>
#include <QLabel>
#include <QMenu>
#include <QSplitter>
#include <QTimer>
#include <QWidget>
#include <map>
#include <memory>
#include <vector>
#include "recording_options.h"
#include "octo_flex_view.h"

namespace octo_flex {

class VideoRecorder;

class OctoFlexViewContainer : public QWidget {
    Q_OBJECT

   public:
    using Ptr = std::shared_ptr<OctoFlexViewContainer>;

    explicit OctoFlexViewContainer(QWidget* parent = nullptr);
    ~OctoFlexViewContainer() override;

    // Get the currently selected view.
    OctoFlexView* getCurrentView() const;

    // Get all views.
    std::vector<OctoFlexView*> getAllViews() const;

    // Set the object manager (applies to all views).
    void setObjectManager(ObjectManager::Ptr obj_mgr);

    // Create the initial view.
    OctoFlexView* createInitialView();

    // Container-level recording controls.
    bool startRecording(const RecordingOptions& options);
    bool pauseRecording();
    bool resumeRecording();
    bool stopRecording();
    bool isRecording() const;
    bool isRecordingPaused() const;
    std::string getLastRecordingError() const;

   public slots:
    // Split the current view vertically (top/bottom).
    void splitVertical();

    // Split the current view horizontally (left/right).
    void splitHorizontal();

    // Remove the current view.
    void removeCurrentView();

    // Expand the current view.
    void expandCurrentView();

    // Collapse the current view.
    void collapseCurrentView();

   protected:
    // Extend the view context menu (add split and remove actions).
    void extendViewContextMenu(OctoFlexView* view);

    // Event filter (tracks the active view).
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

   private:
    // Create a new view.
    OctoFlexView* createView();

    // Common split helper.
    OctoFlexView* splitView(OctoFlexView* view, Qt::Orientation orientation);

    // Remove a specific view.
    void removeView(OctoFlexView* view);

    // Recursively find the QSplitter that owns the view.
    QSplitter* findParentSplitter(OctoFlexView* view) const;

    // Save original view sizes.
    void saveViewSizes();

    // Restore original view sizes.
    void restoreViewSizes();

    // Clean up empty splitters.
    void cleanupEmptySplitters(QWidget* widget);

    // Update the "only view" state.
    void updateViewsOnlyStatus();

    // Capture one container frame for recording.
    void captureRecordingFrame();

    // Update overlay text for recording status.
    void updateRecordingStatusLabel();

    // Current recorded time in milliseconds.
    qint64 currentRecordedMs() const;

   private:
    QSplitter* mainSplitter_;           // Main splitter.
    OctoFlexView* currentView_;         // Currently active view.
    OctoFlexView* expandedView_;        // Currently expanded view.
    ObjectManager::Ptr objectManager_;  // Object manager.
    std::vector<OctoFlexView*> views_;  // List of all views.

    // Saved splitter sizes before expand.
    std::map<QSplitter*, QList<int>> savedSplitterSizes_;

    // View ID counter for assigning unique IDs.
    int viewIdCounter_ = 1;

    // Recording state.
    RecordingOptions recordingOptions_;
    std::unique_ptr<VideoRecorder> recorder_;
    QTimer* recordingTimer_ = nullptr;
    QTimer* recordingStatusTimer_ = nullptr;
    QLabel* recordingStatusLabel_ = nullptr;
    bool isRecording_ = false;
    bool isRecordingPaused_ = false;
    qint64 recordedElapsedMs_ = 0;
    QElapsedTimer recordingSegmentTimer_;
    int recordingWidth_ = 0;
    int recordingHeight_ = 0;
    std::string lastRecordingError_;
};

}  // namespace octo_flex

#endif  // OCTO_FLEX_VIEW_CONTAINER_H
