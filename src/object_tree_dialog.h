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


#ifndef OBJECT_TREE_DIALOG_H
#define OBJECT_TREE_DIALOG_H


#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>
#include <set>
#include "object_manager.h"

namespace octo_flex {

// Dialog mode enum.
enum class ObjectTreeMode {
    ALL,        // All selectable items (layers and objects).
    LAYER_ONLY  // Layers only.
};

class ObjectTreeDialog : public QDialog {
    Q_OBJECT

   public:
    explicit ObjectTreeDialog(QWidget* parent = nullptr);
    ~ObjectTreeDialog() override;

    // Set the object manager.
    void setObjectManager(ObjectManager::Ptr objManager);

    // Set dialog mode.
    void setMode(ObjectTreeMode mode);

    // Get selected object IDs.
    std::set<std::string> getSelectedObjects() const;

    // Set preselected items.
    void setPreselectedItems(const std::set<std::string>& items);

   private slots:
    // Handle item state changes.
    void onItemChanged(QStandardItem* item);

    // OK button click.
    void onOkClicked();

    // Cancel button click.
    void onCancelClicked();

   private:
    // Initialize UI.
    void initUI();

    // Build object tree.
    void buildObjectTree();

    // Recursively build tree nodes.
    QStandardItem* buildTreeNode(const std::string& path, const std::map<std::string, std::vector<std::string>>& tree);

    // Recursively set child check state.
    void setChildCheckState(QStandardItem* item, Qt::CheckState state);

    // Update parent check state.
    void updateParentCheckState(QStandardItem* item);

    // Parse object ID to a path.
    std::vector<std::string> parseObjectId(const std::string& objectId);

    // Collect selected objects.
    void collectSelectedObjects(QStandardItem* item);

    // Check whether a path is a directory node.
    bool isDirectory(const std::string& path) const;

    // Update item selection state.
    void updateItemSelectionState(QStandardItem* item);

   private:
    QTreeView* treeView_;
    QStandardItemModel* model_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    ObjectManager::Ptr objManager_;
    std::map<std::string, std::string> itemToObjectId_;  // Item to object ID mapping.
    std::set<std::string> selectedObjects_;              // Selected object ID set.
    std::set<std::string> preselectedItems_;             // Preselected item set.
    std::set<std::string> directoryPaths_;               // Directory path set.
    ObjectTreeMode mode_ = ObjectTreeMode::ALL;          // Dialog mode.
};

}  // namespace octo_flex

#endif  // OBJECT_TREE_DIALOG_H
