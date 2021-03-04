#pragma once

class QWidget;

namespace dialogs {
[[nodiscard]] auto ask_about_cancel(QWidget *) -> bool;
auto warn_empty_workspace(QWidget *) -> void;
} // namespace dialogs
