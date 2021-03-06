#pragma once

class QWidget;

namespace dialogs {
[[nodiscard]] auto ask_about_cancel(QWidget *) -> bool;
[[nodiscard]] auto ask_repeat_workspace_preview(QWidget *) -> int;
auto warn_empty_workspace(QWidget *) -> void;
} // namespace dialogs
