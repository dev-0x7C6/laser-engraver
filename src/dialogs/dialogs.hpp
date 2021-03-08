#pragma once

#include <src/dialogs/font-dialog.h>
#include <src/gcode-generator.hpp>

#include <memory>

class QWidget;
class QString;
class QProgressDialog;

namespace dialogs {
[[nodiscard]] auto add_dialog_layer(const QString &title, const QString &text, upload_instruction &&interpreter) -> upload_instruction;
[[nodiscard]] auto ask_about_cancel(QWidget *parent) -> bool;
[[nodiscard]] auto ask_open_image(QWidget *parent) -> QString;
[[nodiscard]] auto ask_repeat_workspace_preview(QWidget *parent) -> int;
[[nodiscard]] auto wait_connect_engraver() -> std::unique_ptr<QProgressDialog>;
auto ask_font_object(QWidget *parent, std::function<void(TextWithFont)> &&) -> void;
auto ask_gcode_file(QWidget *parent, std::function<void(QString &&path)> &&) -> void;
auto warn_empty_workspace(QWidget *) -> void;
} // namespace dialogs
