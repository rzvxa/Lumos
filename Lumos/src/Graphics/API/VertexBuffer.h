#pragma once
#include "Core/Core.h"

namespace Lumos
{
	namespace Graphics
	{
		enum class BufferUsage
		{
			STATIC, DYNAMIC, STREAM
		};

		class CommandBuffer;
        class Pipeline;

		class LUMOS_EXPORT VertexBuffer
		{
		public:
			virtual ~VertexBuffer() = default;
			virtual void Resize(u32 size) = 0;
			virtual void SetData(u32 size, const void* data) = 0;
			virtual void SetDataSub(u32 size, const void* data, u32 offset) = 0;
			virtual void ReleasePointer() = 0;
			virtual void Bind(CommandBuffer* commandBuffer, Pipeline* pipeline) = 0;
			virtual void Unbind() = 0;

			template<typename T>
			T* GetPointer()
			{
				return static_cast<T*>(GetPointerInternal());
			}
        protected:
            static VertexBuffer* (*CreateFunc)(const BufferUsage&);
			virtual void* GetPointerInternal() = 0;
		public:
			static VertexBuffer* Create(const BufferUsage& usage = BufferUsage::STATIC);
		};
	}
}
