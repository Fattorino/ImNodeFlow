#pragma once

#include <imgui.h>
#include <utility>
#include <vector>
#include <functional>
#include <memory>

#include "../imgui_node_editor/imgui_node_editor.h"
#include "../imgui_node_editor/imgui_node_editor_internal.h"

namespace ImNode = ax::NodeEditor;

class BasePin;
class BaseLink;
class BaseNode;

enum ImNodeType
{
	ImNodeType_Start,
	ImNodeType_Middle,
	ImNodeType_End
};

// ---------------------------------------------------------------------------------------------------------------------

class INF
{
public:
	friend class BaseNode;

    INF() = delete;

    static void init(std::vector<std::shared_ptr<BaseNode>>* nodes);
    static void destroy();

    static void update();

    template<typename T>
    static void pushNode(ImVec2 pos = ImVec2(0.f, 0.f), bool convert = false);

private:
    static ImNode::EditorContext* m_editorContext;

    static std::vector<std::shared_ptr<BaseNode>>* m_nodes;
    static int m_uniqueID;
    static std::vector<BaseLink*> m_nodeLinks;

    static bool m_validateMultiLink(BasePin* fromPinPtr, BasePin* toPinPtr);
    static bool m_validateLink(BasePin* fromPinPtr, BasePin* toPinPtr);
};

// ---------------------------------------------------------------------------------------------------------------------

class BasePin
{
public:
	BasePin(BaseNode* parent, ImNode::PinKind kind, bool allowMultiLink, const std::string&& text)
	{
		m_parent = parent;
		m_kind = kind;
        m_allowMultiLink = allowMultiLink;
		m_text = text;
	}

	void render();

    void setLink(BaseLink* link) { m_link = link; }

	[[nodiscard]] ImNode::PinId getID() const { return m_id; }
	BaseNode* getParent() { return m_parent; }
	bool isInput() { return m_kind == ImNode::PinKind::Input; }
	[[nodiscard]] bool canMultiLink() const { return m_allowMultiLink; }
    BaseLink* getLink() { return m_link; }

private:
	uintptr_t m_id = reinterpret_cast<uintptr_t>(this);
	ImNode::PinKind m_kind;

	BaseNode* m_parent;
    BaseLink* m_link = nullptr;
	bool m_allowMultiLink;

	std::string m_text;

};

// ---------------------------------------------------------------------------------------------------------------------

class BaseLink
{
public:
	BaseLink(int* id, BasePin* fromPin, BasePin* toPin)
	{
		this->id = *id;
		*id += 1;
		this->fromPin = fromPin;
		this->toPin = toPin;
	}

	ImNode::LinkId id;
	BasePin* fromPin;
	BasePin* toPin;
};

// ---------------------------------------------------------------------------------------------------------------------

class BaseNode
{
public:
	explicit BaseNode(int nodeType, float width = 0.f)
	{
		m_id = INF::m_uniqueID++;
		m_nodeType = nodeType;
		m_width = width;
	}

	virtual void render() = 0;

	virtual std::vector<float> getOut(int index) { return {}; }
	virtual void resolveChain() {}

	ImNode::NodeId getID() { return m_id; }
	[[nodiscard]] int getNodeType() const { return m_nodeType; }
	[[nodiscard]] float getWidth() const { return m_width; }

private:
	ImNode::NodeId m_id;
	int m_nodeType;
	float m_width;
};

// ---------------------------------------------------------------------------------------------------------------------

template<typename T>
void INF::pushNode(ImVec2 pos, bool convert)
{
    static_assert(std::is_base_of<BaseNode, T>::value, "Pushed type is not subclass of BaseNode!");
    m_nodes->emplace_back(std::make_shared<T>());
    if (convert)
        ImNode::SetNodePosition(m_nodes->back()->getID(), reinterpret_cast<ImNode::Detail::EditorContext*>(m_editorContext)->ToCanvas(pos));
    else
        ImNode::SetNodePosition(m_nodes->back()->getID(), pos);
}
