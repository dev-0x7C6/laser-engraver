#pragma once

#include <src/gcode-generator.hpp>

class QWidget;
class QString;

namespace dialogs {
[[nodiscard]] auto add_dialog_layer(const QString &title, const QString &text, upload_instruction &&interpreter) -> upload_instruction;
[[nodiscard]] auto ask_about_cancel(QWidget *) -> bool;
[[nodiscard]] auto ask_repeat_workspace_preview(QWidget *) -> int;
auto warn_empty_workspace(QWidget *) -> void;
} // namespace dialogs
