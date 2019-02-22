#pragma once
#include "Utilities/TSingleton.h"
#include "VK.h"
#include "VKContext.h"

namespace Lumos
{
	namespace graphics
	{
		struct QueueFamilyIndices
		{
			int graphicsFamily = -1;
			int presentFamily = -1;

			bool isComplete()
			{
				return graphicsFamily >= 0 && presentFamily >= 0;
			}
		};

		class LUMOS_EXPORT VKDevice : public TSingleton<VKDevice>
		{
			friend class TSingleton<VKDevice>;

		public:
			VKDevice();
			~VKDevice();

			bool Init();
			void Unload();
			bool MemoryTypeFromProperties(uint32_t typeBits, vk::MemoryPropertyFlags reqMask, uint32_t* typeIndex);
			void CreatePipelineCache();

			vk::Device GetDevice()							const { return m_Device; };
			vk::PhysicalDevice GetGPU()						const { return m_PhysicalDevice; };
			vk::Queue GetGraphicsQueue()					const { return m_GraphicsQueue; };
			vk::Queue GetPresentQueue()						const { return m_PresentQueue; };
			uint32_t GetGraphicsQueueFamilyIndex()			const { return m_GraphicsQueueFamilyIndex; };
			vk::SurfaceKHR GetSurface()						const { return m_Surface; };
			vk::Format GetFormat()							const { return m_Format; };
			vk::PhysicalDeviceProperties GetGPUProperties()	const { return m_PhysicalDeviceProperties; };
			vk::PipelineCache GetPipelineCache() 			const { return m_PipelineCache; }

			VKContext* GetVKContext() 						const { return m_VKContext; }

			uint m_SwapChainSize = 0;
            
#ifdef LUMOS_PLATFORM_IOS
            static void* m_IOSView;
#endif

		private:

			vk::Device m_Device;
			vk::PhysicalDevice m_PhysicalDevice;
			vk::PhysicalDeviceProperties m_PhysicalDeviceProperties;
			vk::PhysicalDeviceMemoryProperties m_MemoryProperties;
			std::vector<vk::QueueFamilyProperties> m_QueueFamiliyProperties;
			vk::SurfaceKHR m_Surface;
			uint32_t m_GraphicsQueueFamilyIndex;
			vk::Format m_Format;
			vk::Queue m_GraphicsQueue;
			vk::Queue m_PresentQueue;
			vk::PipelineCache m_PipelineCache;
			vk::DescriptorPool m_DescriptorPool;

			VKContext* m_VKContext;
		};
	}
}