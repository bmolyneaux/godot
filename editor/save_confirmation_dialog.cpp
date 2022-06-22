#include "save_confirmation_dialog.h"

#include "core/os/keyboard.h"
#include "core/string/print_string.h"
#include "core/string/translation.h"
#include "scene/gui/line_edit.h"

void SaveConfirmationDialog::_input_from_window(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> key = p_event;
	if (key.is_valid() && key->is_pressed() && key->get_keycode() == Key::ESCAPE) {
		_cancel_pressed();
	}
}

void SaveConfirmationDialog::_parent_focused() {
	if (!is_exclusive()) {
		_cancel_pressed();
	}
}

void SaveConfirmationDialog::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (is_visible()) {
				save_and_close->grab_focus();
				_update_child_rects();
				parent_visible = get_parent_visible_window();
				if (parent_visible) {
					parent_visible->connect("focus_entered", callable_mp(this, &SaveConfirmationDialog::_parent_focused));
				}
			} else {
				if (parent_visible) {
					parent_visible->disconnect("focus_entered", callable_mp(this, &SaveConfirmationDialog::_parent_focused));
					parent_visible = nullptr;
				}
			}
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			bg->add_theme_style_override("panel", bg->get_theme_stylebox(SNAME("panel"), SNAME("AcceptDialog")));
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (parent_visible) {
				parent_visible->disconnect("focus_entered", callable_mp(this, &SaveConfirmationDialog::_parent_focused));
				parent_visible = nullptr;
			}
		} break;

		case NOTIFICATION_READY:
		case NOTIFICATION_WM_SIZE_CHANGED: {
			if (is_visible()) {
				_update_child_rects();
			}
		} break;

		case NOTIFICATION_WM_CLOSE_REQUEST: {
			_cancel_pressed();
		} break;
	}
}

void SaveConfirmationDialog::_save_and_close_pressed() {
	set_visible(false);
	emit_signal(SNAME("save_and_close"));
}

void SaveConfirmationDialog::_cancel_pressed() {
	Window *parent_window = parent_visible;
	if (parent_visible) {
		parent_visible->disconnect("focus_entered", callable_mp(this, &SaveConfirmationDialog::_parent_focused));
		parent_visible = nullptr;
	}

	call_deferred(SNAME("hide"));

	emit_signal(SNAME("cancelled"));

	if (parent_window) {
		//parent_window->grab_focus();
	}
}

void SaveConfirmationDialog::_dont_save_pressed()
{
	set_visible(false);
	emit_signal(SNAME("dont_save"));
}

void SaveConfirmationDialog::_item_activated(int index)
{
	emit_signal(SNAME("resource_file_activated"), resource_list->get_item_text(index));
}

void SaveConfirmationDialog::_update_child_rects() {
	Size2 label_size = label->get_minimum_size();
	if (label->get_text().is_empty()) {
		label_size.height = 0;
	}
	int margin = hbc->get_theme_constant(SNAME("margin"), SNAME("Dialogs"));
	Size2 size = get_size();
	Size2 hminsize = hbc->get_combined_minimum_size();

	Vector2 cpos(margin, margin + label_size.height);
	Vector2 csize(size.x - margin * 2, size.y - margin * 3 - hminsize.y - label_size.height);

	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c) {
			continue;
		}

		if (c == hbc || c == label || c == bg || c->is_set_as_top_level()) {
			continue;
		}

		c->set_position(cpos);
		c->set_size(csize);
	}

	cpos.y += csize.y + margin;
	csize.y = hminsize.y;

	hbc->set_position(cpos);
	hbc->set_size(csize);

	bg->set_position(Point2());
	bg->set_size(size);
}

Size2 SaveConfirmationDialog::_get_contents_minimum_size() const {
	int margin = hbc->get_theme_constant(SNAME("margin"), SNAME("Dialogs"));
	Size2 minsize = label->get_combined_minimum_size();

	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c) {
			continue;
		}

		if (c == hbc || c == label || c->is_set_as_top_level()) {
			continue;
		}

		Size2 cminsize = c->get_combined_minimum_size();
		minsize.x = MAX(cminsize.x, minsize.x);
		minsize.y = MAX(cminsize.y, minsize.y);
	}

	Size2 hminsize = hbc->get_combined_minimum_size();
	minsize.x = MAX(hminsize.x, minsize.x);
	minsize.y += hminsize.y;
	minsize.x += margin * 2;
	minsize.y += margin * 3; //one as separation between hbc and child

	Size2 wmsize = get_min_size();
	minsize.x = MAX(wmsize.x, minsize.x);
	return minsize;
}

void SaveConfirmationDialog::_bind_methods() {
	ADD_SIGNAL(MethodInfo("save_and_close"));
	ADD_SIGNAL(MethodInfo("cancel"));
	ADD_SIGNAL(MethodInfo("dont_save", PropertyInfo(Variant::STRING_NAME, "action")));
	ADD_SIGNAL(MethodInfo("resource_file_activated", PropertyInfo(Variant::STRING_NAME, "file_path")));
}

void SaveConfirmationDialog::confirm_resources(const List<ResourceFile>& resources) {
	resource_list->clear();
	for (const ResourceFile& resource : resources) {
		resource_list->add_item(resource.file_path, resource.icon);
	}
	popup_centered();

	_resources = resources;
}

List<SaveConfirmationDialog::ResourceFile> SaveConfirmationDialog::get_selected_resources() {
	List<ResourceFile> resources;
	for (int i : resource_list->get_selected_items()) {
		resources.push_back(_resources[i]);
	}
	return resources;
}

SaveConfirmationDialog::SaveConfirmationDialog() {
	set_wrap_controls(true);
	set_visible(false);
	set_transient(true);
	set_exclusive(true);
	set_clamp_to_embedder(true);

	bg = memnew(Panel);
	add_child(bg, false, INTERNAL_MODE_FRONT);

	hbc = memnew(HBoxContainer);

	int margin = hbc->get_theme_constant(SNAME("margin"), SNAME("Dialogs"));
	int button_margin = hbc->get_theme_constant(SNAME("button_margin"), SNAME("Dialogs"));

	label = memnew(Label);
	label->set_anchor(SIDE_RIGHT, Control::ANCHOR_END);
	label->set_begin(Point2(margin, margin));
	label->set_text(TTRC("Select resources to save."));
	add_child(label, false, INTERNAL_MODE_FRONT);

	// TODO(#4299): ItemList's nothing_selected signal implies that deselection with LMB should work. Maybe this is a bug?
	resource_list = memnew(ItemList);
	resource_list->set_anchor(SIDE_RIGHT, Control::ANCHOR_END);
	resource_list->set_anchor(SIDE_BOTTOM, Control::ANCHOR_END);
	resource_list->set_begin(Point2(margin, button_margin + 10));  // no label margin constant
	resource_list->set_end(Point2(-margin, -button_margin - 10));
	resource_list->set_select_mode(ItemList::SELECT_MULTI);
	resource_list->set_custom_minimum_size(Size2(0, 200));
	add_child(resource_list, false, INTERNAL_MODE_FRONT);

	add_child(hbc, false, INTERNAL_MODE_FRONT);

	hbc->add_spacer();
	save_and_close = memnew(Button);
	save_and_close->set_text(TTRC("Save & Close"));
	hbc->add_child(save_and_close);
	hbc->add_spacer();
	cancel = memnew(Button);
	cancel->set_text(TTRC("Cancel"));
	hbc->add_child(cancel);
	hbc->add_spacer();
	dont_save = memnew(Button);
	dont_save->set_text(TTRC("Don't Save"));
	hbc->add_child(dont_save);
	hbc->add_spacer();

	resource_list->connect("item_activated", callable_mp(this, &SaveConfirmationDialog::_item_activated));

	save_and_close->connect("pressed", callable_mp(this, &SaveConfirmationDialog::_save_and_close_pressed));
	cancel->connect("pressed", callable_mp(this, &SaveConfirmationDialog::_cancel_pressed));
	dont_save->connect("pressed", callable_mp(this, &SaveConfirmationDialog::_dont_save_pressed));

	set_title(TTRC("Save Content"));

	connect("window_input", callable_mp(this, &SaveConfirmationDialog::_input_from_window));
	
	set_min_size(Size2(400, 300));
}

SaveConfirmationDialog::~SaveConfirmationDialog() {
}
