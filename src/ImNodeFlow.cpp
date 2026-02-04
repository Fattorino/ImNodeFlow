#include "ImNodeFlow.h"

namespace ImFlow {
    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    // Helper function to check if a point is near a line segment
    static bool pointNearLineSegment(const ImVec2& p, const ImVec2& a, const ImVec2& b, float threshold) {
        ImVec2 ab = ImVec2(b.x - a.x, b.y - a.y);
        ImVec2 ap = ImVec2(p.x - a.x, p.y - a.y);
        float ab_len_sq = ab.x * ab.x + ab.y * ab.y;
        if (ab_len_sq < 0.001f) return false;
        float t = (ap.x * ab.x + ap.y * ab.y) / ab_len_sq;
        t = ImClamp(t, 0.0f, 1.0f);
        ImVec2 closest = ImVec2(a.x + t * ab.x, a.y + t * ab.y);
        float dist_sq = (p.x - closest.x) * (p.x - closest.x) + (p.y - closest.y) * (p.y - closest.y);
        return dist_sq < (threshold * threshold);
    }

    int Link::addWaypoint(const ImVec2& pos) {
        // pos is in grid coordinates
        // start/end from pinPoint() are in screen coordinates - convert to grid
        ImVec2 startScreen = m_left->pinPoint();
        ImVec2 endScreen = m_right->pinPoint();
        ImVec2 start = m_inf->screen2grid(startScreen);
        ImVec2 end = m_inf->screen2grid(endScreen);
        
        // Build list of all points in grid coordinates
        std::vector<ImVec2> points;
        points.push_back(start);
        for (const auto& wp : m_waypoints) {
            points.push_back(wp.pos);
        }
        points.push_back(end);

        // Find which segment the position is closest to
        int bestSegment = -1;
        float bestDist = FLT_MAX;
        for (size_t i = 0; i < points.size() - 1; i++) {
            ImVec2 a = points[i];
            ImVec2 b = points[i + 1];
            // Project pos onto segment [a, b]
            ImVec2 ab = ImVec2(b.x - a.x, b.y - a.y);
            ImVec2 ap = ImVec2(pos.x - a.x, pos.y - a.y);
            float ab_len_sq = ab.x * ab.x + ab.y * ab.y;
            if (ab_len_sq < 0.001f) continue;
            float t = (ap.x * ab.x + ap.y * ab.y) / ab_len_sq;
            t = ImClamp(t, 0.0f, 1.0f);
            ImVec2 closest = ImVec2(a.x + t * ab.x, a.y + t * ab.y);
            float dist = sqrtf((pos.x - closest.x) * (pos.x - closest.x) + (pos.y - closest.y) * (pos.y - closest.y));
            if (dist < bestDist) {
                bestDist = dist;
                bestSegment = i;
            }
        }

        Waypoint wp;
        wp.pos = pos;
        wp.posTarget = pos;  // Initialize target to same position
        if (bestSegment >= 0 && bestSegment < (int)m_waypoints.size()) {
            m_waypoints.insert(m_waypoints.begin() + bestSegment + 1, wp);
            return bestSegment + 1;
        } else {
            m_waypoints.push_back(wp);
            return (int)m_waypoints.size() - 1;
        }
    }

    void Link::removeWaypoint(int index) {
        if (index >= 0 && index < (int)m_waypoints.size()) {
            m_waypoints.erase(m_waypoints.begin() + index);
        }
    }

    void Link::setWaypoints(const std::vector<ImVec2>& positions) {
        m_waypoints.clear();
        for (const auto& pos : positions) {
            Waypoint wp;
            wp.pos = pos;
            wp.posTarget = pos;  // Initialize target to same position
            m_waypoints.push_back(wp);
        }
    }

    int Link::getHoveredWaypoint() const {
        ImVec2 mousePos = m_inf->screen2grid(ImGui::GetMousePos());
        for (int i = 0; i < (int)m_waypoints.size(); i++) {
            float dist = sqrtf((mousePos.x - m_waypoints[i].pos.x) * (mousePos.x - m_waypoints[i].pos.x) +
                               (mousePos.y - m_waypoints[i].pos.y) * (mousePos.y - m_waypoints[i].pos.y));
            if (dist < Waypoint::HOVER_RADIUS) {
                return i;
            }
        }
        return -1;
    }

    void Link::update() {
        // Skip update if link has been invalidated (pins destroyed)
        if (!m_valid) return;
        
        // start/end from pinPoint() are in screen coordinates - convert to grid
        ImVec2 startScreen = m_left->pinPoint();
        ImVec2 endScreen = m_right->pinPoint();
        ImVec2 start = m_inf->screen2grid(startScreen);
        ImVec2 end = m_inf->screen2grid(endScreen);
        
        float thickness = m_left->getStyle()->extra.link_thickness;
        bool mouseClickState = m_inf->getSingleUseClick();
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 mouseGridPos = m_inf->screen2grid(mousePos);

        // Build list of all points (start, waypoints, end) - all in GRID coordinates
        std::vector<ImVec2> points;
        points.push_back(start);
        for (auto& wp : m_waypoints) {
            points.push_back(wp.pos);
        }
        points.push_back(end);

        // Helper to normalize a vector
        auto normalize = [](const ImVec2& v) -> ImVec2 {
            float len = sqrt(v.x * v.x + v.y * v.y);
            if (len < 0.001f) return ImVec2(0, 0);
            return ImVec2(v.x / len, v.y / len);
        };
        
        // Calculate tangents for each segment based on neighboring points
        // tangent1[i] = outgoing tangent at points[i]
        // tangent2[i] = incoming tangent at points[i+1]
        std::vector<ImVec2> tangent1(points.size() - 1);
        std::vector<ImVec2> tangent2(points.size() - 1);
        
        for (size_t i = 0; i < points.size() - 1; i++) {
            ImVec2 curr = points[i];
            ImVec2 next = points[i + 1];
            
            // For the first point (output pin), always go right
            if (i == 0) {
                tangent1[i] = ImVec2(1.0f, 0.0f);
            } else {
                // For waypoints, use direction from previous to next point for smooth curves
                ImVec2 prev = points[i - 1];
                tangent1[i] = normalize(ImVec2(next.x - prev.x, next.y - prev.y));
            }
            
            // For the last point (input pin), always come from left
            if (i == points.size() - 2) {
                tangent2[i] = ImVec2(1.0f, 0.0f);  // incoming from left means tangent points right
            } else {
                // For waypoints, use direction from current to next-next point
                ImVec2 nextnext = points[i + 2];
                tangent2[i] = normalize(ImVec2(nextnext.x - curr.x, nextnext.y - curr.y));
            }
        }

        // Check if link is hovered (any segment) - use screen coordinates for collision
        m_hovered = false;
        for (size_t i = 0; i < points.size() - 1; i++) {
            bool hit = false;
            if (m_waypoints.empty()) {
                // No waypoints: use original smart_bezier
                hit = smart_bezier_collider(mousePos, m_inf->grid2screen(points[i]), m_inf->grid2screen(points[i + 1]), 2.5);
            } else {
                // With waypoints: use tangent-aware collision
                hit = smart_bezier_collider_with_tangents(mousePos, 
                    m_inf->grid2screen(points[i]), m_inf->grid2screen(points[i + 1]),
                    tangent1[i], tangent2[i], 2.5);
            }
            if (hit) {
                m_hovered = true;
                thickness = m_left->getStyle()->extra.link_hovered_thickness;
                if (mouseClickState && getHoveredWaypoint() < 0) {
                    m_inf->consumeSingleUseClick();
                    m_selected = true;
                }
                break;
            }
        }

        // Handle waypoint interactions
        int hoveredWp = getHoveredWaypoint();
        
        // Update hovered waypoint
        for (size_t i = 0; i < m_waypoints.size(); i++) {
            m_waypoints[i].hovered = (i == hoveredWp);
        }

        // Start dragging waypoint
        if (hoveredWp >= 0 && mouseClickState) {
            m_draggedWaypointIndex = hoveredWp;
            m_waypoints[hoveredWp].dragged = true;
            m_waypoints[hoveredWp].posTarget = m_waypoints[hoveredWp].pos;  // Initialize target
            m_inf->consumeSingleUseClick();
        }

        // Drag waypoint
        if (m_draggedWaypointIndex >= 0) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                float step = m_inf->getStyle().grid_size / m_inf->getStyle().grid_subdivisions;
                // Apply delta to target position (not snapped)
                m_waypoints[m_draggedWaypointIndex].posTarget += m_inf->getScreenSpaceDelta();
                // Snap to grid for display
                m_waypoints[m_draggedWaypointIndex].pos.x = round(m_waypoints[m_draggedWaypointIndex].posTarget.x / step) * step;
                m_waypoints[m_draggedWaypointIndex].pos.y = round(m_waypoints[m_draggedWaypointIndex].posTarget.y / step) * step;
            } else {
                m_waypoints[m_draggedWaypointIndex].dragged = false;
                m_waypoints[m_draggedWaypointIndex].posTarget = m_waypoints[m_draggedWaypointIndex].pos;  // Sync target
                m_draggedWaypointIndex = -1;
            }
        }

        // Delete waypoint on Delete key when hovered
        if (hoveredWp >= 0 && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
            removeWaypoint(hoveredWp);
        }
        // Delete selected link on Delete key (only if no waypoint is hovered)
        else if (m_selected && hoveredWp < 0 && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
            m_right->deleteLink();
        }

        // Deselect on click elsewhere
        if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_hovered && m_draggedWaypointIndex < 0) {
            m_selected = false;
        }

        // Draw the link segments
        ImU32 color = m_left->getStyle()->color;
        ImU32 outlineColor = m_left->getStyle()->extra.outline_color;

        // Rebuild points in case waypoints moved (still in GRID coords)
        points.clear();
        points.push_back(start);
        for (auto& wp : m_waypoints) {
            points.push_back(wp.pos);
        }
        points.push_back(end);
        
        // Recalculate tangents after points rebuild
        tangent1.resize(points.size() - 1);
        tangent2.resize(points.size() - 1);
        for (size_t i = 0; i < points.size() - 1; i++) {
            ImVec2 curr = points[i];
            ImVec2 next = points[i + 1];
            
            if (i == 0) {
                tangent1[i] = ImVec2(1.0f, 0.0f);
            } else {
                ImVec2 prev = points[i - 1];
                tangent1[i] = normalize(ImVec2(next.x - prev.x, next.y - prev.y));
            }
            
            if (i == points.size() - 2) {
                tangent2[i] = ImVec2(1.0f, 0.0f);
            } else {
                ImVec2 nextnext = points[i + 2];
                tangent2[i] = normalize(ImVec2(nextnext.x - curr.x, nextnext.y - curr.y));
            }
        }

        // Draw outline if selected - convert to screen for drawing
        if (m_selected) {
            for (size_t i = 0; i < points.size() - 1; i++) {
                if (m_waypoints.empty()) {
                    smart_bezier(m_inf->grid2screen(points[i]), m_inf->grid2screen(points[i + 1]), outlineColor,
                                 thickness + m_left->getStyle()->extra.link_selected_outline_thickness);
                } else {
                    smart_bezier_with_tangents(m_inf->grid2screen(points[i]), m_inf->grid2screen(points[i + 1]),
                                 tangent1[i], tangent2[i], outlineColor,
                                 thickness + m_left->getStyle()->extra.link_selected_outline_thickness);
                }
            }
        }

        // Draw segments - convert to screen for drawing
        for (size_t i = 0; i < points.size() - 1; i++) {
            if (m_waypoints.empty()) {
                smart_bezier(m_inf->grid2screen(points[i]), m_inf->grid2screen(points[i + 1]), color, thickness);
            } else {
                smart_bezier_with_tangents(m_inf->grid2screen(points[i]), m_inf->grid2screen(points[i + 1]),
                             tangent1[i], tangent2[i], color, thickness);
            }
        }

        // Draw waypoints - convert to screen for drawing
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        for (auto& wp : m_waypoints) {
            ImVec2 screenPos = m_inf->grid2screen(wp.pos);
            float radius = Waypoint::RADIUS;
            if (wp.hovered || wp.dragged) {
                radius = Waypoint::HOVER_RADIUS;
            }
            draw_list->AddCircleFilled(screenPos, radius, color);
            if (wp.hovered || wp.dragged) {
                draw_list->AddCircle(screenPos, radius, outlineColor, 12, 2.0f);
            }
        }
    }

    Link::~Link() {
        if (!m_cleanupEnabled || !m_left) return;
        m_left->deleteLink();
    }

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    bool BaseNode::isHovered() {
        ImVec2 paddingTL = {m_style->padding.x, m_style->padding.y};
        ImVec2 paddingBR = {m_style->padding.z, m_style->padding.w};
        return ImGui::IsMouseHoveringRect(m_inf->grid2screen(m_pos - paddingTL),
                                          m_inf->grid2screen(m_pos + m_size + paddingBR));
    }

    void BaseNode::update() {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImGui::PushID(this);
        bool mouseClickState = m_inf->getSingleUseClick();
        ImVec2 offset = m_inf->grid2screen({0.f, 0.f});
        ImVec2 paddingTL = {m_style->padding.x, m_style->padding.y};
        ImVec2 paddingBR = {m_style->padding.z, m_style->padding.w};

        draw_list->ChannelsSetCurrent(1); // Foreground
        ImGui::SetCursorScreenPos(offset + m_pos);

        ImGui::BeginGroup();

        // Header
        ImGui::BeginGroup();
        ImGui::TextColored(m_style->header_title_color, "%s", m_title.c_str());
        ImGui::Spacing();
        ImGui::EndGroup();
        float headerH = ImGui::GetItemRectSize().y;
        float titleW = ImGui::GetItemRectSize().x;

        // Render order depends on flipped state
        if (m_flipped) {
            // FLIPPED: Outputs on LEFT, Content, Inputs on RIGHT

            // Outputs (left side when flipped)
            float maxW = 0.0f;
            for (auto &p: m_outs) {
                float w = p->calcWidth();
                if (w > maxW)
                    maxW = w;
            }
            for (auto &p: m_dynamicOuts) {
                float w = p.second->calcWidth();
                if (w > maxW)
                    maxW = w;
            }
            ImGui::BeginGroup();
            for (auto &p: m_outs) {
                if ((m_pos + ImVec2(titleW, 0) + m_inf->getGrid().scroll()).x <
                    ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                    p->setPos(ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p->calcWidth(), 0.f));
                else
                    p->setPos(ImVec2((m_pos + ImVec2(titleW - p->calcWidth(), 0) + m_inf->getGrid().scroll()).x,
                                     ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
                p->update();
            }
            for (auto &p: m_dynamicOuts) {
                if ((m_pos + ImVec2(titleW, 0) + m_inf->getGrid().scroll()).x <
                    ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                    p.second->setPos(
                            ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p.second->calcWidth(), 0.f));
                else
                    p.second->setPos(
                            ImVec2((m_pos + ImVec2(titleW - p.second->calcWidth(), 0) + m_inf->getGrid().scroll()).x,
                                   ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
                p.second->update();
                p.first -= 1;
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            // Content (center)
            ImGui::BeginGroup();
            draw();
            ImGui::Dummy(ImVec2(0.f, 0.f));
            ImGui::EndGroup();
            ImGui::SameLine();

            // Inputs (right side when flipped)
            if (!m_ins.empty() || !m_dynamicIns.empty()) {
                ImGui::BeginGroup();
                for (auto &p: m_ins) {
                    p->setPos(ImGui::GetCursorPos());
                    p->update();
                }
                for (auto &p: m_dynamicIns) {
                    if (p.first == 1) {
                        p.second->setPos(ImGui::GetCursorPos());
                        p.second->update();
                        p.first = 0;
                    }
                }
                ImGui::EndGroup();
            }

        } else {
            // NORMAL: Inputs on LEFT, Content, Outputs on RIGHT

            // Inputs
            if (!m_ins.empty() || !m_dynamicIns.empty()) {
                ImGui::BeginGroup();
                for (auto &p: m_ins) {
                    p->setPos(ImGui::GetCursorPos());
                    p->update();
                }
                for (auto &p: m_dynamicIns) {
                    if (p.first == 1) {
                        p.second->setPos(ImGui::GetCursorPos());
                        p.second->update();
                        p.first = 0;
                    }
                }
                ImGui::EndGroup();
                ImGui::SameLine();
            }

            // Content
            ImGui::BeginGroup();
            draw();
            ImGui::Dummy(ImVec2(0.f, 0.f));
            ImGui::EndGroup();
            ImGui::SameLine();

            // Outputs
            float maxW = 0.0f;
            for (auto &p: m_outs) {
                float w = p->calcWidth();
                if (w > maxW)
                    maxW = w;
            }
            for (auto &p: m_dynamicOuts) {
                float w = p.second->calcWidth();
                if (w > maxW)
                    maxW = w;
            }
            ImGui::BeginGroup();
            for (auto &p: m_outs) {
                if ((m_pos + ImVec2(titleW, 0) + m_inf->getGrid().scroll()).x <
                    ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                    p->setPos(ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p->calcWidth(), 0.f));
                else
                    p->setPos(ImVec2((m_pos + ImVec2(titleW - p->calcWidth(), 0) + m_inf->getGrid().scroll()).x,
                                     ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
                p->update();
            }
            for (auto &p: m_dynamicOuts) {
                if ((m_pos + ImVec2(titleW, 0) + m_inf->getGrid().scroll()).x <
                    ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                    p.second->setPos(
                            ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p.second->calcWidth(), 0.f));
                else
                    p.second->setPos(
                            ImVec2((m_pos + ImVec2(titleW - p.second->calcWidth(), 0) + m_inf->getGrid().scroll()).x,
                                   ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
                p.second->update();
                p.first -= 1;
            }
            ImGui::EndGroup();
        }

        ImGui::EndGroup();
        m_size = ImGui::GetItemRectSize();
        ImVec2 headerSize = ImVec2(m_size.x + paddingBR.x, headerH);

        // Background
        draw_list->ChannelsSetCurrent(0);
        draw_list->AddRectFilled(offset + m_pos - paddingTL, offset + m_pos + m_size + paddingBR, m_style->bg,
                                 m_style->radius);
        draw_list->AddRectFilled(offset + m_pos - paddingTL, offset + m_pos + headerSize, m_style->header_bg,
                                 m_style->radius, ImDrawFlags_RoundCornersTop);
        m_fullSize = m_size + paddingTL + paddingBR;
        ImU32 col = m_style->border_color;
        float thickness = m_style->border_thickness;
        ImVec2 ptl = paddingTL;
        ImVec2 pbr = paddingBR;
        if (m_selected) {
            col = m_style->border_selected_color;
            thickness = m_style->border_selected_thickness;
        }
        if (thickness < 0.f) {
            ptl.x -= thickness / 2;
            ptl.y -= thickness / 2;
            pbr.x -= thickness / 2;
            pbr.y -= thickness / 2;
            thickness *= -1.f;
        }
        draw_list->AddRect(offset + m_pos - ptl, offset + m_pos + m_size + pbr, col, m_style->radius, 0, thickness);


        if (ImGui::IsWindowHovered() && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_inf->on_selected_node())
            selected(false);

        if (isHovered()) {
            m_inf->hoveredNode(this);
            if (mouseClickState) {
                selected(true);
                m_inf->consumeSingleUseClick();
            }
        }

        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::IsAnyItemActive() && isSelected())
            destroy();

        bool onHeader = ImGui::IsMouseHoveringRect(offset + m_pos - paddingTL, offset + m_pos + headerSize);
        if (onHeader && mouseClickState) {
            m_inf->consumeSingleUseClick();
            m_dragged = true;
            m_inf->draggingNode(true);
        }
        if (m_dragged || (m_selected && m_inf->isNodeDragged())) {
            float step = m_inf->getStyle().grid_size / m_inf->getStyle().grid_subdivisions;
            m_posTarget += m_inf->getScreenSpaceDelta();
            // "Slam" The position
            m_pos.x = round(m_posTarget.x / step) * step;
            m_pos.y = round(m_posTarget.y / step) * step;

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_dragged = false;
                m_inf->draggingNode(false);
                m_posTarget = m_pos;
            }
        }
        ImGui::PopID();

        // Deleting dead pins
        m_dynamicIns.erase(std::remove_if(m_dynamicIns.begin(), m_dynamicIns.end(),
                                          [](const std::pair<int, std::shared_ptr<Pin>> &p) { return p.first == 0; }),
                           m_dynamicIns.end());
        m_dynamicOuts.erase(std::remove_if(m_dynamicOuts.begin(), m_dynamicOuts.end(),
                                           [](const std::pair<int, std::shared_ptr<Pin>> &p) { return p.first == 0; }),
                            m_dynamicOuts.end());
    }

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    int ImNodeFlow::m_instances = 0;

    bool ImNodeFlow::on_selected_node() {
        return std::any_of(m_nodes.begin(), m_nodes.end(),
                           [](const auto &n) { return n.second->isSelected() && n.second->isHovered(); });
    }

    bool ImNodeFlow::on_free_space() {
        return std::all_of(m_nodes.begin(), m_nodes.end(),
                           [](const auto &n) { return !n.second->isHovered(); })
               && std::all_of(m_links.begin(), m_links.end(),
                              [](const auto &l) { 
                                  auto link = l.lock();
                                  return !link || (!link->isHovered() && link->getHoveredWaypoint() < 0);
                              });
    }

    std::weak_ptr<Link> ImNodeFlow::getHoveredLink() {
        for (auto& l : m_links) {
            auto link = l.lock();
            if (link && link->isHovered()) {
                return l;
            }
        }
        return std::weak_ptr<Link>();
    }

    int ImNodeFlow::addWaypointToLink(std::shared_ptr<Link> link, const ImVec2& pos) {
        if (link) {
            return link->addWaypoint(pos);
        }
        return -1;
    }

    ImVec2 ImNodeFlow::screen2grid( const ImVec2 & p )
    {
        if ( ImGui::GetCurrentContext() == m_context.getRawContext() )
            return p - m_context.scroll();
        return ( p - m_context.origin() ) / m_context.scale() - m_context.scroll();
    }

    ImVec2 ImNodeFlow::grid2screen( const ImVec2 & p )
    {
        if ( ImGui::GetCurrentContext() == m_context.getRawContext() )
            return p + m_context.scroll();
        return ( p + m_context.scroll() ) * m_context.scale() + m_context.origin();
    }

    void ImNodeFlow::addLink(std::shared_ptr<Link> &link) {
        m_links.push_back(link);
    }

    void ImNodeFlow::update() {
        // Updating looping stuff
        m_hovering = nullptr;
        m_hoveredNode = nullptr;
        m_hoveredLink.reset();  // Reset hovered link each frame
        m_draggingNode = m_draggingNodeNext;
        m_singleUseClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        m_doubleUseClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

        // Create child canvas
        m_context.begin();
        ImGui::GetIO().IniFilename = nullptr;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        // Display grid
        ImVec2 gridSize = ImGui::GetWindowSize();
        float subGridStep = m_style.grid_size / m_style.grid_subdivisions;
        for (float x = fmodf(m_context.scroll().x, m_style.grid_size); x < gridSize.x; x += m_style.grid_size)
            draw_list->AddLine(ImVec2(x, 0.0f), ImVec2(x, gridSize.y), m_style.colors.grid);
        for (float y = fmodf(m_context.scroll().y, m_style.grid_size); y < gridSize.y; y += m_style.grid_size)
            draw_list->AddLine(ImVec2(0.0f, y), ImVec2(gridSize.x, y), m_style.colors.grid);
        if (m_context.scale() > 0.7f) {
            for (float x = fmodf(m_context.scroll().x, subGridStep); x < gridSize.x; x += subGridStep)
                draw_list->AddLine(ImVec2(x, 0.0f), ImVec2(x, gridSize.y), m_style.colors.subGrid);
            for (float y = fmodf(m_context.scroll().y, subGridStep); y < gridSize.y; y += subGridStep)
                draw_list->AddLine(ImVec2(0.0f, y), ImVec2(gridSize.x, y), m_style.colors.subGrid);
        }

        // Update and draw groups (background)
        m_hoveredGroup = nullptr;
        for (auto& [gid, group] : m_groups) {
            if (group.members.empty()) continue;
            
            // Calculate bounding box of all member nodes
            ImVec2 minPos(FLT_MAX, FLT_MAX);
            ImVec2 maxPos(-FLT_MAX, -FLT_MAX);
            bool hasValidNodes = false;
            
            for (NodeUID nodeUid : group.members) {
                auto it = m_nodes.find(nodeUid);
                if (it == m_nodes.end()) continue;
                
                hasValidNodes = true;
                ImVec2 nodePos = it->second->getPos();
                ImVec2 nodeSize = it->second->getSize();
                
                minPos.x = std::min(minPos.x, nodePos.x);
                minPos.y = std::min(minPos.y, nodePos.y);
                maxPos.x = std::max(maxPos.x, nodePos.x + nodeSize.x);
                maxPos.y = std::max(maxPos.y, nodePos.y + nodeSize.y);
            }
            
            if (!hasValidNodes) continue;
            
            // Add padding
            minPos.x -= group.padding;
            minPos.y -= group.padding + 20; // Extra space for title
            maxPos.x += group.padding;
            maxPos.y += group.padding;
            
            // Convert to screen coordinates
            ImVec2 screenMin = grid2screen(minPos);
            ImVec2 screenMax = grid2screen(maxPos);
            
            // Check if hovered
            ImVec2 mousePos = ImGui::GetMousePos();
            group.hovered = (mousePos.x >= screenMin.x && mousePos.x <= screenMax.x &&
                            mousePos.y >= screenMin.y && mousePos.y <= screenMax.y);
            if (group.hovered) {
                m_hoveredGroup = &group;
            }
            
            // Draw group background
            ImU32 bgColor = group.color;
            ImU32 borderCol = group.borderColor;
            if (group.selected) {
                borderCol = IM_COL32(255, 200, 100, 255);
            } else if (group.hovered) {
                bgColor = IM_COL32(
                    (group.color & 0xFF),
                    ((group.color >> 8) & 0xFF),
                    ((group.color >> 16) & 0xFF),
                    std::min(255, (int)((group.color >> 24) & 0xFF) + 30)
                );
            }
            
            draw_list->AddRectFilled(screenMin, screenMax, bgColor, 8.0f);
            draw_list->AddRect(screenMin, screenMax, borderCol, 8.0f, 0, 2.0f);
            
            // Draw group name
            ImVec2 textPos(screenMin.x + 8, screenMin.y + 4);
            draw_list->AddText(textPos, IM_COL32(255, 255, 255, 200), group.name.c_str());
            
            // Handle group dragging
            if (group.hovered && !m_hoveredNode && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                group.dragging = true;
                group.dragOffset = ImVec2(mousePos.x - screenMin.x, mousePos.y - screenMin.y);
                m_selectedGroup = &group;
                // Deselect other groups
                for (auto& [oid, other] : m_groups) {
                    if (oid != gid) other.selected = false;
                }
                group.selected = true;
            }
            
            if (group.dragging) {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    // Calculate delta in grid coordinates
                    ImVec2 delta = getScreenSpaceDelta();
                    
                    // Move all member nodes
                    for (NodeUID nodeUid : group.members) {
                        auto it = m_nodes.find(nodeUid);
                        if (it != m_nodes.end()) {
                            ImVec2 nodePos = it->second->getPos();
                            it->second->setPos(nodePos + screen2grid(delta) - screen2grid(ImVec2(0,0)));
                        }
                    }
                } else {
                    group.dragging = false;
                }
            }
        }
        
        // Delete selected group on Delete key
        if (m_selectedGroup && ImGui::IsKeyPressed(ImGuiKey_Delete, false) && !m_hoveredNode) {
            deleteGroup(m_selectedGroup->uid);
        }

        // Update and draw nodes
        // TODO: I don't like this
        draw_list->ChannelsSplit(2);
        for (auto &node: m_nodes) { node.second->update(); }
        // Remove "toDelete" nodes
        for (auto iter = m_nodes.begin(); iter != m_nodes.end();) {
            if (iter->second->toDestroy()) {
                // Invalidate all links on all pins BEFORE destroying the node
                // to prevent dangling pointer access in Link::update()
                for (auto& pin : iter->second->getIns()) {
                    pin->invalidateAllLinks();
                }
                for (auto& pin : iter->second->getOuts()) {
                    pin->invalidateAllLinks();
                }
                iter = m_nodes.erase(iter);
            }
            else
                ++iter;
        }
        draw_list->ChannelsMerge();
        for (auto &node: m_nodes) { node.second->updatePublicStatus(); }

        // Update and draw links
        for (auto &l: m_links) { if (!l.expired()) l.lock()->update(); }

        // Links drop-off
        if (m_dragOut && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (!m_hovering) {
                if (on_free_space() && m_droppedLinkPopUp) {
                    if (m_droppedLinkPupUpComboKey == ImGuiKey_None || ImGui::IsKeyDown(m_droppedLinkPupUpComboKey)) {
                        m_droppedLinkLeft = m_dragOut;
                        ImGui::OpenPopup("DroppedLinkPopUp");
                    }
                }
            } else
                m_dragOut->createLink(m_hovering);
        }

        // Links drag-out
        if (!m_draggingNode && m_hovering && !m_dragOut && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_dragOut = m_hovering;
        if (m_dragOut) {
            if (m_dragOut->getType() == PinType_Output)
                smart_bezier(m_dragOut->pinPoint(), ImGui::GetMousePos(), m_dragOut->getStyle()->color,
                             m_dragOut->getStyle()->extra.link_dragged_thickness);
            else
                smart_bezier(ImGui::GetMousePos(), m_dragOut->pinPoint(), m_dragOut->getStyle()->color,
                             m_dragOut->getStyle()->extra.link_dragged_thickness);

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                m_dragOut = nullptr;
        }

        // Box selection
        if (on_free_space() && !m_draggingNode && !m_dragOut && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
            m_boxSelecting = true;
            m_boxSelectStart = ImGui::GetMousePos();
            // Deselect all nodes if not holding Ctrl
            if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && !ImGui::IsKeyDown(ImGuiKey_RightCtrl)) {
                for (auto& node : m_nodes) {
                    node.second->selected(false);
                }
            }
        }

        if (m_boxSelecting) {
            ImVec2 boxEnd = ImGui::GetMousePos();
            ImVec2 boxMin = ImVec2(std::min(m_boxSelectStart.x, boxEnd.x), std::min(m_boxSelectStart.y, boxEnd.y));
            ImVec2 boxMax = ImVec2(std::max(m_boxSelectStart.x, boxEnd.x), std::max(m_boxSelectStart.y, boxEnd.y));

            // Draw selection rectangle
            draw_list->AddRectFilled(boxMin, boxMax, m_boxSelectColor);
            draw_list->AddRect(boxMin, boxMax, m_boxSelectBorderColor, 0.0f, 0, 1.5f);

            // Select nodes that intersect with the box
            for (auto& node : m_nodes) {
                ImVec2 nodePos = grid2screen(node.second->getPos());
                ImVec2 nodeSize = node.second->getSize() * m_context.scale();
                ImVec2 nodeMin = nodePos;
                ImVec2 nodeMax = nodePos + nodeSize;

                // Check if node intersects with selection box
                bool intersects = !(nodeMax.x < boxMin.x || nodeMin.x > boxMax.x ||
                                   nodeMax.y < boxMin.y || nodeMin.y > boxMax.y);

                if (intersects) {
                    node.second->selected(true);
                }
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_boxSelecting = false;
            }
        }

        // Right-click PopUp
        if (m_rightClickPopUp && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
            m_hoveredNodeAux = m_hoveredNode;
            // Capture hovered link at the moment of right-click
            m_hoveredLinkAux = getHoveredLink().lock();
            ImGui::OpenPopup("RightClickPopUp");
        }
        if (ImGui::BeginPopup("RightClickPopUp")) {
            m_rightClickPopUp(m_hoveredNodeAux);
            ImGui::EndPopup();
        }

        // Dropped Link PopUp
        if (ImGui::BeginPopup("DroppedLinkPopUp")) {
            m_droppedLinkPopUp(m_droppedLinkLeft);
            ImGui::EndPopup();
        }

        // Removing dead Links
        m_links.erase(std::remove_if(m_links.begin(), m_links.end(),
                                     [](const std::weak_ptr<Link> &l) { return l.expired(); }), m_links.end());

        // Clearing recursion blacklist
        m_pinRecursionBlacklist.clear();

        // Ctrl+G to create group from selection
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_G, false) && ImGui::IsWindowFocused()) {
            createGroupFromSelection();
        }

        m_context.end();
    }

    // ===== GROUP MANAGEMENT IMPLEMENTATION =====

    NodeGroup::GroupUID ImNodeFlow::createGroupFromSelection(const std::string& name)
    {
        std::set<NodeUID> selectedNodes;
        for (auto& [uid, node] : m_nodes) {
            if (node->isSelected()) {
                selectedNodes.insert(uid);
            }
        }
        
        if (selectedNodes.empty()) {
            return 0;
        }
        
        // Remove nodes from any existing groups
        for (NodeUID nodeUid : selectedNodes) {
            removeNodeFromGroup(nodeUid);
        }
        
        NodeGroup group;
        group.uid = m_nextGroupUid++;
        group.name = name;
        group.members = selectedNodes;
        
        m_groups[group.uid] = group;
        return group.uid;
    }

    void ImNodeFlow::addNodeToGroup(NodeGroup::GroupUID groupUid, NodeUID nodeUid)
    {
        auto it = m_groups.find(groupUid);
        if (it == m_groups.end()) return;
        
        // Remove from any existing group first
        removeNodeFromGroup(nodeUid);
        
        it->second.members.insert(nodeUid);
    }

    void ImNodeFlow::removeNodeFromGroup(NodeUID nodeUid)
    {
        for (auto& [gid, group] : m_groups) {
            group.members.erase(nodeUid);
        }
        // Clean up empty groups
        for (auto it = m_groups.begin(); it != m_groups.end();) {
            if (it->second.members.empty()) {
                it = m_groups.erase(it);
            } else {
                ++it;
            }
        }
    }

    void ImNodeFlow::deleteGroup(NodeGroup::GroupUID groupUid)
    {
        m_groups.erase(groupUid);
        if (m_selectedGroup && m_selectedGroup->uid == groupUid) {
            m_selectedGroup = nullptr;
        }
        if (m_hoveredGroup && m_hoveredGroup->uid == groupUid) {
            m_hoveredGroup = nullptr;
        }
    }

    NodeGroup* ImNodeFlow::findGroupForNode(NodeUID nodeUid)
    {
        for (auto& [gid, group] : m_groups) {
            if (group.members.count(nodeUid) > 0) {
                return &group;
            }
        }
        return nullptr;
    }
}
