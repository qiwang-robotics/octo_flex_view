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


#ifndef INFO_PANEL_H
#define INFO_PANEL_H


#include <QColor>
#include <QPainter>
#include <QPushButton>
#include <QWidget>
#include <map>
#include <string>
#include <vector>

namespace octo_flex {

// Info item type enum.
enum class InfoItemType {
    NORMAL,   // Normal info - black.
    WARNING,  // Warning info - orange.
    ERROR     // Error info - red.
};

class InfoPanel {
   public:
    InfoPanel(QWidget* parent = nullptr);
    ~InfoPanel();

    // Initialize the info panel.
    void initialize(QWidget* parent);

    // Clean up resources.
    void cleanup();

    // Info item management.
    void setInfoItem(const std::string& id, const std::string& info, InfoItemType type = InfoItemType::NORMAL);
    void removeInfoItem(const std::string& id);
    void clearInfoItems();

    // Show/hide.
    void toggle();
    bool isVisible() const;

    // Draw the info panel.
    void draw(QPainter& painter);

    // Set properties.
    void setWidth(int width);
    void setMargin(int margin);
    void setOpacity(float opacity);

    // Get properties.
    int getWidth() const;
    int getMargin() const;
    float getOpacity() const;
    QPushButton* getToggleButton() const;

    // Update button position.
    void updateButtonPosition();

   private:
    // Internal info item struct.
    struct InfoItem {
        std::string info;
        InfoItemType type;
    };

    // Get text color by type.
    QColor getColorForType(InfoItemType type) const;

    // Info panel data.
    std::map<std::string, InfoItem> infoItems_;
    std::vector<std::string> infoItemsOrder_;

    // Visibility state.
    bool visible_ = false;

    // Style properties.
    int width_ = 300;
    int margin_ = 10;
    float opacity_ = 0.7f;

    // Control button.
    QPushButton* toggleButton_ = nullptr;
    QWidget* parentWidget_ = nullptr;
};

}  // namespace octo_flex

#endif  // INFO_PANEL_H
