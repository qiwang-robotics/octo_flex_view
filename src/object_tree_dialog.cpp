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

#include "object_tree_dialog.h"
#include <QDebug>
#include <QHeaderView>
#include <algorithm>
#include <iostream>

namespace octo_flex {
namespace {
const std::string kObjectPrefix = "__object__:";

bool IsObjectPath(const std::string& path) { return path.rfind(kObjectPrefix, 0) == 0; }

std::string StripObjectPrefix(const std::string& path) {
    if (!IsObjectPath(path)) {
        return path;
    }
    return path.substr(kObjectPrefix.size());
}
}  // namespace

ObjectTreeDialog::ObjectTreeDialog(QWidget* parent)
    : QDialog(parent), treeView_(nullptr), model_(nullptr), okButton_(nullptr), cancelButton_(nullptr) {
    setWindowTitle("Select Objects");
    setMinimumSize(400, 500);
    initUI();
}

ObjectTreeDialog::~ObjectTreeDialog() {
    // Qt deletes child widgets automatically.
}

void ObjectTreeDialog::setMode(ObjectTreeMode mode) {
    mode_ = mode;

    // Rebuild if the tree is already constructed.
    if (model_ && model_->rowCount() > 0) {
        buildObjectTree();
    }
}

void ObjectTreeDialog::initUI() {
    // Create main layout.
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create label.
    QLabel* label = new QLabel("Select objects:", this);
    mainLayout->addWidget(label);

    // Create tree view.
    treeView_ = new QTreeView(this);
    treeView_->setHeaderHidden(true);
    treeView_->setSelectionMode(QAbstractItemView::NoSelection);
    treeView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(treeView_);

    // Create model.
    model_ = new QStandardItemModel(this);
    treeView_->setModel(model_);

    // Connect signals/slots.
    connect(model_, &QStandardItemModel::itemChanged, this, &ObjectTreeDialog::onItemChanged);

    // Create button layout.
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Create OK button.
    okButton_ = new QPushButton("OK", this);
    connect(okButton_, &QPushButton::clicked, this, &ObjectTreeDialog::onOkClicked);
    buttonLayout->addWidget(okButton_);

    // Create Cancel button.
    cancelButton_ = new QPushButton("Cancel", this);
    connect(cancelButton_, &QPushButton::clicked, this, &ObjectTreeDialog::onCancelClicked);
    buttonLayout->addWidget(cancelButton_);

    // Add button layout.
    mainLayout->addLayout(buttonLayout);

    // Set main layout.
    setLayout(mainLayout);
}

void ObjectTreeDialog::setObjectManager(ObjectManager::Ptr objManager) {
    objManager_ = objManager;
    buildObjectTree();
}

bool ObjectTreeDialog::isDirectory(const std::string& path) const {
    if (path.empty()) {
        return true;
    }

    if (IsObjectPath(path)) {
        return false;
    }

    return directoryPaths_.find(path) != directoryPaths_.end();
}

void ObjectTreeDialog::buildObjectTree() {
    // Clear model.
    model_->clear();
    itemToObjectId_.clear();
    selectedObjects_.clear();
    directoryPaths_.clear();

    // Get all objects.
    const LayerList layers = objManager_->layers();

    // Build tree structure.
    std::map<std::string, std::vector<std::string>> objectTree;
    // First pass: preprocess all layer IDs and fill directory set.
    for (const auto& layerPair : layers) {
        const std::string& layerId = layerPair.first;

        // Add layer ID to directory set.
        directoryPaths_.insert(layerId);

        // Handle multi-level directory structure.
        std::vector<std::string> parts = parseObjectId(layerId);
        if (parts.size() > 1) {
            // Build intermediate directories.
            std::string currentPath = "";
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) {
                    currentPath += "#";
                }
                currentPath += parts[i];
                directoryPaths_.insert(currentPath);
            }
        }
    }

    // Log directory set.
    std::cout << "Directory set size: " << directoryPaths_.size() << std::endl;
    for (const auto& dirPath : directoryPaths_) {
        std::cout << "  - dir: " << dirPath << std::endl;
    }

    // Second pass: build tree structure.
    for (const auto& layerPair : layers) {
        const Layer::Ptr& layer = layerPair.second;
        const ObjectList objects = layer->objects();
        const std::string& layerId = layerPair.first;

        // Ensure root directory exists.
        if (objectTree.find("") == objectTree.end()) {
            objectTree[""] = std::vector<std::string>();
        }

        // Handle multi-level directory structure.
        std::vector<std::string> parts = parseObjectId(layerId);
        if (parts.size() > 1) {
            // Build intermediate directories.
            std::string currentPath = "";
            std::string parentPath = "";

            for (size_t i = 0; i < parts.size(); ++i) {
                // Update current path.
                if (i > 0) {
                    currentPath += "#";
                }
                currentPath += parts[i];

                // Ensure current directory exists.
                if (objectTree.find(currentPath) == objectTree.end()) {
                    objectTree[currentPath] = std::vector<std::string>();
                }

                // Add current directory to parent.
                if (i == 0) {
                    // First-level dir goes under root.
                    bool alreadyAdded = false;
                    for (const auto& child : objectTree[""]) {
                        if (child == parts[0]) {
                            alreadyAdded = true;
                            break;
                        }
                    }

                    if (!alreadyAdded) {
                        objectTree[""].push_back(parts[0]);
                    }
                } else {
                    // Other levels go under the parent.
                    bool alreadyAdded = false;
                    for (const auto& child : objectTree[parentPath]) {
                        if (child == currentPath) {
                            alreadyAdded = true;
                            break;
                        }
                    }

                    if (!alreadyAdded) {
                        objectTree[parentPath].push_back(currentPath);
                    }
                }

                // Update parent path.
                parentPath = currentPath;
            }
        } else {
            // Add layer to root.
            bool layerAlreadyAdded = false;
            for (const auto& child : objectTree[""]) {
                if (child == layerId) {
                    layerAlreadyAdded = true;
                    break;
                }
            }

            if (!layerAlreadyAdded) {
                objectTree[""].push_back(layerId);
            }
        }

        // Ensure layer directory exists.
        if (objectTree.find(layerId) == objectTree.end()) {
            objectTree[layerId] = std::vector<std::string>();
        }

        // Add each object under the current layer.
        for (const auto& objectPair : objects) {
            const Object::Ptr& object = objectPair.second;
            const std::string& objectId = object->id();

            // Log object info.
            std::cout << "Process object [" << objectId << "] in layer [" << layerId << "]" << std::endl;

            // Add object to its layer directory.
            bool objAlreadyAdded = false;
            for (const auto& child : objectTree[layerId]) {
                if (child == objectId) {
                    objAlreadyAdded = true;
                    break;
                }
            }

            if (!objAlreadyAdded) {
                objectTree[layerId].push_back(kObjectPrefix + objectId);
            }
        }
    }

    // Debug log.
    std::cout << "Tree built, total nodes: " << objectTree.size() << std::endl;
    for (const auto& pair : objectTree) {
        std::cout << "Node [" << pair.first << "] has " << pair.second.size() << " children:" << std::endl;
        for (const auto& child : pair.second) {
            std::cout << "  - " << child << std::endl;
        }
    }

    // Build root node.
    QStandardItem* rootItem = buildTreeNode("", objectTree);
    model_->appendRow(rootItem);

    // Expand all nodes.
    treeView_->expandAll();
}

QStandardItem* ObjectTreeDialog::buildTreeNode(const std::string& path,
                                               const std::map<std::string, std::vector<std::string>>& tree) {
    // Create node.
    QStandardItem* item = new QStandardItem();

    // Set node text.
    if (path.empty()) {
        item->setText("Root");
    } else {
        // Get current node name.
        std::vector<std::string> parts = parseObjectId(path);
        if (!parts.empty()) {
            // Use the last part as node name.
            item->setText(QString::fromStdString(parts.back()));
        } else {
            item->setText(QString::fromStdString(path));
        }
    }

    // Determine whether this node is a layer node.
    bool isLayerNode = isDirectory(path);

    // Set node checkability.
    // In layer-only mode, only layer nodes are checkable.
    // In object selection mode, all nodes are checkable.
    if (mode_ == ObjectTreeMode::LAYER_ONLY) {
        item->setCheckable(isLayerNode);
        // Keep layer nodes enabled.
        item->setEnabled(true);
    } else {
        item->setCheckable(true);
        item->setEnabled(true);
    }

    // Check if it is preselected.
    if (!path.empty() && preselectedItems_.find(path) != preselectedItems_.end()) {
        item->setCheckState(Qt::Checked);
    } else {
        item->setCheckState(Qt::Unchecked);
    }

    // Store full path in item data.
    item->setData(QString::fromStdString(path), Qt::UserRole);

    // Store node-to-object ID mapping.
    // Use a unique key to avoid name collisions.
    std::string uniqueKey = item->text().toStdString() + "_" + path;
    itemToObjectId_[uniqueKey] = path;

    // Find child nodes.
    auto it = tree.find(path);
    if (it != tree.end()) {
        // Split directories and objects.
        std::vector<std::string> directories;
        std::vector<std::string> objects;

        for (const std::string& childPath : it->second) {
            // Determine whether this is a layer or object.
            if (isDirectory(childPath)) {
                // Layer or directory.
                directories.push_back(childPath);
            } else {
                // Otherwise object.
                objects.push_back(childPath);
            }
        }

        // Add directories first.
        for (const std::string& dirPath : directories) {
            QStandardItem* childItem = buildTreeNode(dirPath, tree);
            item->appendRow(childItem);
        }

        // Then add objects.
        for (const std::string& objPath : objects) {
            // Create object node.
            QStandardItem* childItem = new QStandardItem();

            // Object path is the object ID; display directly.
            std::string rawObjectId = StripObjectPrefix(objPath);
            childItem->setText(QString::fromStdString(rawObjectId));

            // Set node checkability.
            // In layer-only mode, object nodes are not checkable; otherwise all nodes are.
            if (mode_ == ObjectTreeMode::LAYER_ONLY) {
                childItem->setCheckable(false);
                // Keep object nodes visible even in layer-only mode, but disabled.
                childItem->setEnabled(false);
            } else {
                childItem->setCheckable(true);
                childItem->setEnabled(true);
            }

            // Check if it is preselected.
            if (preselectedItems_.find(rawObjectId) != preselectedItems_.end()) {
                childItem->setCheckState(Qt::Checked);
            } else {
                childItem->setCheckState(Qt::Unchecked);
            }

            // Store full path in item data.
            childItem->setData(QString::fromStdString(objPath), Qt::UserRole);

            // Store node-to-object ID mapping.
            // Use a unique key to avoid name collisions.
            std::string uniqueKey = childItem->text().toStdString() + "_" + objPath;
            itemToObjectId_[uniqueKey] = rawObjectId;

            // Add to parent.
            item->appendRow(childItem);
        }
    }

    return item;
}

std::vector<std::string> ObjectTreeDialog::parseObjectId(const std::string& objectId) {
    std::vector<std::string> result;

    // Check whether object ID is empty.
    if (objectId.empty()) {
        return result;
    }

    // Find all "#" positions.
    std::vector<size_t> hashPositions;
    size_t pos = 0;
    while ((pos = objectId.find('#', pos)) != std::string::npos) {
        hashPositions.push_back(pos);
        pos++;
    }

    if (hashPositions.empty()) {
        // If there is no "#", treat the entire ID as one part.
        result.push_back(objectId);
        return result;
    }

    // Handle multi-level structure.
    std::string currentPart = objectId.substr(0, hashPositions[0]);
    if (!currentPart.empty()) {
        result.push_back(currentPart);
    }

    for (size_t i = 0; i < hashPositions.size() - 1; ++i) {
        currentPart = objectId.substr(hashPositions[i] + 1, hashPositions[i + 1] - hashPositions[i] - 1);
        if (!currentPart.empty()) {
            result.push_back(currentPart);
        }
    }

    // Add the last part.
    if (hashPositions.size() > 0) {
        currentPart = objectId.substr(hashPositions.back() + 1);
        if (!currentPart.empty()) {
            result.push_back(currentPart);
        }
    }

    return result;
}

void ObjectTreeDialog::onItemChanged(QStandardItem* item) {
    if (!item) return;

    // Prevent signal recursion.
    disconnect(model_, &QStandardItemModel::itemChanged, this, &ObjectTreeDialog::onItemChanged);

    // Get checked state.
    Qt::CheckState state = item->checkState();

    // Set child check state.
    setChildCheckState(item, state);

    // Update parent check state.
    QStandardItem* parentItem = item->parent();
    if (parentItem) {
        updateParentCheckState(parentItem);
    }
    connect(model_, &QStandardItemModel::itemChanged, this, &ObjectTreeDialog::onItemChanged);

    // Update selected object IDs.
    selectedObjects_.clear();

    // Walk all items.
    for (int i = 0; i < model_->rowCount(); ++i) {
        QStandardItem* rootItem = model_->item(i);
        if (rootItem) {
            collectSelectedObjects(rootItem);
        }
    }

    // Update tree view to show changes immediately.
    treeView_->update();

    // Debug log.
    std::cout << "Selected object count: " << selectedObjects_.size() << std::endl;
    for (const auto& objId : selectedObjects_) {
        std::cout << "  - " << objId << std::endl;
    }
}

void ObjectTreeDialog::setChildCheckState(QStandardItem* item, Qt::CheckState state) {
    if (!item) return;

    // Set current item check state.
    item->setCheckState(state);

    // Recursively set all child check states.
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem* childItem = item->child(i);
        if (childItem) {
            // In layer-only mode, skip object children.
            if (mode_ == ObjectTreeMode::LAYER_ONLY) {
                std::string childPath = childItem->data(Qt::UserRole).toString().toStdString();
                if (!isDirectory(childPath)) {
                    continue;
                }
            }

            setChildCheckState(childItem, state);
        }
    }
}

void ObjectTreeDialog::updateParentCheckState(QStandardItem* item) {
    if (!item) return;

    // Check child check states.
    int checkedCount = 0;
    int partiallyCheckedCount = 0;
    int totalCount = 0;
    int directoryCount = 0;  // Directory count.
    int objectCount = 0;     // Object count.

    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem* childItem = item->child(i);
        if (!childItem) continue;

        // Get child path to determine directory vs object.
        std::string childPath = childItem->data(Qt::UserRole).toString().toStdString();
        bool isDir = isDirectory(childPath);

        // In layer-only mode, skip object counts but record object total.
        if (mode_ == ObjectTreeMode::LAYER_ONLY) {
            if (!isDir) {
                objectCount++;
                continue;
            } else {
                directoryCount++;
            }
        }

        totalCount++;

        if (childItem->checkState() == Qt::Checked) {
            checkedCount++;
        } else if (childItem->checkState() == Qt::PartiallyChecked) {
            partiallyCheckedCount++;
        }
    }

    // Update parent check state.
    if (totalCount > 0) {
        // In layer-only mode, if both objects and directories exist, mark partially checked.
        if (mode_ == ObjectTreeMode::LAYER_ONLY && objectCount > 0 && directoryCount > 0) {
            item->setCheckState(Qt::PartiallyChecked);
        } else if (checkedCount == totalCount) {
            item->setCheckState(Qt::Checked);
        } else if (checkedCount > 0 || partiallyCheckedCount > 0) {
            item->setCheckState(Qt::PartiallyChecked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }

    // Recursively update parent items.
    QStandardItem* parentItem = item->parent();
    if (parentItem) {
        updateParentCheckState(parentItem);
    }
}

void ObjectTreeDialog::collectSelectedObjects(QStandardItem* item) {
    if (!item) return;

    // Check whether current item is checked.
    if (item->checkState() == Qt::Checked) {
        // Get object ID from item data.
        std::string itemPath = item->data(Qt::UserRole).toString().toStdString();
        std::string rawPath = StripObjectPrefix(itemPath);

        if (!itemPath.empty()) {
            // Check whether it is a directory.
            bool isDir = isDirectory(itemPath);

            if (isDir) {
                // If layer or directory, add all objects under it.
                if (mode_ == ObjectTreeMode::ALL) {
                    // Walk all layers.

                    const LayerList layers = objManager_->layers();
                    for (const auto& [layerId, layer] : layers) {
                        // If current layer is selected or a child layer.
                        if (layerId == itemPath ||
                            (layerId.length() > itemPath.length() && layerId.substr(0, itemPath.length()) == itemPath &&
                             layerId[itemPath.length()] == '#')) {
                            const auto objects = layer->objects();
                            // Add all objects under the layer.
                            for (const auto& [objId, obj] : objects) {
                                selectedObjects_.insert(objId);
                            }
                        }
                    }
                }

                // In layer-only mode, add the layer ID itself.
                if (mode_ == ObjectTreeMode::LAYER_ONLY) {
                    selectedObjects_.insert(itemPath);
                }
            } else if (mode_ == ObjectTreeMode::ALL) {
                // If not a directory and in ALL mode, add directly.
                selectedObjects_.insert(rawPath);
            }
        }
    }

    // Recurse into children.
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem* childItem = item->child(i);
        if (childItem) {
            collectSelectedObjects(childItem);
        }
    }
}

std::set<std::string> ObjectTreeDialog::getSelectedObjects() const { return selectedObjects_; }

void ObjectTreeDialog::onOkClicked() { accept(); }

void ObjectTreeDialog::onCancelClicked() { reject(); }

void ObjectTreeDialog::setPreselectedItems(const std::set<std::string>& items) {
    preselectedItems_ = items;

    // Update check state if the tree is already built.
    if (model_ && model_->rowCount() > 0) {
        // Walk all items and check against preselected set.
        for (int i = 0; i < model_->rowCount(); ++i) {
            QStandardItem* rootItem = model_->item(i);
            if (rootItem) {
                updateItemSelectionState(rootItem);
            }
        }
    }
}

// Update item selection state.
void ObjectTreeDialog::updateItemSelectionState(QStandardItem* item) {
    if (!item) return;

    // Get item path.
    std::string itemPath = item->data(Qt::UserRole).toString().toStdString();
    std::string rawPath = StripObjectPrefix(itemPath);

    // Check whether it is in the preselected set.
    if (!rawPath.empty() && preselectedItems_.find(rawPath) != preselectedItems_.end()) {
        item->setCheckState(Qt::Checked);
    }

    // Recurse into children.
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem* childItem = item->child(i);
        if (childItem) {
            updateItemSelectionState(childItem);
        }
    }
}

}  // namespace octo_flex
