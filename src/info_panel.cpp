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


#include "info_panel.h"
#include <QFont>
#include <QTimer>
#include <algorithm>
#include <iostream>

namespace octo_flex {

InfoPanel::InfoPanel(QWidget* parent) : parentWidget_(parent), visible_(false) {
    if (parent) {
        initialize(parent);
    }
}

InfoPanel::~InfoPanel() { cleanup(); }

void InfoPanel::initialize(QWidget* parent) {
    parentWidget_ = parent;

    // Create the toggle button.
    toggleButton_ = new QPushButton(parent);
    toggleButton_->setText("≢");
    toggleButton_->setFixedSize(20, 20);
    toggleButton_->move(margin_, margin_);
    toggleButton_->setStyleSheet("QPushButton { background-color: rgba(255, 255, 255, 150); border: none; }");
    toggleButton_->setFocusPolicy(Qt::NoFocus);  // Prevent the button from taking focus.

    // Ensure toggleButton_ is fully created before installing the event filter.
    QTimer::singleShot(0, [this]() {
        if (toggleButton_ && parentWidget_) {
            toggleButton_->show();
        }
    });
}

void InfoPanel::cleanup() {
    if (toggleButton_) {
        delete toggleButton_;
        toggleButton_ = nullptr;
    }
}

void InfoPanel::setInfoItem(const std::string& id, const std::string& info, InfoItemType type) {
    // Check whether the ID already exists.
    auto it = infoItems_.find(id);
    if (it == infoItems_.end()) {
        // If the ID does not exist, add it to the order list.
        infoItemsOrder_.push_back(id);
        // std::cout << "InfoPanel::setInfoItem: add new item [" << id << "]: " << info
        //<< " (type: " << static_cast<int>(type) << ")" << std::endl;
    } else {
        // std::cout << "InfoPanel::setInfoItem: update item [" << id << "]: " << info
        //<< " (type: " << static_cast<int>(type) << ")" << std::endl;
    }

    // Update or add the info item.
    infoItems_[id] = {info, type};

    // Log current info items.
    // std::cout << "InfoPanel::setInfoItem: item count: " << infoItemsOrder_.size() << std::endl;
    for (const auto& itemId : infoItemsOrder_) {
        if (infoItems_.find(itemId) != infoItems_.end()) {
            // std::cout << "  - [" << itemId << "]: " << infoItems_[itemId].info
            //<< " (type: " << static_cast<int>(infoItems_[itemId].type) << ")" << std::endl;
        }
    }

    // Trigger repaint.
    if (parentWidget_) {
        parentWidget_->update();
    }
}

void InfoPanel::removeInfoItem(const std::string& id) {
    // Remove from the map.
    infoItems_.erase(id);

    // Remove from the order list.
    auto it = std::find(infoItemsOrder_.begin(), infoItemsOrder_.end(), id);
    if (it != infoItemsOrder_.end()) {
        infoItemsOrder_.erase(it);
    }

    // Trigger repaint.
    if (parentWidget_) {
        parentWidget_->update();
    }
}

void InfoPanel::clearInfoItems() {
    infoItems_.clear();
    infoItemsOrder_.clear();

    if (parentWidget_) {
        parentWidget_->update();
    }
}

void InfoPanel::toggle() {
    visible_ = !visible_;

    // Update button text and log debug info.
    if (toggleButton_) {
        if (visible_) {
            toggleButton_->setText("≡");
            // std::cout << "InfoPanel: show panel, item count: " << infoItemsOrder_.size() << std::endl;

            // Log all current info items.
            for (const auto& id : infoItemsOrder_) {
                auto it = infoItems_.find(id);
                if (it != infoItems_.end()) {
                    // std::cout << "  - [" << id << "]: " << it->second.info
                    //<< " (type: " << static_cast<int>(it->second.type) << ")" << std::endl;
                }
            }
        } else {
            toggleButton_->setText("≢");
            // std::cout << "InfoPanel: hide panel" << std::endl;
        }
    }

    // Trigger repaint.
    if (parentWidget_) {
        // std::cout << "InfoPanel: request parent repaint" << std::endl;
        parentWidget_->update();
    }
}

bool InfoPanel::isVisible() const { return visible_; }

QColor InfoPanel::getColorForType(InfoItemType type) const {
    switch (type) {
        case InfoItemType::WARNING:
            return QColor(255, 140, 0);  // Orange.
        case InfoItemType::ERROR:
            return QColor(255, 0, 0);  // Red.
        case InfoItemType::NORMAL:
        default:
            return QColor(0, 0, 0);  // Black.
    }
}

void InfoPanel::draw(QPainter& painter) {
    if (!visible_) {
        return;
    }

    if (infoItemsOrder_.empty()) {
        // std::cout << "InfoPanel::draw: no info items to show" << std::endl;
        return;
    }

    // std::cout << "InfoPanel::draw: render panel with " << infoItemsOrder_.size() << " items" << std::endl;

    // Compute panel position and size.
    int panelX = margin_;
    int panelY = margin_ + (toggleButton_ ? toggleButton_->height() : 0) + 5;
    int panelWidth = width_;
    int lineHeight = 20;
    int panelHeight = lineHeight * infoItemsOrder_.size() + 10;

    // Draw translucent background.
    QColor bgColor(255, 255, 255, static_cast<int>(opacity_ * 255));
    painter.fillRect(panelX, panelY, panelWidth, panelHeight, bgColor);

    // Set font.
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    // Draw each info item.
    int textY = panelY + 15;
    for (const auto& id : infoItemsOrder_) {
        auto it = infoItems_.find(id);
        if (it != infoItems_.end()) {
            // Set color based on item type.
            QColor textColor = getColorForType(it->second.type);
            painter.setPen(textColor);

            QString text = QString::fromStdString(it->second.info);
            painter.drawText(panelX + 10, textY, text);
            // std::cout << "InfoPanel::draw: render item [" << id << "]: " << it->second.info
            //<< " (color: " << textColor.red() << "," << textColor.green() << "," << textColor.blue() << ")" <<
            // std::endl;
            textY += lineHeight;
        }
    }
}

void InfoPanel::setWidth(int width) {
    width_ = width;
    if (parentWidget_) {
        parentWidget_->update();
    }
}

void InfoPanel::setMargin(int margin) {
    margin_ = margin;
    updateButtonPosition();
    if (parentWidget_) {
        parentWidget_->update();
    }
}

void InfoPanel::setOpacity(float opacity) {
    opacity_ = opacity;
    if (parentWidget_) {
        parentWidget_->update();
    }
}

int InfoPanel::getWidth() const { return width_; }

int InfoPanel::getMargin() const { return margin_; }

float InfoPanel::getOpacity() const { return opacity_; }

QPushButton* InfoPanel::getToggleButton() const {
    // Ensure toggleButton_ is initialized.
    return toggleButton_;
}

void InfoPanel::updateButtonPosition() {
    if (toggleButton_) {
        toggleButton_->move(margin_, margin_);
    }
}

}  // namespace octo_flex
