#pragma once

#include "stdafx.h"

#include "Common.h"
#include "ER_CoreClock.h"
#include "ER_CoreTime.h"
#include "ER_CoreComponent.h"
#include "ER_CoreServicesContainer.h"
#include "ER_Sandbox.h"
#include "ER_CPUProfiler.h"

#include "RHI\ER_RHI.h"

namespace Library
{
	class ER_Core
	{
	public:
		ER_Core(ER_RHI* aRHI, HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand, UINT width, UINT height, bool isFullscreen);
		virtual ~ER_Core();

		HINSTANCE Instance() const;
		HWND WindowHandle() const;
		const WNDCLASSEX& Window() const;
		const std::wstring& WindowClass() const;
		const std::wstring& WindowTitle() const;
		int ScreenWidth() const;
		int ScreenHeight() const;
		float AspectRatio() const;

		template<typename T>
		std::pair<bool, int> FindInGameComponents(std::vector<ER_CoreComponent*>& components, const T& component)
		{
			std::pair<bool, int> result;

			auto it = std::find(components.begin(), components.end(), component);

			if (it != components.end())
			{
				result.second = distance(components.begin(), it);
				result.first = true;
			}
			else
			{
				result.second = -1;
				result.first = false;
			}

			return result;
		}

		template<typename T>
		bool IsInGameComponents(std::vector<ER_CoreComponent*>& components, const T& component)
		{
			std::pair<bool, int> result;

			auto it = std::find(components.begin(), components.end(), component);

			if (it != components.end())
			{
				return true;
			}
			else
			{
				return false;
			}

		}

		template<typename T>
		std::pair<bool, int> FindInGameLevels(std::vector<ER_Sandbox*>& levels, const T& level)
		{
			std::pair<bool, int> result;

			auto it = std::find(levels.begin(), levels.end(), level);

			if (it != levels.end())
			{
				result.second = distance(levels.begin(), it);
				result.first = true;
			}
			else
			{
				result.second = -1;
				result.first = false;
			}

			return result;
		}

		//const std::vector<ER_CoreComponent*>& Components() const;
		const ER_CoreServicesContainer& GetServices() const;

		virtual void Run();
		virtual void Exit();
		virtual void Initialize();
		virtual void Update(const ER_CoreTime& gameTime);
		virtual void Draw(const ER_CoreTime& gameTime) = 0;

		ER_Sandbox* GetLevel() { return mCurrentSandbox; }
		ER_CPUProfiler* CPUProfiler() const { return mCPUProfiler; }
		ER_RHI* GetRHI() { return mRHI; }
	protected:
		virtual void InitializeWindow();
		virtual void Shutdown();

		ER_CoreClock mCoreClock;
		ER_CoreTime mCoreTime;
		ER_CPUProfiler* mCPUProfiler = nullptr;
		ER_Sandbox* mCurrentSandbox = nullptr;
		ER_RHI* mRHI = nullptr;

		std::vector<ER_CoreComponent*> mCoreEngineComponents; //non-rendering engine components (Input, etc.) TODO remove because we have mServices
		ER_CoreServicesContainer mServices;

		HINSTANCE mInstance;
		std::wstring mWindowClass;
		std::wstring mWindowTitle;
		int mShowCommand;

		HWND mWindowHandle;
		WNDCLASSEX mWindow;

		UINT mScreenWidth;
		UINT mScreenHeight;
		bool mIsFullscreen;
	private:
		ER_Core(const ER_Core& rhs);
		ER_Core& operator=(const ER_Core& rhs);

		POINT CenterWindow(int windowWidth, int windowHeight);
		static LRESULT WINAPI WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	};
}