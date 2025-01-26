#pragma once

#include <imgui.h>
#include <imgui_internal.h>

namespace ImFlow
{
	struct ImSelectionRect
	{
		ImU32 mDragRectColor{IM_COL32(180, 200, 255, 255)};

		bool handle()
		{
			bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && !ImGui::IsAnyItemHovered();
			mIsMouseDragging = hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 5.f);

			if (mIsMouseDragging && !mHasDragStarted)
			{
				mStartDragPos = ImGui::GetMousePos();
				mHasDragStarted = true;
			}

			if (mHasDragStarted && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				mStartDragPos = {};
				mHasDragStarted = false;
			}

			if (!mHasDragStarted || !mIsMouseDragging)
			{
				mStartDragPos = {};
				return false;
			}

			auto min = ImMin(mStartDragPos, ImGui::GetMousePos());
			auto max = ImMax(mStartDragPos, ImGui::GetMousePos());
			mDragRect = ImRect(min, max);

			auto draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRect(min, max, mDragRectColor, 0.f, 0, 2.f);

			return true;
		}

		bool isOverlapped(const ImRect &rect) const
		{
			if (!mHasDragStarted)
				return false;

			return mDragRect.Overlaps(rect);
		}

	private:
		bool mIsMouseDragging{false};
		bool mHasDragStarted{false};
		ImVec2 mStartDragPos{};
		ImRect mDragRect{};
	};
} // namespace ImFlow