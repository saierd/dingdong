#include "key_screen.h"

#include <optional>

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "ui/constants.h"
#include "ui/gtk_helpers.h"
#include "ui/main_window.h"

int const keyListMinWidth = 250;

class ActionListColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ActionListColumns() {
        add(id);
        add(selected);
        add(caption);
    }

    Gtk::TreeModelColumn<std::string> id;
    Gtk::TreeModelColumn<bool> selected;
    Gtk::TreeModelColumn<Glib::ustring> caption;
};

class KeyScreen::Impl {
public:
    Impl(AccessControl* _accessControl) : accessControl(_accessControl), keyList(2) {
        keyList.signal_row_activated().connect([this](auto...) { updateKeyFormFromListSelection(); });
        keyList.set_activate_on_single_click(true);
        keyList.set_headers_visible(false);
        keyList.get_column(0)->set_visible(false);
        resetFont(keyList);

        keyListScrollView.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);
        keyListScrollView.add(keyList);

        buttonBox.set_spacing(defaultSpacing);

        addButton.signal_clicked().connect([this]() { createNewKey(); });
        loadButtonIcon(addIcon, "/add.svg");
        addButton.set_image(addIcon);
        styleButton(addButton, colorGreen);
        buttonBox.pack_start(addButton, true, true);

        deleteButton.signal_clicked().connect([this]() { deleteCurrentKey(); });
        loadButtonIcon(deleteIcon, "/delete.svg");
        deleteButton.set_image(deleteIcon);
        styleButton(deleteButton, colorRed);
        buttonBox.pack_start(deleteButton, true, true);

        leftVBox.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
        leftVBox.set_spacing(defaultSpacing);
        leftVBox.set_size_request(keyListMinWidth);
        leftVBox.pack_start(keyListScrollView, true, true);
        leftVBox.pack_end(buttonBox, false, false);

        captionLabel.set_text("Caption:");
        captionLabel.set_alignment(Gtk::ALIGN_START);
        captionEntry.signal_changed().connect([this]() { captionChanged(); });
        resetFont(captionEntry);

        captionBox.set_spacing(defaultSpacing);
        captionBox.pack_start(captionLabel, false, false);
        captionBox.pack_end(captionEntry, true, true);

        actionListModel = Gtk::ListStore::create(actionListColumns);
        actionListModel->set_sort_column(actionListColumns.caption, Gtk::SORT_ASCENDING);
        actionListModel->signal_row_changed().connect(
            [this](auto, Gtk::TreeModel::iterator const& row) { actionSelectionChanged(row); });
        actionList.set_model(actionListModel);
        actionList.append_column_editable("Selected", actionListColumns.selected);
        actionList.append_column("Caption", actionListColumns.caption);
        actionList.set_headers_visible(false);
        resetFont(actionList);

        actionListScrollView.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);
        actionListScrollView.add(actionList);

        rightVBox.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
        rightVBox.set_spacing(defaultSpacing);
        rightVBox.pack_start(captionBox, false, false);
        rightVBox.pack_start(actionListScrollView, true, true);

        leftVBox.set_margin_right(defaultSpacing);
        rightVBox.set_margin_left(defaultSpacing);
        splitter.add1(leftVBox);
        splitter.add2(rightVBox);
        splitter.show_all();

        updateKeyList();
    }

    void updateKeyList(bool forceKeyFormUpdate = false) {
        keyList.clear_items();

        for (auto const& key : accessControl->keys()) {
            auto row = keyList.append();
            keyList.set_text(row, 0, key.id().toString());
            std::string caption = key.caption();
            if (caption.empty()) {
                caption = "New Key";
            }
            keyList.set_text(row, 1, caption);
        }

        auto selectRow = [this](guint row) { keyList.get_selection()->select(keyList.get_model()->children()[row]); };

        // Restore the previous selection.
        keyList.get_selection().clear();
        if (currentKey) {
            for (guint row = 0; row < keyList.size(); row++) {
                if (keyList.get_text(row) == currentKey->id().toString()) {
                    selectRow(row);
                    break;
                }
            }
        }
        if (keyList.get_selected().empty() && keyList.size() > 0) {
            selectRow(0);
        }

        updateKeyFormFromListSelection(forceKeyFormUpdate);
    }

    void updateKeyFormFromListSelection(bool forceKeyFormUpdate = false) {
        bool foundSelectedKey = false;
        if (!keyList.get_selected().empty()) {
            UUID selectedId(keyList.get_text(keyList.get_selected()[0]));

            for (auto const& key : accessControl->keys()) {
                if (key.id() == selectedId) {
                    updateKeyForm(key, forceKeyFormUpdate);
                    foundSelectedKey = true;
                    break;
                }
            }
        }

        if (foundSelectedKey) {
            rightVBox.show();
        } else {
            rightVBox.hide();
        }
    }

    void updateKeyForm(Key const& key, bool force = false) {
        if (!force && currentKey && currentKey->id() == key.id()) return;

        updatingKeyForm = true;
        currentKey = key;

        captionEntry.set_text(key.caption());
        captionEntry.grab_focus_without_selecting();

        actionListModel->clear();
        for (auto const& action : accessControl->actions()) {
            auto row = *(actionListModel->append());
            row[actionListColumns.id] = action->id();
            row[actionListColumns.caption] = action->caption();
            row[actionListColumns.selected] = key.hasAction(*action);
        }

        updatingKeyForm = false;
    }

    void captionChanged() {
        if (!currentKey || updatingKeyForm) return;

        currentKey->setCaption(captionEntry.get_text());
        accessControl->insertOrReplaceKey(*currentKey);
        updateKeyList();  // Update list for new caption.
    }

    void actionSelectionChanged(Gtk::TreeModel::iterator const& row) {
        if (!currentKey || updatingKeyForm) return;

        auto action = accessControl->actionById(row->get_value(actionListColumns.id));
        if (!action) return;

        bool selected = row->get_value(actionListColumns.selected);
        if (selected) {
            currentKey->addAction(action);
        } else {
            currentKey->removeAction(*action);
        }

        accessControl->insertOrReplaceKey(*currentKey);
    }

    void createNewKey() {
        // TODO: Let the user input a key.
        createNewKey("123");
    }

    void createNewKey(std::string const& key) {
        Key newKey("", key);

        accessControl->insertOrReplaceKey(newKey);
        currentKey = newKey;
        updateKeyList(true);
    }

    void deleteCurrentKey() {
        if (!currentKey) return;

        accessControl->removeKey(*currentKey);
        currentKey.reset();
        updateKeyList();
    }

    void handleScannedKey(std::string const& key) {
        // If the key is a known key, select it in the key list.
        for (auto const& existingKey : accessControl->keys()) {
            if (existingKey.matches(key)) {
                currentKey = existingKey;
                updateKeyList(true);
                return;
            }
        }

        // If the key is not known, create a new one.
        createNewKey(key);
    }

public:
    AccessControl* accessControl;
    std::optional<Key> currentKey;
    bool updatingKeyForm = false;

    Gtk::Paned splitter;

    Gtk::Box leftVBox;
    Gtk::Box rightVBox;

    Gtk::ScrolledWindow keyListScrollView;
    Gtk::ListViewText keyList;

    Gtk::Box buttonBox;
    Gtk::Image addIcon;
    Gtk::Button addButton;
    Gtk::Image deleteIcon;
    Gtk::Button deleteButton;

    Gtk::Box captionBox;
    Gtk::Label captionLabel;
    Gtk::Entry captionEntry;
    Gtk::ScrolledWindow actionListScrollView;
    Glib::RefPtr<Gtk::ListStore> actionListModel;
    ActionListColumns actionListColumns;
    Gtk::TreeView actionList;
};

KeyScreen::KeyScreen(AccessControl* accessControl) {
    impl = std::make_unique<Impl>(accessControl);
}

KeyScreen::~KeyScreen() = default;

Gtk::Widget& KeyScreen::widget() {
    return impl->splitter;
}

std::vector<ScreenButton> KeyScreen::buttons() {
    ScreenButton backButton([this]() {
        // Discard changes.
        impl->accessControl->reloadKeyFile();
        impl->updateKeyList();
        mainWindow->popScreen();
    });
    backButton.icon = "/back.svg";

    ScreenButton saveButton([this]() {
        impl->accessControl->saveKeyFile();
        impl->updateKeyList();
        mainWindow->popScreen();
    });
    saveButton.icon = "/save.svg";
    saveButton.color = colorGreen;

    return { backButton, saveButton };
}

bool KeyScreen::handleScannedKey(std::string const& key) {
    impl->handleScannedKey(key);
    return true;
}
