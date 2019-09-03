#pragma once

#include "LM.h"
#include "Maths/BoundingShape.h"

#include "Core/Serialisable.h"

namespace Lumos 
{
	class Entity;
	using ComponentType = std::uint8_t;

	class LUMOS_EXPORT LumosComponent : public Serialisable
	{
	public:
		virtual ~LumosComponent() = default;
		virtual Entity* GetEntity() { return m_Entity; }

		virtual void OnRenderComponent() {};
		virtual void OnUpdateComponent(float dt) {};
        virtual void OnIMGUI() {}

		void SetEntity(Entity* entity) { m_Entity = entity; }

		virtual void UpdateBoundingShape() { };
		virtual void OnUpdateTransform(const Maths::Matrix4& entityTransform) {};

		Maths::BoundingShape* GetBoundingShape() const { return m_BoundingShape.get(); }

		const String& GetName() const { return m_Name; }
		bool& GetActive() { return m_Active; }
		const bool GetCanDisable() const { return m_CanDisable; }
		void SetActive(bool active) { m_Active = active; }
        
        void SetName(const String& name) { m_Name = name; }

	protected:
		Entity* m_Entity = nullptr;
		String m_Name;
		bool m_Active = true;
		bool m_CanDisable = true;
		Scope<Maths::BoundingShape> m_BoundingShape;
	};

}
