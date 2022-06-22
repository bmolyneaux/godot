#ifndef SAVE_CONFIRMATION_DIALOG_H
#define SAVE_CONFIRMATION_DIALOG_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/panel.h"
#include "scene/gui/popup.h"
#include "scene/gui/texture_button.h"
#include "scene/main/window.h"

class SaveConfirmationDialog : public Window {
	GDCLASS(SaveConfirmationDialog, Window);

	Window *parent_visible = nullptr;
	Panel *bg;
	HBoxContainer *hbc;
	Label *label;
	ItemList *resource_list;
	Button *save_and_close;
	Button *dont_save;
	Button *cancel;

	void _update_child_rects();

	void _input_from_window(const Ref<InputEvent> &p_event);
	void _parent_focused();

	void _save_and_close_pressed();
	void _cancel_pressed();
	void _dont_save_pressed();
	void _item_activated(int index);

protected:
	virtual Size2 _get_contents_minimum_size() const override;

	void _notification(int p_what);
	static void _bind_methods();

public:
	struct ResourceFile {
		enum Type {
			RESOURCE,
			SCRIPT,
			SCENE
		};
		Type type;
		String file_path;
		Ref<Texture2D> icon;
	};

	void confirm_resources(const List<ResourceFile>& resources);

	List<ResourceFile> get_selected_resources();

	SaveConfirmationDialog();
	~SaveConfirmationDialog();

private:
	List<ResourceFile> _resources;
};

#endif
