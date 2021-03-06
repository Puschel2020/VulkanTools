/*
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 * - Richard S. Wright Jr. <richard@lunarg.com>
 * - Christophe Riccio <christophe@lunarg.com>
 */

#include "../vkconfig_core/util.h"

#include "widget_setting_vuid_search.h"

WidgetSettingVUIDSearch::WidgetSettingVUIDSearch(const std::vector<std::string> &layer_vuids,
                                                 const std::vector<std::string> &selected_vuids)
    : QWidget(nullptr), layer_vuids(layer_vuids) {
    // We always want the list presented sorted. Note: This is not
    // strictly necessary.
    std::sort(this->layer_vuids.begin(), this->layer_vuids.end());

    for (std::size_t i = 0, n = selected_vuids.size(); i < n; ++i) {
        RemoveString(this->layer_vuids, selected_vuids[i]);
    }

    _user_box = new QLineEdit(this);
    _user_box->setFocusPolicy(Qt::StrongFocus);

    _add_button = new QPushButton(this);
    _add_button->setText("Add");

    _user_box->show();
    _user_box->setText("");
    _user_box->installEventFilter(this);

    _search_vuid = nullptr;  // Safe to delete a pointer
    ResetCompleter();

    connect(_add_button, SIGNAL(pressed()), this, SLOT(addButtonPressed()));
}

void WidgetSettingVUIDSearch::resizeEvent(QResizeEvent *event) {
    const int button_size = 52;
    QSize parentSize = event->size();
    _user_box->setGeometry(0, 0, parentSize.width() - 2 - button_size, parentSize.height());
    _add_button->setGeometry(parentSize.width() - button_size, 0, button_size, parentSize.height());
}

/// Reload the completer with a revised list of VUID's.
/// I'm quite impressed with how fast this brute force implementation
/// runs in release mode.
void WidgetSettingVUIDSearch::ResetCompleter() {
    if (_search_vuid != nullptr) _search_vuid->deleteLater();

    _search_vuid = new QCompleter(ConvertString(layer_vuids), this);
    _search_vuid->setCaseSensitivity(Qt::CaseSensitive);
    _search_vuid->setCompletionMode(QCompleter::PopupCompletion);
    _search_vuid->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    _search_vuid->setFilterMode(Qt::MatchContains);
    _search_vuid->setMaxVisibleItems(15);
    _search_vuid->setCaseSensitivity(Qt::CaseInsensitive);
    _user_box->setCompleter(_search_vuid);
    connect(_search_vuid, SIGNAL(activated(const QString &)), this, SLOT(addCompleted(const QString &)), Qt::QueuedConnection);
}

// Add the text in the edit control to the list, and clear the control
// This is not really used much, only if they want to add something
// that is not in the completer list.
void WidgetSettingVUIDSearch::addButtonPressed() {
    QString entry = _user_box->text();
    if (entry.isEmpty()) return;

    emit itemSelected(entry);  // Triggers update of GUI
    emit itemChanged();        // Triggers save of profile
    _user_box->setText("");

    // Remove the just added item from the search list
    std::size_t ref_size = layer_vuids.size();
    RemoveString(layer_vuids, entry.toStdString());
    if (ref_size > layer_vuids.size()) {
        ResetCompleter();
    }
}

// Clear the edit control after the completer is finished.
void WidgetSettingVUIDSearch::addCompleted(const QString &addedItem) {
    (void)addedItem;
    // We can't do this right away, the completer emits it's signal
    // before it's really "complete". If we clear the control too soon
    // it clears the completers value too. This might be a Qt bug, but this
    // works really well as a work-a-round
    addButtonPressed();
}

// Item was removed from master list, so add it back to the search
// list.
void WidgetSettingVUIDSearch::addToSearchList(const QString &new_vuid) {
    this->layer_vuids.push_back(new_vuid.toStdString());
    std::sort(this->layer_vuids.begin(), this->layer_vuids.end());
    ResetCompleter();
}

// Ignore mouse wheel events in combo box, otherwise, it fills the list box with ID's
bool WidgetSettingVUIDSearch::eventFilter(QObject *target, QEvent *event) {
    (void)target;
    if (event->type() == QEvent::Wheel) {
        event->ignore();
        return true;
    }

    return _user_box->eventFilter(target, event);
}
