#pragma once

struct ImGuiIO;
class Input;

void SyncImGuiGamepad(const Input& input, ImGuiIO& io);